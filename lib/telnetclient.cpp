/*
 * Copyright (c) 2013, Christian Loose
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "telnetclient.h"

#include <QDebug>
#include <QTcpSocket>

namespace q5250 {

namespace Command {

enum Commands {
    SE = 240,
    SB = 250,
    WILL = 251,
    WONT = 252,
    DO = 253,
    DONT = 254,
    IAC = 255
};

Commands fromByte(const unsigned char byte)
{
    return (Commands)byte;
}

bool isOptionCommand(const unsigned char byte)
{
    Commands command = fromByte(byte);
    return command == Command::WILL || command == Command::WONT ||
           command == Command::DO || command == Command::DONT;
}

}

namespace Option {

enum Options {
    TRANSMIT_BINARY = 0,    // RFC856
    ECHO = 1,               // RFC857
    SUPPRESS_GO_AHEAD = 3,  // RFC858
    STATUS = 5,             // RFC859
    LOGOUT = 18,            // RFC727
    TERMINAL_TYPE = 24,     // RFC1091
    END_OF_RECORD = 25,     // RFC885
    NAWS = 31,              // RFC1073
    NEW_ENVIRON = 39        // RFC1572
};

Options fromByte(const unsigned char byte)
{
    return (Options)byte;
}

}

namespace SubnegotiationCommand {

enum {
    IS = 0,
    SEND = 1
};

}

class TelnetClient::Private : public QObject
{
    Q_OBJECT

public:
    explicit Private(TelnetClient *parent);

    void setSocket(QTcpSocket *socket);

    TelnetClient *q;
    QTcpSocket *socket;
    QString terminalType;
    QByteArray buffer;

    bool connected;

    void consume();
    int parseCommand(const QByteArray &data);

    Command::Commands replyFor(Command::Commands command, bool allowed);

    void sendCommand(Command::Commands command, Option::Options option);
    void sendCommand(const char *command, int length);
    void sendString(const QString &str);

    bool isOptionAllowed(Option::Options option);

    bool isOptionCommand(const QByteArray &data);
    bool isSubnegotiationCommand(const QByteArray &data);

    QByteArray subnegotiationParameters(const QByteArray &data);

    void stateTerminalType(const QByteArray &data);

public slots:
    void socketConnected();
    void socketReadyRead();
};

TelnetClient::Private::Private(TelnetClient *parent) :
    q(parent),
    socket(0),
    connected(false),
    terminalType("UNKNOWN")
{
    setSocket(new QTcpSocket(this));
}

void TelnetClient::Private::setSocket(QTcpSocket *s)
{
    if (socket) {
        socket->flush();
    }
    delete socket;

    socket = s;
    connected = false;

    if (socket) {
        connect(socket, &QTcpSocket::connected, this, &Private::socketConnected);
        connect(socket, &QTcpSocket::readyRead, this, &Private::socketReadyRead);
    }
}

void TelnetClient::Private::consume()
{
    int currentPos = 0;
    int previousPos = -1;

    while (previousPos < currentPos && currentPos < buffer.size()) {
        previousPos = currentPos;

        const uchar c = (uchar)buffer[currentPos];
        switch (c) {
        case Command::IAC:
            currentPos += parseCommand(buffer.mid(currentPos));
            break;

        default:
            break;
        }
    }

    if (currentPos < buffer.size()) {
        buffer = buffer.mid(currentPos);
    } else {
        buffer.clear();
    }
}

int TelnetClient::Private::parseCommand(const QByteArray &data)
{
    if (data.isEmpty()) return 0;

    if (isOptionCommand(data)) {
        Command::Commands command = Command::fromByte(data[1]);
        Option::Options option = Option::fromByte(data[2]);

        bool allowed = isOptionAllowed(option);

        sendCommand(replyFor(command, allowed), option);

        return 3;
    }

    if (isSubnegotiationCommand(data)) {
        QByteArray parameters = subnegotiationParameters(data);

        Option::Options option = Option::fromByte(parameters[0]);
        switch (option) {
        case Option::TERMINAL_TYPE:
            stateTerminalType(parameters);
            break;

        default:
            break;
        }

        return parameters.size() + 4;
    }

    return 0;
}

Command::Commands TelnetClient::Private::replyFor(Command::Commands command, bool allowed)
{
    switch (command) {
    case Command::DO:
        return allowed ? Command::WILL : Command::WONT;
        break;

    case Command::DONT:
        return Command::WONT;

    case Command::WILL:
        return allowed ? Command::DO : Command::DONT;

    case Command::WONT:
        return Command::DONT;

    default:
        break;
    }
}

void TelnetClient::Private::sendCommand(Command::Commands command, Option::Options option)
{
    QByteArray replyData;
    replyData.resize(3);

    replyData[0] = Command::IAC;
    replyData[1] = command;
    replyData[2] = option;

    socket->write(replyData);
}

void TelnetClient::Private::sendCommand(const char *command, int length)
{
    QByteArray replyData(command, length);
    socket->write(replyData);
}

void TelnetClient::Private::sendString(const QString &str)
{
    socket->write(str.toLocal8Bit());
}

bool TelnetClient::Private::isOptionAllowed(Option::Options option)
{
    // the following telnet options are supported
    return option == Option::END_OF_RECORD ||
           option == Option::NEW_ENVIRON   ||
           option == Option::TERMINAL_TYPE ||
           option == Option::TRANSMIT_BINARY;
}

bool TelnetClient::Private::isOptionCommand(const QByteArray &data)
{
    return data.size() >= 3 && Command::isOptionCommand(data[1]);
}

bool TelnetClient::Private::isSubnegotiationCommand(const QByteArray &data)
{
    return data.size() >= 4 && Command::fromByte(data[1]) == Command::SB;
}

QByteArray TelnetClient::Private::subnegotiationParameters(const QByteArray &data)
{
    for (int i = 2; i < data.size() - 1; ++i) {
        if (Command::fromByte(data[i]) == Command::IAC && Command::fromByte(data[i+1]) == Command::SE) {
            return data.mid(2, i-2);
        }
    }

    return QByteArray();
}

void TelnetClient::Private::stateTerminalType(const QByteArray &data)
{
    if (data[1] != SubnegotiationCommand::SEND)
        return;

    const char c1[4] = { (char)Command::IAC, (char)Command::SB, Option::TERMINAL_TYPE, SubnegotiationCommand::IS };
    sendCommand(c1, sizeof(c1));

    sendString(terminalType);

    const char c2[2] = { (char)Command::IAC, (char)Command::SE };
    sendCommand(c2, sizeof(c2));
}

void TelnetClient::Private::socketConnected()
{
    connected = true;
}

void TelnetClient::Private::socketReadyRead()
{
    buffer.append(socket->readAll());
    consume();
}

TelnetClient::TelnetClient(QObject *parent) :
    QObject(parent),
    d(new Private(this))
{
}

TelnetClient::~TelnetClient()
{
}

void TelnetClient::setTerminalType(const QString &terminalType)
{
    d->terminalType = terminalType;
}

QString TelnetClient::terminalType() const
{
    return d->terminalType;
}

void TelnetClient::connectToHost(const QString &hostName, quint16 port)
{
    if (d->connected) {
        qWarning() << "telnet client is already connected";
        return;
    }

    d->socket->connectToHost(hostName, port);
}

} // namespace q5250

#include "telnetclient.moc"
