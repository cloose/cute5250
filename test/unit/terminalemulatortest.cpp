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
#include <terminal/formattable.h>
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
    MOCK_METHOD3(setCharacterAt, void(unsigned char, unsigned char, unsigned char));
    MOCK_METHOD3(repeatCharacterToAddress, void(unsigned char, unsigned char, unsigned char));
    MOCK_METHOD1(addField, void(q5250::Field*));
};

class FormatTableMock : public FormatTable
{
public:
    MOCK_METHOD0(clear, void());
    MOCK_METHOD1(append, void(q5250::Field*));
    MOCK_CONST_METHOD2(fieldAt, q5250::Field*(const Cursor &, int));
    MOCK_CONST_METHOD0(isEmpty, bool());
    MOCK_CONST_METHOD1(map, void(std::function<void (q5250::Field*)> func));
};

class TerminalDisplayMock : public TerminalDisplay
{
public:
    MOCK_METHOD0(clear, void());
    MOCK_METHOD3(displayText, void(unsigned char, unsigned char, const QString&));
    MOCK_METHOD1(displayAttribute, void(unsigned char));
    MOCK_METHOD2(displayCursor, void(unsigned char, unsigned char));
};

class ATerminalEmulator : public Test
{
public:
    DisplayBufferMock displayBuffer;
    FormatTableMock formatTable;
    TerminalDisplayMock terminalDisplay;
    TerminalEmulator terminal;

    static const char ESC = 0x04;
    static const char ClearUnitCommand = 0x40;
    static const char WriteToDisplayCommand = 0x11;
    static const char WriteStructuredFieldCommand = 0xf3;

    static const char StartOfHeaderOrder = 0x01;
    static const char RepeatToAddressOrder = 0x02;
    static const char SetBufferAddressOrder = 0x11;
    static const char StartOfFieldOrder = 0x1d;

    static const char GreenAttribute = 0x20;
    static const char GreenUnderlineAttribute = 0x24;
    static const char NonDisplay4Attribute = 0x3f;

    static const char EbcdicBlank = '\x40';

    ATerminalEmulator()
    {
        terminal.setDisplayBuffer(&displayBuffer);
        terminal.setFormatTable(&formatTable);
        terminal.setTerminalDisplay(&terminalDisplay);

        DefaultValue<QSize>::Set(QSize(0, 0));
    }

    QByteArray createGdsHeaderWithLength(char length, char opCode = 0x03)
    {
        char fullLength = 0x0a + length;
        const char gdsHeader[] { 0x00, fullLength, 0x12, (char)0xa0, 0x00, 0x00, 0x04, 0x00, 0x00, opCode };
        return QByteArray(gdsHeader, 10);
    }

    QByteArray createWriteToDisplayCommandWithOrderLength(char length)
    {
        static const char streamData[]{ESC, WriteToDisplayCommand, 0x00, 0x18};
        char fullLength = length + 4;
        return createGdsHeaderWithLength(fullLength) + QByteArray::fromRawData(streamData, 4);
    }

    QByteArray create5250QueryCommand()
    {
        static const char streamData[]{ESC, WriteStructuredFieldCommand, 0x00, 0x05, (char)0xd9, (char)0x70, 0x00};
        return createGdsHeaderWithLength(6) + QByteArray::fromRawData(streamData, 6);
    }

    QByteArray createGeneralDataStream(const QByteArray &data)
    {
        const char opCode = 0x00;
        return createGdsHeaderWithLength(data.size(), opCode) + data;
    }

    QByteArray textAsEbcdic(const QString &text)
    {
        static QTextCodec *codec = QTextCodec::codecForName("IBM500");
        return codec->fromUnicode(text);
    }

    void moveCursorTo(unsigned char column, unsigned char row)
    {
        for (int i = 1; i < column; ++i) {
            terminal.handleKeypress(Qt::Key_Right, QString());
        }

        for (int i = 1; i < row; ++i) {
            terminal.handleKeypress(Qt::Key_Down, QString());
        }
    }
};

static const QString ArbitraryText{"ABC"};

