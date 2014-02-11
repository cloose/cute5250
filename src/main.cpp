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
#include <QApplication>

#include <QDebug>

#include <generaldatastream.h>
#include <telnet/tcpsockettelnetconnection.h>
#include <telnet/telnetclient.h>
#include <terminal/displaybuffer.h>
#include <terminal/terminalemulator.h>
using namespace q5250;

class TerminalDisplayBuffer : public DisplayBuffer
{
// DisplayBuffer interface
public:
    void setSize(unsigned char columns, unsigned char rows)
    {
        qDebug() << Q_FUNC_INFO << columns << rows;
    }

    void setBufferAddress(unsigned char column, unsigned char row)
    {
        qDebug() << Q_FUNC_INFO << column << row;
    }

    unsigned char column() const
    {
        return 0;
    }

    unsigned char row() const
    {
        return 0;
    }

    void setCharacter(unsigned char ch)
    {
        qDebug() << Q_FUNC_INFO << ch;
    }
};

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
    TerminalEmulator *terminal;
};

Main::Main(QObject *parent) :
    QObject(parent),
    connection(new TcpSocketTelnetConnection(this)),
    client(new TelnetClient(connection)),
    terminal(new TerminalEmulator())
{
    connect(connection, &TcpSocketTelnetConnection::readyRead,
            client, &TelnetClient::readyRead);
    connect(client, &TelnetClient::dataReceived,
            terminal, &TerminalEmulator::dataReceived);

    client->setTerminalType("IBM-3477-FC");
    terminal->setDisplayBuffer(new TerminalDisplayBuffer());
    connection->connectToHost(QStringLiteral("ASKNIDEV"), 23);
}

void Main::dataReceived(const QByteArray &data)
{
//    qDebug() << "--- SERVER ---";
//    for (int i = 0; i < data.size(); ++i) {
//        qDebug() << QString::number(uchar(data[i]), 16)
//                 << QString::number(uchar(data[i]))
//                 << data[i];
//    }

//    GeneralDataStream stream(data);
//    qDebug() << "Valid?" << stream.isValid();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Main main;

    return app.exec();
}

#include "main.moc"
