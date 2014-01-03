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
#ifndef FAKETELNETSERVER_H
#define FAKETELNETSERVER_H

#include <QObject>

class QTcpServer;
class QTcpSocket;

class FakeTelnetServer : public QObject
{
    Q_OBJECT

public:
    enum Commands {
        SE = 240,
        SB = 250,
        WILL = 251,
        WONT = 252,
        DO = 253,
        DONT = 254,
        IAC = 255
    };

    enum Options {
        TRANSMIT_BINARY = 0,
        ECHO = 1,
        SUPPRESS_GO_AHEAD = 3,
        TERMINAL_TYPE = 24,
        END_OF_RECORD = 25,
        NEW_ENVIRON = 39
    };

    enum SubnegotiationCommands {
        IS = 0,
        SEND = 1
    };

    explicit FakeTelnetServer(QObject *parent = 0);
    ~FakeTelnetServer();

    void listenOnTelnetPort();
    void sendCommandToClient(Commands command, Options option);
    void sendSubnegotiationToClient(Options option, SubnegotiationCommands command);

    void hasConnectionFromClient();
    void hasReceivedCommand(Commands command, Options option);
    void hasReceivedNoCommand();
    void hasReceivedTerminalType(const QString &terminalType);

public slots:
    void clientConnected();
    void receivedData();

private:
    QTcpServer *tcpServer;
    QTcpSocket *clientSocket;

    bool connectionAvailable;
    QByteArray lastDataReceived;
};

#endif // FAKETELNETSERVER_H
