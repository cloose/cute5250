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
    void acknowledgesNewEnvironOption();
    void acknowledgesTerminalTypeOption();
    void acknowledgesEndOfRecordOption();
    void repliesToMultipleOptions();
};


void TelnetClientTest::acknowledgesNewEnvironOption()
{
    FakeTelnetServer server;
    TelnetClient client;

    server.listenOnTelnetPort();
    client.connectToHost("localhost");
    server.hasConnectionFromClient();

    server.sendCommandToClient(FakeTelnetServer::DO, FakeTelnetServer::NEW_ENVIRON);
    server.hasReceivedCommand(FakeTelnetServer::WILL, FakeTelnetServer::NEW_ENVIRON);
}

void TelnetClientTest::acknowledgesTerminalTypeOption()
{
    FakeTelnetServer server;
    TelnetClient client;

    server.listenOnTelnetPort();
    client.connectToHost("localhost");
    server.hasConnectionFromClient();

    server.sendCommandToClient(FakeTelnetServer::DO, FakeTelnetServer::TERMINAL_TYPE);
    server.hasReceivedCommand(FakeTelnetServer::WILL, FakeTelnetServer::TERMINAL_TYPE);
}

void TelnetClientTest::acknowledgesEndOfRecordOption()
{
    FakeTelnetServer server;
    TelnetClient client;

    server.listenOnTelnetPort();
    client.connectToHost("localhost");
    server.hasConnectionFromClient();

    server.sendCommandToClient(FakeTelnetServer::DO, FakeTelnetServer::END_OF_RECORD);
    server.sendCommandToClient(FakeTelnetServer::WILL, FakeTelnetServer::END_OF_RECORD);

    server.hasReceivedCommand(FakeTelnetServer::WILL, FakeTelnetServer::END_OF_RECORD);
    server.hasReceivedCommand(FakeTelnetServer::DO, FakeTelnetServer::END_OF_RECORD);
}

void TelnetClientTest::repliesToMultipleOptions()
{
    FakeTelnetServer server;
    TelnetClient client;

    server.listenOnTelnetPort();
    client.connectToHost("localhost");
    server.hasConnectionFromClient();

    server.sendCommandToClient(FakeTelnetServer::DO, FakeTelnetServer::NEW_ENVIRON);
    server.sendCommandToClient(FakeTelnetServer::DO, FakeTelnetServer::TERMINAL_TYPE);

    server.hasReceivedCommand(FakeTelnetServer::WILL, FakeTelnetServer::NEW_ENVIRON);
    server.hasReceivedCommand(FakeTelnetServer::WILL, FakeTelnetServer::TERMINAL_TYPE);
}

QTEST_MAIN(TelnetClientTest)

#include "telnetclienttest.moc"
