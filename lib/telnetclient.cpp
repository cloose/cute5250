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

class TelnetClient::Private : public QObject
{
    Q_OBJECT

public:
    explicit Private(TelnetClient *parent);

    void setSocket(QTcpSocket *socket);

    TelnetClient *q;
    QTcpSocket *socket;
    QByteArray buffer;

    bool connected;

    void consume();
    int parseCommand(const QByteArray &data);

    void sendCommand(Command::Commands command, Option::Options option);

    bool isOptionAllowed(Option::Options option);

    bool isOptionCommand(const QByteArray &data);
    bool isCommand(const unsigned char byte);

public slots:
    void socketConnected();
    void socketReadyRead();
};

TelnetClient::Private::Private(TelnetClient *parent) :
    q(parent),
    socket(0),
    connected(false)
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

        if (command == Command::DO && allowed) {
            sendCommand(Command::WILL, option);
        }

        if (command == Command::WILL && allowed) {
            sendCommand(Command::DO, option);
        }

        return 3;
    }

    return 0;
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
    return data.size() >= 3 && isCommand(data[1]);
}

bool TelnetClient::Private::isCommand(const unsigned char byte)
{
    Command::Commands command = (Command::Commands)byte;
    return command == Command::WILL || command == Command::WONT ||
            command == Command::DO || command == Command::DONT;
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
