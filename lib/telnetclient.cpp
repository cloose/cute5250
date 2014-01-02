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

}

namespace Option {

enum Options {
    TERMINAL_TYPE = 24,
    NEW_ENVIRON = 39
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

    bool connected;

    bool isCommand(unsigned char byte, Command::Commands command)
    {
        return (Command::Commands)byte == command;
    }

    bool isOption(unsigned char byte, Option::Options option)
    {
        return (Option::Options)byte == option;
    }

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

void TelnetClient::Private::socketConnected()
{
    connected = true;
}

void TelnetClient::Private::socketReadyRead()
{
    QByteArray receivedData = socket->readAll();

    QByteArray data;
    data.resize(3);

    if (isCommand(receivedData[1], Command::DO) && isOption(receivedData[2], Option::NEW_ENVIRON)) {
        data[0] = Command::IAC;
        data[1] = Command::WILL;
        data[2] = Option::NEW_ENVIRON;
    }

    if (isCommand(receivedData[1], Command::DO) && isOption(receivedData[2], Option::TERMINAL_TYPE)) {
        data[0] = Command::IAC;
        data[1] = Command::WILL;
        data[2] = Option::TERMINAL_TYPE;
    }

    socket->write(data);
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
