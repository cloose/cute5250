#include <QtTest>

#include <QDataStream>
#include <QString>
#include <QTextCodec>

#include <terminalemulation.h>
using q5250::TerminalEmulation;

class TerminalEmulationTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void notifiesClearUnitWhenReceived();
    void notifiesSetBufferAddressWhenReceived();
    void notifiesRepeatCharacterWhenReceived();
    void notifiesDisplayTextWhenReceived();

private:
    QByteArray createGeneralDataStreamFromData(const QByteArray &data);

    QTextCodec *ebcdic500;
};


void TerminalEmulationTest::initTestCase()
{
    ebcdic500 = QTextCodec::codecForName("IBM500");
}

void TerminalEmulationTest::notifiesClearUnitWhenReceived()
{
    QByteArray data;
    data.append(0x04);  // ESC
    data.append(0x40);  // CU

    TerminalEmulation emulation;

    QSignalSpy spy(&emulation, SIGNAL(clearUnit()));

    emulation.dataReceived(createGeneralDataStreamFromData(data));

    QCOMPARE(spy.count(), 1);
}

void TerminalEmulationTest::notifiesSetBufferAddressWhenReceived()
{
    unsigned char expectedRow = 2;
    unsigned char expectedColumn = 15;

    QByteArray data;
    data.append(0x04);        // ESC
    data.append(0x11);        // WTD
    data.append((char)0x00);  // CC1
    data.append((char)0x00);  // CC2
    data.append(0x11);        // SBA
    data.append(expectedRow);
    data.append(expectedColumn);

    TerminalEmulation emulation;

    QSignalSpy spy(&emulation, SIGNAL(setBufferAddress(unsigned char,unsigned char)));

    emulation.dataReceived(createGeneralDataStreamFromData(data));

    QCOMPARE(spy.count(), 1);

    auto arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toInt(), (int)expectedColumn);
    QCOMPARE(arguments.at(1).toInt(), (int)expectedRow);
}

void TerminalEmulationTest::notifiesRepeatCharacterWhenReceived()
{
    unsigned char expectedRow = 2;
    unsigned char expectedColumn = 15;
    unsigned char expectedCharacter = 0x82;     // 'b' on CP500

    QByteArray data;
    data.append(0x04);        // ESC
    data.append(0x11);        // WTD
    data.append((char)0x00);  // CC1
    data.append((char)0x00);  // CC2
    data.append(0x02);        // RA
    data.append(expectedRow);
    data.append(expectedColumn);
    data.append(expectedCharacter);

    TerminalEmulation emulation;

    QSignalSpy spy(&emulation, SIGNAL(repeatCharacter(uint,uint,uchar)));

    emulation.dataReceived(createGeneralDataStreamFromData(data));

    QCOMPARE(spy.count(), 1);

    auto arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toInt(), (int)expectedColumn);
    QCOMPARE(arguments.at(1).toInt(), (int)expectedRow);
    QCOMPARE(arguments.at(2).toInt(), (int)expectedCharacter);
}

void TerminalEmulationTest::notifiesDisplayTextWhenReceived()
{
    QByteArray ebcdicData = ebcdic500->fromUnicode(QStringLiteral("TestString"));

    QByteArray data;
    data.append(0x04);        // ESC
    data.append(0x11);        // WTD
    data.append((char)0x00);  // CC1
    data.append((char)0x00);  // CC2
    data.append(ebcdicData);

    TerminalEmulation emulation;

    QSignalSpy spy(&emulation, SIGNAL(displayText(QByteArray)));

    emulation.dataReceived(createGeneralDataStreamFromData(data));

    QCOMPARE(spy.count(), 1);

    auto arguments = spy.takeFirst();

    QByteArray actualData = arguments.at(0).toByteArray();
    QCOMPARE(actualData, ebcdicData);
    QCOMPARE(ebcdic500->toUnicode(actualData), QStringLiteral("TestString"));
}

QByteArray TerminalEmulationTest::createGeneralDataStreamFromData(const QByteArray &data)
{
    QByteArray result;

    QDataStream dataStream(&result, QIODevice::WriteOnly);

    quint16 recordLength = 0x000a + data.size();

    // GDS header record
    dataStream << recordLength;    // record length
    dataStream << (quint16)0x12a0; // record type
    dataStream << (quint16)0x0000; // reserved bytes
    dataStream << (quint8)0x04;    // variable header length
    dataStream << (quint16)0x0000; // flags
    dataStream << (quint8)0x03;    // opcode

    // stream content
    for (int i = 0; i < data.size(); ++i) {
        dataStream << (quint8)data[i];
    }

    return result;
}

QTEST_MAIN(TerminalEmulationTest)

#include "terminalemulationtest.moc"
