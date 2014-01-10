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
    void notifiesReceivedOptionCommands();

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

void TelnetParserTest::notifiesReceivedOptionCommands()
{
    QSignalSpy spy(&parser, SIGNAL(optionCommandReceived(uchar, uchar)));

    const char data[] = { '\xff', '\xfd', '\0' };   // IAC DO TRANSMIT_BINARY
    parser.parse(QByteArray::fromRawData(data, 3));

    QCOMPARE(spy.count(), 1);
    QCOMPARE((uchar)spy[0][0].toInt(), (uchar)data[1]);
    QCOMPARE((uchar)spy[0][1].toInt(), (uchar)data[2]);
}

QTEST_APPLESS_MAIN(TelnetParserTest)

#include "telnetparsertest.moc"
