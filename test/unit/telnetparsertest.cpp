#include <QtTest>

#include <telnetparser.h>
using q5250::TelnetParser;

class TelnetParserTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void notifiesReceivedData();
    void replacesEscapedIACBytes();

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

QTEST_APPLESS_MAIN(TelnetParserTest)

#include "telnetparsertest.moc"