namespace q5250 {

inline bool operator==(const Field &lhs, const Field &rhs)
{
    return lhs.format == rhs.format &&
           lhs.attribute == rhs.attribute &&
           lhs.length == rhs.length &&
           lhs.startColumn == rhs.startColumn &&
           lhs.startRow == rhs.startRow &&
           lhs.content == rhs.content;
}

inline bool operator==(const Cursor &lhs, const Cursor &rhs)
{
    return lhs.column() == rhs.column() &&
           lhs.row() == rhs.row();
}

}

TEST_F(ATerminalEmulator, callsUpdateAfterParsingReceivedData)
{
    EXPECT_CALL(displayBuffer, size()).Times(2).WillRepeatedly(Return(QSize(0, 0)));
    EXPECT_CALL(terminalDisplay, clear()).Times(1);
    EXPECT_CALL(terminalDisplay, displayCursor(_, _)).Times(1);

    terminal.dataReceived(QByteArray());
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

    terminal.parseStreamData(data);
}

TEST_F(ATerminalEmulator, setDisplayBufferToDefaultSizeOnReceivingClearUnit)
{
    const char streamData[]{ESC, ClearUnitCommand};
    QByteArray data = createGdsHeaderWithLength(2) + QByteArray::fromRawData(streamData, 2);

    EXPECT_CALL(displayBuffer, setSize(80, 25));

    terminal.parseStreamData(data);
}

TEST_F(ATerminalEmulator, clearsFormatTableOnReceivingClearUnit)
{
    const char streamData[]{ESC, ClearUnitCommand};
    QByteArray data = createGdsHeaderWithLength(2) + QByteArray::fromRawData(streamData, 2);

    EXPECT_CALL(formatTable, clear());

    terminal.parseStreamData(data);
}

TEST_F(ATerminalEmulator, setDisplayBufferToReceivedAddress)
{
    const char column = 5;
    const char row = 2;
    const char streamData[]{SetBufferAddressOrder, row, column};
    QByteArray data = createWriteToDisplayCommandWithOrderLength(3) + QByteArray::fromRawData(streamData, 3);

    EXPECT_CALL(displayBuffer, setBufferAddress(column, row));

    terminal.parseStreamData(data);
}

TEST_F(ATerminalEmulator, writesCharactersToDisplayBuffer)
{
    const QByteArray ebcdicText = textAsEbcdic(ArbitraryText);
    const QByteArray data = createWriteToDisplayCommandWithOrderLength(ebcdicText.length()) + ebcdicText;

    EXPECT_CALL(displayBuffer, setCharacter(ebcdicText.at(0)));
    EXPECT_CALL(displayBuffer, setCharacter(ebcdicText.at(1)));
    EXPECT_CALL(displayBuffer, setCharacter(ebcdicText.at(2)));

    terminal.parseStreamData(data);
}

TEST_F(ATerminalEmulator, writesAttributesToDisplayBuffer)
{
    const char attributeData[]{GreenAttribute, NonDisplay4Attribute};
    const QByteArray data = createWriteToDisplayCommandWithOrderLength(2) + QByteArray::fromRawData(attributeData, 2);

    EXPECT_CALL(displayBuffer, setCharacter(GreenAttribute));
    EXPECT_CALL(displayBuffer, setCharacter(NonDisplay4Attribute));

    terminal.parseStreamData(data);
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

    terminal.parseStreamData(data);
}

