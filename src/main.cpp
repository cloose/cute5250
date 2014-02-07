/*
 * Copyright (c) 2014, Christian Loose
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
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
#include <QCoreApplication>

#include <QDebug>

#include <telnet/tcpsockettelnetconnection.h>
#include <telnet/telnetclient.h>
using namespace q5250;

class Main : public QObject
{
    Q_OBJECT

public:
    Main(QObject *parent = 0);

private slots:
    void dataReceived(const QByteArray &data);

private:
    TcpSocketTelnetConnection *connection;
    TelnetClient *client;
};

Main::Main(QObject *parent) :
    QObject(parent),
    connection(new TcpSocketTelnetConnection(this)),
    client(new TelnetClient(connection))
{
    connect(connection, &TcpSocketTelnetConnection::readyRead,
            client, &TelnetClient::readyRead);
    connect(client, &TelnetClient::dataReceived,
            this, &Main::dataReceived);

    connection->connectToHost(QStringLiteral("ASKNIDEV"), 23);
}

void Main::dataReceived(const QByteArray &data)
{
    qDebug() << "--- SERVER ---";
    for (int i = 0; i < data.size(); ++i) {
        qDebug() << QString::number(uchar(data[i]), 16)
                 << QString::number(uchar(data[i]))
                 << data[i];
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Main main;

    return app.exec();
}

#include "main.moc"
