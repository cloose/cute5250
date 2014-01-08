#include <QtTest>

#include <generaldatastream.h>
using q5250::GeneralDataStream;

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

class GeneralDataStreamTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void copesWithEmptyByteArray();
    void reportsAtEndIfContainsOnlyHeader();
    void invalidIfRecordTypeWrong();
    void skipsHeaderWhenReading();
    void canSeekToPreviousByte();
    void doesNotSeekToPreviousByteBeyondStartOfContent();
};


void GeneralDataStreamTest::copesWithEmptyByteArray()
{
    QByteArray data;
    GeneralDataStream stream(data);

    QVERIFY(stream.atEnd());
    QCOMPARE(stream.readByte(), (unsigned char)0);
}

void GeneralDataStreamTest::reportsAtEndIfContainsOnlyHeader()
{
    QByteArray data;

    QDataStream dataStream(&data, QIODevice::WriteOnly);
    // GDS header record
    dataStream << (quint16)0x000a; // record length
    dataStream << (quint16)0x12a0; // record type
    dataStream << (quint16)0x0000; // reserved bytes
    dataStream << (quint8)0x04;    // variable header length
    dataStream << (quint16)0x0000; // flags
    dataStream << (quint8)0x03;    // opcode

    GeneralDataStream stream(data);

    QVERIFY(stream.isValid());
    QVERIFY(stream.atEnd());
    QCOMPARE(stream.readByte(), (unsigned char)0);
}

void GeneralDataStreamTest::invalidIfRecordTypeWrong()
{
    QByteArray data;

    QDataStream dataStream(&data, QIODevice::WriteOnly);
    // GDS header record
    dataStream << (quint16)0x000c; // record length
    dataStream << (quint16)0x9999; // record type
    dataStream << (quint16)0x0000; // reserved bytes
    dataStream << (quint8)0x04;    // variable header length
    dataStream << (quint16)0x0000; // flags
    dataStream << (quint8)0x03;    // opcode

    // stream content (ESC,CU)
    dataStream << (quint8)0x04 << (quint8)0x40;

    GeneralDataStream stream(data);

    QVERIFY(stream.isValid() == false);
}

void GeneralDataStreamTest::skipsHeaderWhenReading()
{
    QByteArray data;

    QDataStream dataStream(&data, QIODevice::WriteOnly);
    // GDS header record
    dataStream << (quint16)0x000c; // record length
    dataStream << (quint16)0x12a0; // record type
    dataStream << (quint16)0x0000; // reserved bytes
    dataStream << (quint8)0x04;    // variable header length
    dataStream << (quint16)0x0000; // flags
    dataStream << (quint8)0x03;    // opcode

    // stream content (ESC,CU)
    dataStream << (quint8)0x04 << (quint8)0x40;

    GeneralDataStream stream(data);

    QCOMPARE(stream.readByte(), (unsigned char)0x04);
    QCOMPARE(stream.readByte(), (unsigned char)0x40);
}

void GeneralDataStreamTest::canSeekToPreviousByte()
{
    QByteArray data;

    QDataStream dataStream(&data, QIODevice::WriteOnly);
    // GDS header record
    dataStream << (quint16)0x000c; // record length
    dataStream << (quint16)0x12a0; // record type
    dataStream << (quint16)0x0000; // reserved bytes
    dataStream << (quint8)0x04;    // variable header length
    dataStream << (quint16)0x0000; // flags
    dataStream << (quint8)0x03;    // opcode

    // stream content (ESC,CU)
    dataStream << (quint8)0x04 << (quint8)0x40;

    GeneralDataStream stream(data);

    stream.readByte();
    stream.seekToPreviousByte();

    QCOMPARE(stream.readByte(), (unsigned char)0x04);
    QCOMPARE(stream.readByte(), (unsigned char)0x40);
}

void GeneralDataStreamTest::doesNotSeekToPreviousByteBeyondStartOfContent()
{
    QByteArray data;

    QDataStream dataStream(&data, QIODevice::WriteOnly);
    // GDS header record
    dataStream << (quint16)0x000c; // record length
    dataStream << (quint16)0x12a0; // record type
    dataStream << (quint16)0x0000; // reserved bytes
    dataStream << (quint8)0x04;    // variable header length
    dataStream << (quint16)0x0000; // flags
    dataStream << (quint8)0x03;    // opcode

    // stream content (ESC,CU)
    dataStream << (quint8)0x04 << (quint8)0x40;

    GeneralDataStream stream(data);

    stream.readByte();
    stream.seekToPreviousByte();
    stream.seekToPreviousByte();

    QCOMPARE(stream.readByte(), (unsigned char)0x04);
    QCOMPARE(stream.readByte(), (unsigned char)0x40);
}

QTEST_APPLESS_MAIN(GeneralDataStreamTest)

#include "generaldatastreamtest.moc"