TEST_F(ATerminalEmulator, drawsMultipleTextInBufferOnDisplay)
{
    const QByteArray ebcdicText = textAsEbcdic(ArbitraryText);
    const QByteArray ebcdicText2 = textAsEbcdic(QStringLiteral("DEF"));
    const char startRow = 2;
    const char startColumn = 78;
    const QString firstLine = QString("%1%2").arg(ArbitraryText).arg(' ', 77);
    const QString secondLine = QString("%1%2").arg(' ', 77).arg("DEF");

    EXPECT_CALL(displayBuffer, size()).WillRepeatedly(Return(QSize(80, 2)));
    EXPECT_CALL(displayBuffer, characterAt(_, _)).WillRepeatedly(Return(0x00));
    EXPECT_CALL(displayBuffer, characterAt(1, 1)).WillOnce(Return(ebcdicText.at(0)));
    EXPECT_CALL(displayBuffer, characterAt(2, 1)).WillOnce(Return(ebcdicText.at(1)));
    EXPECT_CALL(displayBuffer, characterAt(3, 1)).WillOnce(Return(ebcdicText.at(2)));
    EXPECT_CALL(displayBuffer, characterAt(startColumn, startRow)).WillOnce(Return(ebcdicText2.at(0)));
    EXPECT_CALL(displayBuffer, characterAt(startColumn+1, startRow)).WillOnce(Return(ebcdicText2.at(1)));
    EXPECT_CALL(displayBuffer, characterAt(startColumn+2, startRow)).WillOnce(Return(ebcdicText2.at(2)));
    EXPECT_CALL(terminalDisplay, clear());
    EXPECT_CALL(terminalDisplay, displayText(1, 1, firstLine));
    EXPECT_CALL(terminalDisplay, displayText(1, startRow, secondLine));
    EXPECT_CALL(terminalDisplay, displayCursor(1, 1));

    terminal.update();
}

TEST_F(ATerminalEmulator, drawsTextWithSurroundingAttributesInBufferOnDisplay)
{
    const QByteArray ebcdicText = textAsEbcdic(ArbitraryText);
    const QString remainingLine(5, ' ');

    EXPECT_CALL(displayBuffer, size()).WillRepeatedly(Return(QSize(10, 1)));
    EXPECT_CALL(displayBuffer, characterAt(_, _)).WillRepeatedly(Return(0x00));
    EXPECT_CALL(displayBuffer, characterAt(1, 1)).WillOnce(Return(GreenAttribute));
    EXPECT_CALL(displayBuffer, characterAt(2, 1)).WillOnce(Return(ebcdicText.at(0)));
    EXPECT_CALL(displayBuffer, characterAt(3, 1)).WillOnce(Return(ebcdicText.at(1)));
    EXPECT_CALL(displayBuffer, characterAt(4, 1)).WillOnce(Return(ebcdicText.at(2)));
    EXPECT_CALL(displayBuffer, characterAt(5, 1)).WillOnce(Return(NonDisplay4Attribute));
    EXPECT_CALL(terminalDisplay, clear());
    EXPECT_CALL(terminalDisplay, displayAttribute(GreenAttribute));
    EXPECT_CALL(terminalDisplay, displayText(2, 1, ArbitraryText));
    EXPECT_CALL(terminalDisplay, displayAttribute(NonDisplay4Attribute));
    EXPECT_CALL(terminalDisplay, displayText(6, 1, remainingLine));
    EXPECT_CALL(terminalDisplay, displayCursor(1, 1));

    terminal.update();
}

TEST_F(ATerminalEmulator, clearsFormatTableOnReceivingStartOfHeader)
{
    const char length = 0;
    const char streamData[]{StartOfHeaderOrder, length};
    QByteArray data = createWriteToDisplayCommandWithOrderLength(2) + QByteArray::fromRawData(streamData, 2);

    EXPECT_CALL(formatTable, clear());

    terminal.parseStreamData(data);
}

TEST_F(ATerminalEmulator, addsOutputFieldToDisplayBuffer)
{
    const char fieldLength = 5;
    const char streamData[]{StartOfFieldOrder, GreenUnderlineAttribute, 0x00, fieldLength};
    const QByteArray data = createWriteToDisplayCommandWithOrderLength(4) + QByteArray::fromRawData(streamData, 4);
    q5250::Field outputField = { .format = 0, .attribute = GreenUnderlineAttribute, .length = fieldLength,
                                 .startColumn = 0, .startRow = 0, .content = QByteArray(fieldLength, EbcdicBlank) };

    EXPECT_CALL(displayBuffer, addField(Pointee(outputField)));

    terminal.parseStreamData(data);
}

TEST_F(ATerminalEmulator, doesNotAddOutputFieldToFormatTable)
{
    const char fieldLength = 5;
    const char streamData[]{StartOfFieldOrder, GreenUnderlineAttribute, 0x00, fieldLength};
    const QByteArray data = createWriteToDisplayCommandWithOrderLength(4) + QByteArray::fromRawData(streamData, 4);

    EXPECT_CALL(formatTable, append(_)).Times(0);

    terminal.parseStreamData(data);
}

