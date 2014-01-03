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


FakeTelnetServer::FakeTelnetServer(QObject *parent) :
    QObject(parent),
    tcpServer(new QTcpServer(this)),
    clientSocket(0),
    connectionAvailable(false)
{
    connect(tcpServer, &QTcpServer::newConnection,
            this, &FakeTelnetServer::clientConnected);
}

FakeTelnetServer::~FakeTelnetServer()
{
}

void FakeTelnetServer::listenOnTelnetPort()
{
    QVERIFY2(tcpServer->listen(QHostAddress::Any, 23), qPrintable(tcpServer->errorString()));
}

void FakeTelnetServer::sendCommandToClient(FakeTelnetServer::Commands command, FakeTelnetServer::Options option)
{
    QByteArray data;
    data.resize(3);
    data[0] = (uchar)FakeTelnetServer::IAC;
    data[1] = (uchar)command;
    data[2] = (uchar)option;

    if (!clientSocket) return;

    clientSocket->write(data);
    clientSocket->waitForBytesWritten();
}

void FakeTelnetServer::sendSubnegotiationToClient(FakeTelnetServer::Options option, FakeTelnetServer::SubnegotiationCommands command)
{
    QByteArray data;
    data.resize(6);
    data[0] = (uchar)FakeTelnetServer::IAC;
    data[1] = (uchar)FakeTelnetServer::SB;
    data[2] = (uchar)option;
    data[3] = (uchar)command;
    data[4] = (uchar)FakeTelnetServer::IAC;
    data[5] = (uchar)FakeTelnetServer::SE;

    if (!clientSocket) return;

    clientSocket->write(data);
    clientSocket->waitForBytesWritten();
}

void FakeTelnetServer::hasConnectionFromClient()
{
    QTRY_VERIFY_WITH_TIMEOUT(connectionAvailable, 10000);
}

void FakeTelnetServer::hasReceivedCommand(FakeTelnetServer::Commands command, FakeTelnetServer::Options option)
{
    if (lastDataReceived.isEmpty()) {
        QTRY_VERIFY(!lastDataReceived.isEmpty());
    }

    QVERIFY2(lastDataReceived.size() >= 3, "did not receive a complete option command");
    QCOMPARE((uchar)lastDataReceived[0], (uchar)FakeTelnetServer::IAC);
    QCOMPARE((uchar)lastDataReceived[1], (uchar)command);
    QCOMPARE((uchar)lastDataReceived[2], (uchar)option);

    // consume received command
    lastDataReceived.remove(0, 3);
}

void FakeTelnetServer::hasReceivedTerminalType(const QString &terminalType)
{
    if (lastDataReceived.isEmpty()) {
        QTRY_VERIFY(!lastDataReceived.isEmpty());
    }

    QVERIFY2(lastDataReceived.size() >= 4, "did not receive a complete subnegotiation command");
    QCOMPARE((uchar)lastDataReceived[0], (uchar)FakeTelnetServer::IAC);
    QCOMPARE((uchar)lastDataReceived[1], (uchar)FakeTelnetServer::SB);
    QCOMPARE((uchar)lastDataReceived[2], (uchar)FakeTelnetServer::TERMINAL_TYPE);
    QCOMPARE((uchar)lastDataReceived[3], (uchar)FakeTelnetServer::IS);

    QByteArray parameterData;
    for (int i = 4; i < lastDataReceived.size() - 1; ++i) {
        if (lastDataReceived[i] == FakeTelnetServer::IAC && lastDataReceived[i+1] == FakeTelnetServer::SE) {
            parameterData = lastDataReceived.mid(4, i-4);
        }
    }
    QString actualTerminalType = QString::fromLocal8Bit(parameterData);
    QCOMPARE(actualTerminalType, terminalType);

    // consume received command
    lastDataReceived.remove(0, parameterData.size() + 6);
}

void FakeTelnetServer::clientConnected()
{
    connectionAvailable = true;
    clientSocket = tcpServer->nextPendingConnection();

    connect(clientSocket, &QTcpSocket::readyRead,
            this, &FakeTelnetServer::receivedData);
}

void FakeTelnetServer::receivedData()
{
    lastDataReceived = clientSocket->readAll();
}
