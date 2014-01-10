#include <QtTest>

#include <telnetcommands.h>
using namespace q5250;

class TelnetCommandsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void recognizesIACFromByteValue();
    void recognizesOptionCommandsFromByteValue();
};

void TelnetCommandsTest::recognizesIACFromByteValue()
{
    QVERIFY(Command::isInterpretAsCommand(0)   == false);
    QVERIFY(Command::isInterpretAsCommand(128) == false);
    QVERIFY(Command::isInterpretAsCommand(254) == false);
    QVERIFY(Command::isInterpretAsCommand(255) == true);

    QVERIFY(Command::isInterpretAsCommand(Command::IAC));
}

void TelnetCommandsTest::recognizesOptionCommandsFromByteValue()
{
    QVERIFY(Command::isOptionCommand(0)   == false);
    QVERIFY(Command::isOptionCommand(128) == false);
    QVERIFY(Command::isOptionCommand(254) == true);
    QVERIFY(Command::isOptionCommand(255) == false);

    QVERIFY(Command::isOptionCommand(Command::WILL));
    QVERIFY(Command::isOptionCommand(Command::WONT));
    QVERIFY(Command::isOptionCommand(Command::DO));
    QVERIFY(Command::isOptionCommand(Command::DONT));
}

QTEST_APPLESS_MAIN(TelnetCommandsTest)

#include "telnetcommandstest.moc"
