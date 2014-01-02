#include <QNetworkProxyFactory>
#include <QString>
#include <QtTest>

#include <telnetclient.h>
using q5250::TelnetClient;

#include "faketelnetserver.h"

class TelnetClientTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void clientAcknowledgesNewEnvironOption();
    void clientAcknowledgesTerminalTypeOption();
};


void TelnetClientTest::clientAcknowledgesNewEnvironOption()
{
    FakeTelnetServer server;
    TelnetClient client;

    server.listenOnTelnetPort();
    client.connectToHost("localhost");
    server.hasConnectionFromClient();

    server.sendCommandToClient(FakeTelnetServer::DO, FakeTelnetServer::NEW_ENVIRON);
    server.hasReceivedCommand(FakeTelnetServer::WILL, FakeTelnetServer::NEW_ENVIRON);
}

void TelnetClientTest::clientAcknowledgesTerminalTypeOption()
{
    FakeTelnetServer server;
    TelnetClient client;

    server.listenOnTelnetPort();
    client.connectToHost("localhost");
    server.hasConnectionFromClient();

    server.sendCommandToClient(FakeTelnetServer::DO, FakeTelnetServer::TERMINAL_TYPE);
    server.hasReceivedCommand(FakeTelnetServer::WILL, FakeTelnetServer::TERMINAL_TYPE);
}

QTEST_MAIN(TelnetClientTest)

#include "telnetclienttest.moc"
