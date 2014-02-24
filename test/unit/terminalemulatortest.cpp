/*
 * Copyright (c) 2013-2014, Christian Loose
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <gmock/gmock.h>
using namespace testing;

#include <QByteArray>
#include <QSignalSpy>
#include <QTextCodec>

#include <terminal/displaybuffer.h>
#include <terminal/field.h>
#include <terminal/terminaldisplay.h>
#include <terminal/terminalemulator.h>
using namespace q5250;

class DisplayBufferMock : public DisplayBuffer
{
public:
    MOCK_CONST_METHOD0(size, QSize());
    MOCK_METHOD2(setSize, void(unsigned char, unsigned char));
    MOCK_METHOD2(setBufferAddress, void(unsigned char, unsigned char));
    MOCK_CONST_METHOD2(characterAt, unsigned char(unsigned char, unsigned char));
    MOCK_METHOD1(setCharacter, void(unsigned char));
    MOCK_METHOD3(repeatCharacterToAddress, void(unsigned char, unsigned char, unsigned char));
    MOCK_METHOD0(clearFormatTable, void());
    MOCK_METHOD1(addField, void(q5250::Field*));
};

class TerminalDisplayMock : public TerminalDisplay
{
public:
    MOCK_METHOD3(displayText, void(unsigned char, unsigned char, const QString&));
    MOCK_METHOD1(displayAttribute, void(unsigned char));
};

class ATerminalEmulator : public Test
{
public:
    DisplayBufferMock displayBuffer;
    TerminalDisplayMock terminalDisplay;
    TerminalEmulator terminal;

    static const char ESC = 0x04;
    static const char ClearUnitCommand = 0x40;
    static const char WriteToDisplayCommand = 0x11;

    static const char StartOfHeaderOrder = 0x01;
    static const char RepeatToAddressOrder = 0x02;
    static const char SetBufferAddressOrder = 0x11;
    static const char StartOfFieldOrder = 0x1d;

    static const char GreenAttribute = 0x20;
    static const char GreenUnderlineAttribute = 0x24;
    static const char NonDisplay4Attribute = 0x3f;

    ATerminalEmulator()
    {
        terminal.setDisplayBuffer(&displayBuffer);
        terminal.setTerminalDisplay(&terminalDisplay);

        DefaultValue<QSize>::Set(QSize(0, 0));
    }

    QByteArray createGdsHeaderWithLength(char length)
    {
        char fullLength = 0x0a + length;
        const char gdsHeader[] { 0x00, fullLength, 0x12, (char)0xa0, 0x00, 0x00, 0x04, 0x00, 0x00, 0x03 };
        return QByteArray(gdsHeader, 10);
    }

    QByteArray createWriteToDisplayCommandWithOrderLength(char length)
    {
        static const char streamData[]{ESC, WriteToDisplayCommand, 0x00, 0x18};
        char fullLength = length + 4;
        return createGdsHeaderWithLength(fullLength) + QByteArray::fromRawData(streamData, 4);
    }

    QByteArray textAsEbcdic(const QString &text)
    {
        static QTextCodec *codec = QTextCodec::codecForName("IBM500");
        return codec->fromUnicode(text);
    }
};

static const QString ArbitraryText{"ABC"};

namespace q5250 {

inline bool operator==(const Field &lhs, const Field &rhs)
{
    return lhs.attribute == rhs.attribute &&
           lhs.length == rhs.length;
}

}

TEST_F(ATerminalEmulator, handlesMultipleCommandsInReceivedData)
{
    const char writeToDisplayCommand[]{ESC, WriteToDisplayCommand, 0x00, 0x18, 'A'};
    const char clearUnitCommand[]{ESC, ClearUnitCommand};
    const QByteArray data = createGdsHeaderWithLength(7)
                          + QByteArray::fromRawData(writeToDisplayCommand, 5)
                          + QByteArray::fromRawData(clearUnitCommand, 2);

    EXPECT_CALL(displayBuffer, setCharacter('A'));
    EXPECT_CALL(displayBuffer, setSize(80, 25));

    terminal.dataReceived(data);
}

TEST_F(ATerminalEmulator, setDisplayBufferToDefaultSizeOnReceivingClearUnit)
{
    const char streamData[]{ESC, ClearUnitCommand};
    QByteArray data = createGdsHeaderWithLength(2) + QByteArray::fromRawData(streamData, 2);

    EXPECT_CALL(displayBuffer, setSize(80, 25));

    terminal.dataReceived(data);
}

TEST_F(ATerminalEmulator, clearsFormatTableOnReceivingClearUnit)
{
    const char streamData[]{ESC, ClearUnitCommand};
    QByteArray data = createGdsHeaderWithLength(2) + QByteArray::fromRawData(streamData, 2);

    EXPECT_CALL(displayBuffer, clearFormatTable());

    terminal.dataReceived(data);
}

TEST_F(ATerminalEmulator, setDisplayBufferToReceivedAddress)
{
    const char column = 5;
    const char row = 2;
    const char streamData[]{SetBufferAddressOrder, row, column};
    QByteArray data = createWriteToDisplayCommandWithOrderLength(3) + QByteArray::fromRawData(streamData, 3);

    EXPECT_CALL(displayBuffer, setBufferAddress(column, row));

    terminal.dataReceived(data);
}

TEST_F(ATerminalEmulator, writesCharactersToDisplayBuffer)
{
    const QByteArray ebcdicText = textAsEbcdic(ArbitraryText);
    const QByteArray data = createWriteToDisplayCommandWithOrderLength(ebcdicText.length()) + ebcdicText;

    EXPECT_CALL(displayBuffer, setCharacter(ebcdicText.at(0)));
    EXPECT_CALL(displayBuffer, setCharacter(ebcdicText.at(1)));
    EXPECT_CALL(displayBuffer, setCharacter(ebcdicText.at(2)));

    terminal.dataReceived(data);
}

TEST_F(ATerminalEmulator, writesAttributesToDisplayBuffer)
{
    const char attributeData[]{GreenAttribute, NonDisplay4Attribute};
    const QByteArray data = createWriteToDisplayCommandWithOrderLength(2) + QByteArray::fromRawData(attributeData, 2);

    EXPECT_CALL(displayBuffer, setCharacter(GreenAttribute));
    EXPECT_CALL(displayBuffer, setCharacter(NonDisplay4Attribute));

    terminal.dataReceived(data);
}

TEST_F(ATerminalEmulator, repeatsCharactersToReceivedAddress)
{
    const QByteArray ebcdicText = textAsEbcdic("-");
    const char startRow = 5;
    const char startColumn = 2;
    const char endRow = 5;
    const char endColumn = 4;
    const char streamData[]{SetBufferAddressOrder, startRow, startColumn, RepeatToAddressOrder, endRow, endColumn};
    const QByteArray data = createWriteToDisplayCommandWithOrderLength(7) + QByteArray::fromRawData(streamData, 6) + ebcdicText;

    EXPECT_CALL(displayBuffer, setBufferAddress(startColumn, startRow));
    EXPECT_CALL(displayBuffer, repeatCharacterToAddress(endColumn, endRow, ebcdicText.at(0)));

    terminal.dataReceived(data);
}

TEST_F(ATerminalEmulator, drawsMultipleTextInBufferOnDisplay)
{
    const QByteArray ebcdicText = textAsEbcdic(ArbitraryText);
    const QByteArray ebcdicText2 = textAsEbcdic(QStringLiteral("DEF"));
    const char startRow = 2;
    const char startColumn = 78;
    const char sbaStreamData[] { SetBufferAddressOrder, startRow, startColumn };
    const QByteArray data = createWriteToDisplayCommandWithOrderLength(9)
                          + ebcdicText
                          + QByteArray::fromRawData(sbaStreamData, 3)
                          + ebcdicText2;
    const QString firstLine = QString("%1%2").arg(ArbitraryText).arg(' ', 77);
    const QString secondLine = QString("%1%2").arg(' ', 77).arg("DEF");

    ON_CALL(displayBuffer, size()).WillByDefault(Return(QSize(80, 2)));
    EXPECT_CALL(displayBuffer, characterAt(_, _)).WillRepeatedly(Return(0x00));
    EXPECT_CALL(displayBuffer, characterAt(1, 1)).WillOnce(Return(ebcdicText.at(0)));
    EXPECT_CALL(displayBuffer, characterAt(2, 1)).WillOnce(Return(ebcdicText.at(1)));
    EXPECT_CALL(displayBuffer, characterAt(3, 1)).WillOnce(Return(ebcdicText.at(2)));
    EXPECT_CALL(displayBuffer, characterAt(startColumn, startRow)).WillOnce(Return(ebcdicText2.at(0)));
    EXPECT_CALL(displayBuffer, characterAt(startColumn+1, startRow)).WillOnce(Return(ebcdicText2.at(1)));
    EXPECT_CALL(displayBuffer, characterAt(startColumn+2, startRow)).WillOnce(Return(ebcdicText2.at(2)));
    EXPECT_CALL(terminalDisplay, displayText(1, 1, firstLine));
    EXPECT_CALL(terminalDisplay, displayText(1, startRow, secondLine));

    terminal.dataReceived(data);
}

TEST_F(ATerminalEmulator, drawsTextWithSurroundingAttributesInBufferOnDisplay)
{
    const QByteArray ebcdicText = textAsEbcdic(ArbitraryText);
    const QByteArray data = createWriteToDisplayCommandWithOrderLength(5)
                          + QByteArrayLiteral(GreenAttribute)
                          + ebcdicText
                          + QByteArrayLiteral(NonDisplay4Attribute);
    const QString remainingLine(5, ' ');

    ON_CALL(displayBuffer, size()).WillByDefault(Return(QSize(10, 1)));
    EXPECT_CALL(displayBuffer, characterAt(_, _)).WillRepeatedly(Return(0x00));
    EXPECT_CALL(displayBuffer, characterAt(1, 1)).WillOnce(Return(GreenAttribute));
    EXPECT_CALL(displayBuffer, characterAt(2, 1)).WillOnce(Return(ebcdicText.at(0)));
    EXPECT_CALL(displayBuffer, characterAt(3, 1)).WillOnce(Return(ebcdicText.at(1)));
    EXPECT_CALL(displayBuffer, characterAt(4, 1)).WillOnce(Return(ebcdicText.at(2)));
    EXPECT_CALL(displayBuffer, characterAt(5, 1)).WillOnce(Return(NonDisplay4Attribute));
    EXPECT_CALL(terminalDisplay, displayAttribute(GreenAttribute));
    EXPECT_CALL(terminalDisplay, displayText(2, 1, ArbitraryText));
    EXPECT_CALL(terminalDisplay, displayAttribute(NonDisplay4Attribute));
    EXPECT_CALL(terminalDisplay, displayText(6, 1, remainingLine));

    terminal.dataReceived(data);
}

TEST_F(ATerminalEmulator, clearsFormatTableOnReceivingStartOfHeader)
{
    const char length = 0;
    const char streamData[]{StartOfHeaderOrder, length};
    QByteArray data = createWriteToDisplayCommandWithOrderLength(2) + QByteArray::fromRawData(streamData, 2);

    EXPECT_CALL(displayBuffer, clearFormatTable());

    terminal.dataReceived(data);
}

TEST_F(ATerminalEmulator, addsOutputFieldToDisplayBuffer)
{
    const char fieldLength = 5;
    const char streamData[]{StartOfFieldOrder, GreenUnderlineAttribute, 0x00, fieldLength};
    const QByteArray data = createWriteToDisplayCommandWithOrderLength(4) + QByteArray::fromRawData(streamData, 4);
    q5250::Field outputField = { .format = 0, .attribute = GreenUnderlineAttribute, .length = fieldLength };

    EXPECT_CALL(displayBuffer, addField(Pointee(outputField)));

    terminal.dataReceived(data);
}

TEST_F(ATerminalEmulator, addsInputFieldWithoutControlWordsToDisplayBuffer)
{
    const char fieldLength = 5;
    const char streamData[]{StartOfFieldOrder, 0x40, 0x00, GreenUnderlineAttribute, 0x00, fieldLength};
    const QByteArray data = createWriteToDisplayCommandWithOrderLength(6) + QByteArray::fromRawData(streamData, 6);
    q5250::Field inputField = { .format = 0x4000, .attribute = GreenUnderlineAttribute, .length = fieldLength };

    EXPECT_CALL(displayBuffer, addField(Pointee(inputField)));

    terminal.dataReceived(data);
}

TEST_F(ATerminalEmulator, emitsUpdateFinishedAfterUpdate)
{
    QSignalSpy spy(&terminal, SIGNAL(updateFinished()));
    const QByteArray ebcdicText = textAsEbcdic("A");
    displayBuffer.setCharacter(ebcdicText.at(0));

    ON_CALL(displayBuffer, size()).WillByDefault(Return(QSize(1, 1)));
    EXPECT_CALL(displayBuffer, characterAt(1, 1)).WillOnce(Return(ebcdicText.at(0)));
    EXPECT_CALL(terminalDisplay, displayText(1, 1, QString("A")));

    terminal.update();

    ASSERT_THAT(spy.count(), Eq(1));
}

TEST_F(ATerminalEmulator, DISABLED_addsPressedTextKeyToDisplayBuffer)
{
    const QString arbitraryTextKey("A");
    const QByteArray ebcdicText = textAsEbcdic(arbitraryTextKey);
    EXPECT_CALL(displayBuffer, setCharacter(ebcdicText.at(0)));

    terminal.keyPressed(Qt::Key_A, arbitraryTextKey);
}
