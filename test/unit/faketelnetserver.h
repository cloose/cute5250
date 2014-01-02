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
#include <QThread>

class QThread;
class ServerWorker;

class FakeTelnetServer : public QObject
{
    Q_OBJECT

public:
    enum Commands {
        WILL = 251,
        WONT = 252,
        DO = 253,
        DONT = 254,
        IAC = 255
    };

    enum Options {
        TERMINAL_TYPE = 24,
        NEW_ENVIRON = 39
    };

    explicit FakeTelnetServer(QObject *parent = 0);
    ~FakeTelnetServer();

    void listenOnTelnetPort();
    void sendCommandToClient(Commands command, Options option);

    void hasConnectionFromClient();
    void hasReceivedCommand(Commands command, Options option);

signals:
    void listen();
    void waitForNewConnection();
    void sendToClient(const QByteArray &data);
    void receiveFromClient();

public slots:
    void clientConnected(bool available, bool timeout);
    void receivedData(const QByteArray &data);

private:
    QThread serverThread;
    ServerWorker *worker;
    bool connectionAvailable;
    bool connectionTimeout;
    QByteArray lastDataReceived;
};

#endif // FAKETELNETSERVER_H
