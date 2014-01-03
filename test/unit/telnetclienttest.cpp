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
    void acknowledgesTransmitBinaryOption();
    void deniesOfferToUseUnsupportedOption();
    void doesnotAcknowledgeRequestForEnteredModes();
    void repliesToMultipleOptions();
    void statesTerminalTypeOnRequest();
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

void TelnetClientTest::acknowledgesTransmitBinaryOption()
{
    FakeTelnetServer server;
    TelnetClient client;

    server.listenOnTelnetPort();
    client.connectToHost("localhost");
    server.hasConnectionFromClient();

    server.sendCommandToClient(FakeTelnetServer::DO, FakeTelnetServer::TRANSMIT_BINARY);
    server.sendCommandToClient(FakeTelnetServer::WILL, FakeTelnetServer::TRANSMIT_BINARY);

    server.hasReceivedCommand(FakeTelnetServer::WILL, FakeTelnetServer::TRANSMIT_BINARY);
    server.hasReceivedCommand(FakeTelnetServer::DO, FakeTelnetServer::TRANSMIT_BINARY);
}

void TelnetClientTest::deniesOfferToUseUnsupportedOption()
{
    FakeTelnetServer server;
    TelnetClient client;

    server.listenOnTelnetPort();
    client.connectToHost("localhost");
    server.hasConnectionFromClient();

    server.sendCommandToClient(FakeTelnetServer::WILL, FakeTelnetServer::ECHO);
    server.sendCommandToClient(FakeTelnetServer::WILL, FakeTelnetServer::SUPPRESS_GO_AHEAD);

    server.hasReceivedCommand(FakeTelnetServer::DONT, FakeTelnetServer::ECHO);
    server.hasReceivedCommand(FakeTelnetServer::DONT, FakeTelnetServer::SUPPRESS_GO_AHEAD);
}

void TelnetClientTest::doesnotAcknowledgeRequestForEnteredModes()
{
    // RFC854 requires that we don't acknowledge
    // requests to enter a mode we're already in
    FakeTelnetServer server;
    TelnetClient client;

    server.listenOnTelnetPort();
    client.connectToHost("localhost");
    server.hasConnectionFromClient();

    server.sendCommandToClient(FakeTelnetServer::DO, FakeTelnetServer::TERMINAL_TYPE);
    server.hasReceivedCommand(FakeTelnetServer::WILL, FakeTelnetServer::TERMINAL_TYPE);

    server.sendCommandToClient(FakeTelnetServer::DO, FakeTelnetServer::TERMINAL_TYPE);
    server.hasReceivedNoCommand();
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

void TelnetClientTest::statesTerminalTypeOnRequest()
{
    FakeTelnetServer server;
    TelnetClient client;

    server.listenOnTelnetPort();
    client.connectToHost("localhost");
    server.hasConnectionFromClient();

    server.sendSubnegotiationToClient(FakeTelnetServer::TERMINAL_TYPE, FakeTelnetServer::SEND);
    server.hasReceivedTerminalType("UNKNOWN");

    client.setTerminalType("IBM-3477-FC");
    server.sendSubnegotiationToClient(FakeTelnetServer::TERMINAL_TYPE, FakeTelnetServer::SEND);
    server.hasReceivedTerminalType("IBM-3477-FC");
}

QTEST_MAIN(TelnetClientTest)

#include "telnetclienttest.moc"
