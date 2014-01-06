#include <QtTest>

#include <generaldatastream.h>
using q5250::GeneralDataStream;

class GeneralDataStreamTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void copesWithEmptyByteArray();
};


void GeneralDataStreamTest::copesWithEmptyByteArray()
{
    QByteArray data;
    GeneralDataStream stream(data);

    QVERIFY(stream.atEnd());
    QCOMPARE(stream.readByte(), (unsigned char)0);
}

QTEST_APPLESS_MAIN(GeneralDataStreamTest)

#include "generaldatastreamtest.moc"