TEST_F(ATerminalEmulator, addsInputFieldWithoutControlWordsToDisplayBuffer)
{
    const char fieldLength = 5;
    const char streamData[]{StartOfFieldOrder, 0x40, 0x00, GreenUnderlineAttribute, 0x00, fieldLength};
    const QByteArray data = createWriteToDisplayCommandWithOrderLength(6) + QByteArray::fromRawData(streamData, 6);
    q5250::Field inputField = { .format = 0x4000, .attribute = GreenUnderlineAttribute, .length = fieldLength,
                                .startColumn = 0, .startRow = 0, .content = QByteArray(fieldLength, EbcdicBlank) };

    EXPECT_CALL(displayBuffer, addField(Pointee(inputField)));

    terminal.parseStreamData(data);
}

TEST_F(ATerminalEmulator, addsInputFieldWithoutControlWordsToFormatTable)
{
    const char fieldLength = 5;
    const char streamData[]{StartOfFieldOrder, 0x40, 0x00, GreenUnderlineAttribute, 0x00, fieldLength};
    const QByteArray data = createWriteToDisplayCommandWithOrderLength(6) + QByteArray::fromRawData(streamData, 6);
    q5250::Field inputField = { .format = 0x4000, .attribute = GreenUnderlineAttribute, .length = fieldLength,
                                .startColumn = 0, .startRow = 0, .content = QByteArray(fieldLength, EbcdicBlank) };

    EXPECT_CALL(formatTable, append(Pointee(inputField)));

    terminal.parseStreamData(data);
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

TEST_F(ATerminalEmulator, clearsDisplayOnUpdate)
{
    EXPECT_CALL(terminalDisplay, clear());

    terminal.update();
}

TEST_F(ATerminalEmulator, displaysCursorOnUpdate)
{
    EXPECT_CALL(terminalDisplay, displayCursor(1, 1));

    terminal.update();
}

TEST_F(ATerminalEmulator, callsUpdateAfterHandlingKeypress)
{
    EXPECT_CALL(displayBuffer, size()).Times(2).WillRepeatedly(Return(QSize(0, 0)));
    EXPECT_CALL(terminalDisplay, clear()).Times(1);
    EXPECT_CALL(terminalDisplay, displayCursor(_, _)).Times(1);

    terminal.keyPressed(0, QString());
}

TEST_F(ATerminalEmulator, addsPressedTextKeyToDisplayBufferIfCursorInsideField)
{
    const unsigned char column = 5;
    const unsigned char row = 5;
    moveCursorTo(column, row);
    Cursor cursor; cursor.setPosition(column, row);
    q5250::Field inputField = { .format = 0x4000, .attribute = GreenUnderlineAttribute, .length = 10, .startColumn = column, .startRow = row };
    const QString arbitraryTextKey("A");
    const QByteArray ebcdicText = textAsEbcdic(arbitraryTextKey);

    EXPECT_CALL(displayBuffer, size()).WillRepeatedly(Return(QSize(20, 20)));
    EXPECT_CALL(formatTable, fieldAt(cursor, 20)).WillOnce(Return(&inputField));
    EXPECT_CALL(displayBuffer, setCharacterAt(cursor.column(), cursor.row(), ebcdicText.at(0)));

    terminal.handleKeypress(Qt::Key_A, arbitraryTextKey);
}

TEST_F(ATerminalEmulator, addsTextKeyToFieldContent)
{
    const unsigned char column = 5;
    const unsigned char row = 5;
    moveCursorTo(column, row);
    Cursor cursor; cursor.setPosition(column, row);
    q5250::Field inputField = { .format = 0x4000, .attribute = GreenUnderlineAttribute, .length = 5,
                                .startColumn = column, .startRow = row, .content = QByteArray(5, EbcdicBlank) };
    const QString arbitraryTextKey("A");
    const QByteArray ebcdicText = textAsEbcdic(arbitraryTextKey);

    EXPECT_CALL(displayBuffer, size()).WillRepeatedly(Return(QSize(20, 20)));
    EXPECT_CALL(formatTable, fieldAt(cursor, 20)).WillOnce(Return(&inputField));
    EXPECT_CALL(displayBuffer, setCharacterAt(cursor.column(), cursor.row(), ebcdicText.at(0)));

    terminal.handleKeypress(Qt::Key_A, arbitraryTextKey);

    ASSERT_THAT(inputField.content, Eq(ebcdicText + QByteArray{"\x40\x40\x40\x40"}));
}

TEST_F(ATerminalEmulator, doesNotAddTextKeyToDisplayBufferIfCursorOutsideField)
{
    const unsigned char column = 5;
    const unsigned char row = 5;
    moveCursorTo(column, row);
    Cursor cursor; cursor.setPosition(column, row);
    const QString arbitraryTextKey("A");

    EXPECT_CALL(displayBuffer, size()).WillRepeatedly(Return(QSize(20, 20)));
    EXPECT_CALL(formatTable, fieldAt(cursor, 20)).WillOnce(ReturnNull());
    EXPECT_CALL(displayBuffer, setCharacterAt(_, _, _)).Times(0);

    terminal.handleKeypress(Qt::Key_A, arbitraryTextKey);
}

TEST_F(ATerminalEmulator, doesNotAddTextKeyToDisplayBufferIfFieldIsBypass)
{
    const unsigned char column = 5;
    const unsigned char row = 5;
    moveCursorTo(column, row);
    Cursor cursor; cursor.setPosition(column, row);
    q5250::Field inputField = { .format = 0x6000, .attribute = GreenUnderlineAttribute, .length = 10, .startColumn = column, .startRow = row };
    const QString arbitraryTextKey("A");

    EXPECT_CALL(displayBuffer, size()).WillRepeatedly(Return(QSize(20, 20)));
    EXPECT_CALL(formatTable, fieldAt(cursor, 20)).WillOnce(Return(&inputField));
    EXPECT_CALL(displayBuffer, setCharacterAt(_, _, _)).Times(0);

    terminal.handleKeypress(Qt::Key_A, arbitraryTextKey);
}

TEST_F(ATerminalEmulator, doesNotAddKeyWithEmptyTextToDisplayBuffer)
{
    const QString emptyTextKey("");
    EXPECT_CALL(displayBuffer, setCharacterAt(_, _, _)).Times(0);

    terminal.handleKeypress(0, emptyTextKey);
}

TEST_F(ATerminalEmulator, movesCursorUpOnKeyUp)
{
    terminal.handleKeypress(Qt::Key_Up, QString());

    ASSERT_THAT(terminal.cursorPosition().column(), Eq(1));
    ASSERT_THAT(terminal.cursorPosition().row(), Eq(24));
}

TEST_F(ATerminalEmulator, movesCursorDownOnKeyDown)
{
    terminal.handleKeypress(Qt::Key_Down, QString());

    ASSERT_THAT(terminal.cursorPosition().column(), Eq(1));
    ASSERT_THAT(terminal.cursorPosition().row(), Eq(2));
}

TEST_F(ATerminalEmulator, movesCursorLeftOnKeyLeft)
{
    terminal.handleKeypress(Qt::Key_Left, QString());

    ASSERT_THAT(terminal.cursorPosition().column(), Eq(80));
    ASSERT_THAT(terminal.cursorPosition().row(), Eq(24));
}

TEST_F(ATerminalEmulator, movesCursorRightOnKeyRight)
{
    terminal.handleKeypress(Qt::Key_Right, QString());

    ASSERT_THAT(terminal.cursorPosition().column(), Eq(2));
    ASSERT_THAT(terminal.cursorPosition().row(), Eq(1));
}

TEST_F(ATerminalEmulator, movesCursorRightAfterTextKeyPress)
{
    const unsigned char column = 5;
    const unsigned char row = 5;
    moveCursorTo(column, row);
    q5250::Field inputField = { .format = 0x4000, .attribute = GreenUnderlineAttribute, .length = 10, .startColumn = column, .startRow = row };
    const QString arbitraryTextKey("A");
    ON_CALL(formatTable, fieldAt(_, _)).WillByDefault(Return(&inputField));

    terminal.handleKeypress(Qt::Key_A, arbitraryTextKey);

    ASSERT_THAT(terminal.cursorPosition().column(), Eq(column+1));
    ASSERT_THAT(terminal.cursorPosition().row(), Eq(row));
}

TEST_F(ATerminalEmulator, sendsCursorPositionAndAidByteOnKeyReturn)
{
    QSignalSpy spy(&terminal, SIGNAL(sendData(QByteArray)));
    const QByteArray cursorAndAidBytes {"\x01\x01\xf1"};
    const QByteArray generalDataStream = createGdsHeaderWithLength(cursorAndAidBytes.size(), 0x00) + cursorAndAidBytes;

    terminal.handleKeypress(Qt::Key_Return, QString());

    ASSERT_THAT(spy.count(), Eq(1));
    ASSERT_THAT(spy[0][0].toByteArray(), Eq(generalDataStream));
}

TEST_F(ATerminalEmulator, sendsFieldPositionAndContentOnKeyReturn)
{
    QSignalSpy spy(&terminal, SIGNAL(sendData(QByteArray)));
    const QByteArray fieldPosition {"\x11\x05\x0a"};
    const QByteArray fieldContent {"ABCDE"};
    const QByteArray cursorAndAidBytes {"\x01\x01\xf1"};
    const unsigned streamLength = cursorAndAidBytes.size() + fieldPosition.size() + fieldContent.size();
    const QByteArray generalDataStream = createGdsHeaderWithLength(streamLength, 0x00) + cursorAndAidBytes + fieldPosition + fieldContent;
    q5250::Field inputField = { .format = 0x4000, .attribute = GreenUnderlineAttribute, .length = 5,
                                .startColumn = 10, .startRow = 5, .content = fieldContent };
    EXPECT_CALL(formatTable, map(_)).WillOnce(InvokeArgument<0>(&inputField));

    terminal.handleKeypress(Qt::Key_Return, QString());

    ASSERT_THAT(spy.count(), Eq(1));
    ASSERT_THAT(spy[0][0].toByteArray(), Eq(generalDataStream));
}

#include <QDebug>
TEST_F(ATerminalEmulator, repliesTo5250QueryCommand)
{
    QSignalSpy spy(&terminal, SIGNAL(sendData(QByteArray)));
    const char queryResponse[]{
        0x00, 0x00, (char)0x88,
        0x00, 0x44, (char)0xd9, (char)0x70, (char)0x80,
        0x06, 0x00,                                                                                     // Workstation Control Unit
        0x01, 0x01, 0x00,                                                                               // Code Level
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Reserved
        0x01,                                                                                           // Workstation Type
        (char)0xf5, (char)0xf2, (char)0xf5, (char)0xf1,                                                 // Machine Type
        (char)0xf0, (char)0xf1, (char)0xf1,                                                             // Model Number
        0x02,                                                                                           // Keyboard ID: Standard
        0x00,                                                                                           // Extended Keyboard ID
        0x00,                                                                                           // Reserved
        0x00, 0x00, 0x00, 0x00,                                                                         // Serial Number
        0x01, 0x00,                                                                                     // Input Fields: 256
        0x00,                                                                                           // Control Unit Customization
        0x00, 0x00,                                                                                     // Reserved
        // Device Capabilities
        0x23,                                                                                           // 0010 0011
        0x31,                                                                                           // 0011 0001
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,                                                                                           // Reserved
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,                                                                                           // Reserved
        0x00,
        0x00,
        0x00,                                                                                           // Reserved
        0x00,
        0x00,
        0x00,
        0x00,
        0x00, 0x00, 0x00                                                                                // Reserved
    };

    terminal.parseStreamData(create5250QueryCommand());

    ASSERT_THAT(spy.count(), Eq(1));
    ASSERT_THAT(spy[0][0].toByteArray(), Eq(createGeneralDataStream(QByteArray::fromRawData(queryResponse, 71))));
}
