#include <QtTest>

#include <telnetparser.h>
using q5250::TelnetParser;

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

class TelnetParserTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void notifiesReceivedData();
    void replacesEscapedIACBytes();
    void notifiesAllReceivedOptionCommands();
    void notifiesReceivedSubnegotationCommandWithoutParameters();
    void notifiesReceivedSubnegotationCommandWithParameters();

private:
    TelnetParser parser;
};


void TelnetParserTest::notifiesReceivedData()
{
    QSignalSpy spy(&parser, SIGNAL(dataReceived(QByteArray)));

    QByteArray expectedData("TEST");
    parser.parse(expectedData);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy[0][0].toByteArray(), expectedData);
}

void TelnetParserTest::replacesEscapedIACBytes()
{
    QSignalSpy spy(&parser, SIGNAL(dataReceived(QByteArray)));

    const char data[] = { 'a', '\xff', '\xff', 'b' };
    parser.parse(QByteArray::fromRawData(data, 4));

    const char expectedData[] = {'a', '\xff', 'b'};

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy[0][0].toByteArray(), QByteArray::fromRawData(expectedData, 3));
}

void TelnetParserTest::notifiesAllReceivedOptionCommands()
{
    QSignalSpy spy(&parser, SIGNAL(optionCommandReceived(uchar, uchar)));

    // IAC DO TRANSMIT_BINARY, IAC DO END_OF_RECORD
    const char data[] = { '\xff', '\xfd', '\0', '\xff', '\xfd', '\x19' };
    parser.parse(QByteArray::fromRawData(data, 6));

    QCOMPARE(spy.count(), 2);

    // IAC DO TRANSMIT-BINARY
    QCOMPARE((uchar)spy[0][0].toInt(), (uchar)data[1]);
    QCOMPARE((uchar)spy[0][1].toInt(), (uchar)data[2]);

    // IAC DO END-OF-RECORD
    QCOMPARE((uchar)spy[1][0].toInt(), (uchar)data[4]);
    QCOMPARE((uchar)spy[1][1].toInt(), (uchar)data[5]);
}

void TelnetParserTest::notifiesReceivedSubnegotationCommandWithoutParameters()
{
    QSignalSpy spy(&parser, SIGNAL(subnegotationReceived(uchar, uchar, QByteArray)));
    QSignalSpy dataSpy(&parser, SIGNAL(dataReceived(QByteArray)));

    // IAC SB TERMINAL-TYPE SEND IAC SE
    const char data[] = { '\xff', '\xfa', '\x18', '\x01', '\xff', '\xf0' };
    parser.parse(QByteArray::fromRawData(data, 6));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(dataSpy.count(), 0);

    // TERMINAL-TYPE
    QCOMPARE((uchar)spy[0][0].toInt(), (uchar)data[2]);

    // SEND
    QCOMPARE((uchar)spy[0][1].toInt(), (uchar)data[3]);

    QVERIFY(spy[0][2].toByteArray().isEmpty());
}

void TelnetParserTest::notifiesReceivedSubnegotationCommandWithParameters()
{
    QSignalSpy spy(&parser, SIGNAL(subnegotationReceived(uchar, uchar, QByteArray)));
    QSignalSpy dataSpy(&parser, SIGNAL(dataReceived(QByteArray)));

    // IAC SB NEW-ENVIRON SEND ABC IAC SE
    const char data[] = { '\xff', '\xfa', '\x27', '\x01', 'A', 'B', 'C', '\xff', '\xf0' };
    parser.parse(QByteArray::fromRawData(data, 9));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(dataSpy.count(), 0);

    // TERMINAL-TYPE
    QCOMPARE((uchar)spy[0][0].toInt(), (uchar)data[2]);

    // SEND
    QCOMPARE((uchar)spy[0][1].toInt(), (uchar)data[3]);

    QCOMPARE(spy[0][2].toByteArray(), QByteArray("ABC"));
}

QTEST_APPLESS_MAIN(TelnetParserTest)

#include "telnetparsertest.moc"
