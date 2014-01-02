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
#include "faketelnetserver.h"

#include <QDebug>
#include <QEventLoop>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtTest>

namespace QTest {

template<>
char *toString<uchar>(const uchar &value)
{
    QByteArray ba = "<";
    ba += QByteArray::number(value);
    ba += ">";
    return qstrdup(ba.data());
}

}

class ServerWorker : public QObject
{
    Q_OBJECT

public:
    explicit ServerWorker(QObject *parent = 0);

signals:
    void clientConnected(bool available, bool timeout);
    void receivedData(const QByteArray &data);

public slots:
    void listenOnTelnetPort();
    void waitForNewConnection();
    void sendToClient(const QByteArray &data);
    void receiveFromClient();

private:
    QTcpServer *tcpServer;
    QTcpSocket *clientSocket;
};

ServerWorker::ServerWorker(QObject *parent) :
    QObject(parent),
    tcpServer(new QTcpServer(this)),
    clientSocket(0)
{
}

void ServerWorker::listenOnTelnetPort()
{
    tcpServer->listen(QHostAddress::Any, 23);
}

void ServerWorker::waitForNewConnection()
{
    bool timeout;
    bool available = tcpServer->waitForNewConnection(5000, &timeout);

    if (available) {
        clientSocket = tcpServer->nextPendingConnection();
    }

    emit clientConnected(available, timeout);
}

void ServerWorker::sendToClient(const QByteArray &data)
{
    if (!clientSocket) return;

    clientSocket->write(data);
    clientSocket->waitForBytesWritten();
}

void ServerWorker::receiveFromClient()
{
    if (!clientSocket) return;

    QByteArray data;

    if (clientSocket->waitForReadyRead(5000)) {
        data = clientSocket->readAll();
    }

    emit receivedData(data);
}


FakeTelnetServer::FakeTelnetServer(QObject *parent) :
    QObject(parent),
    worker(0),
    connectionAvailable(false),
    connectionTimeout(false)
{
    worker = new ServerWorker();
    worker->moveToThread(&serverThread);

    connect(&serverThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &FakeTelnetServer::listen, worker, &ServerWorker::listenOnTelnetPort);
    connect(this, &FakeTelnetServer::waitForNewConnection, worker, &ServerWorker::waitForNewConnection);
    connect(this, &FakeTelnetServer::sendToClient, worker, &ServerWorker::sendToClient);
    connect(this, &FakeTelnetServer::receiveFromClient, worker, &ServerWorker::receiveFromClient);
    connect(worker, &ServerWorker::clientConnected, this, &FakeTelnetServer::clientConnected);
    connect(worker, &ServerWorker::receivedData, this, &FakeTelnetServer::receivedData);

    serverThread.start();
}

FakeTelnetServer::~FakeTelnetServer()
{
    serverThread.quit();
    serverThread.wait();
}

void FakeTelnetServer::listenOnTelnetPort()
{
    emit listen();
}

void FakeTelnetServer::sendCommandToClient(FakeTelnetServer::Commands command, FakeTelnetServer::Options option)
{
    QByteArray data;
    data.resize(3);
    data[0] = (uchar)FakeTelnetServer::IAC;
    data[1] = (uchar)command;
    data[2] = (uchar)option;
    emit sendToClient(data);
}

void FakeTelnetServer::hasConnectionFromClient()
{
    emit waitForNewConnection();
    QTRY_VERIFY(connectionAvailable);
}

void FakeTelnetServer::hasReceivedCommand(FakeTelnetServer::Commands command, FakeTelnetServer::Options option)
{
    if (lastDataReceived.isEmpty()) {
        emit receiveFromClient();
        QTRY_VERIFY(!lastDataReceived.isEmpty());
    }

    QVERIFY2(lastDataReceived.size() >= 3, "did not receive a complete option command");
    QCOMPARE((uchar)lastDataReceived[0], (uchar)FakeTelnetServer::IAC);
    QCOMPARE((uchar)lastDataReceived[1], (uchar)command);
    QCOMPARE((uchar)lastDataReceived[2], (uchar)option);

    // consume received command
    lastDataReceived.remove(0, 3);
}

void FakeTelnetServer::clientConnected(bool available, bool timeout)
{
    connectionAvailable = available;
    connectionTimeout = timeout;
}

void FakeTelnetServer::receivedData(const QByteArray &data)
{
    lastDataReceived = data;
}

#include "faketelnetserver.moc"
