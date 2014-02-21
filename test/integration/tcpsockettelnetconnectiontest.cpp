/*
 * Copyright (c) 2013-2014, Christian Loose
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
#include <gmock/gmock.h>
using namespace testing;

#include <QByteArray>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>

#include <telnet/tcpsockettelnetconnection.h>
using namespace q5250;

class FakeTelnetServer : public QObject
{
    Q_OBJECT
public:
    FakeTelnetServer() : tcpServer(new QTcpServer(this)), clientSocket(0)
    {
        connect(tcpServer, &QTcpServer::newConnection, [&]() {
           clientSocket = tcpServer->nextPendingConnection();
           connect(clientSocket, &QTcpSocket::readyRead, [&]() {
               QByteArray data = clientSocket->readAll();
               emit receivedData(data);
           });
           emit clientConnected();
        });
    }
    ~FakeTelnetServer()
    {
        if (clientSocket) {
            clientSocket->close();
        }
        tcpServer->close();
    }

    void listen() { tcpServer->listen(QHostAddress::Any, 8023); }
    void sendDataToClient(const QByteArray &data)
    {
        if (!clientSocket) return;

        clientSocket->write(data);
        clientSocket->waitForBytesWritten();
    }

signals:
    void clientConnected();
    void receivedData(const QByteArray &data);

private:
    QTcpServer *tcpServer;
    QTcpSocket *clientSocket;
};

class ATcpSocketTelnetConnection : public Test
{
public:
    ATcpSocketTelnetConnection() { server.listen(); }

    bool openConnection(TcpSocketTelnetConnection &connection)
    {
        QSignalSpy spy(&server, SIGNAL(clientConnected()));
        connection.connectToHost("localhost", 8023);
        return spy.wait();
    }

    FakeTelnetServer server;
    QByteArray ArbitraryRawData{"A"};
};

TEST_F(ATcpSocketTelnetConnection, opensConnectionToAServer)
{
    TcpSocketTelnetConnection connection;
    QSignalSpy spy(&connection, SIGNAL(connected()));

    connection.connectToHost("localhost", 8023);

    ASSERT_TRUE(spy.wait());
}

TEST_F(ATcpSocketTelnetConnection, emitsReadyReadWhenReceivingDataFromServer)
{
    TcpSocketTelnetConnection connection;
    QSignalSpy spy(&connection, SIGNAL(readyRead()));
    openConnection(connection);

    server.sendDataToClient(ArbitraryRawData);

    ASSERT_TRUE(spy.wait());
}

TEST_F(ATcpSocketTelnetConnection, readsAllDataReceivedFromServer)
{
    TcpSocketTelnetConnection connection;
    openConnection(connection);
    QSignalSpy spy(&connection, SIGNAL(readyRead()));
    server.sendDataToClient(ArbitraryRawData);
    spy.wait();

    QByteArray receivedData = connection.readAll();

    ASSERT_THAT(receivedData, Eq(ArbitraryRawData));
}

TEST_F(ATcpSocketTelnetConnection, sendsDataToServer)
{
    TcpSocketTelnetConnection connection;
    openConnection(connection);
    QSignalSpy spy(&server, SIGNAL(receivedData(QByteArray)));

    connection.write(ArbitraryRawData);

    ASSERT_TRUE(spy.wait());
    ASSERT_THAT(spy[0][0].toByteArray(), Eq(ArbitraryRawData));
}

#include "tcpsockettelnetconnectiontest.moc"
