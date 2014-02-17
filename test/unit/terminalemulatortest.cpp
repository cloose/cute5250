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
#include <QTextCodec>

#include <terminal/displaybuffer.h>
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
    static const char GreenAttribute = 0x20;
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

TEST_F(ATerminalEmulator, setDisplayBufferToDefaultSizeOnReceivingClearUnit)
{
    const char streamData[]{ESC, ClearUnitCommand};
    QByteArray data = createGdsHeaderWithLength(2) + QByteArray::fromRawData(streamData, 2);

    EXPECT_CALL(displayBuffer, setSize(80, 25));

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
    const QString text2("DEF");
    const QByteArray ebcdicText = textAsEbcdic(ArbitraryText);
    const QByteArray ebcdicText2 = textAsEbcdic(text2);
    const char startRow = 25;
    const char startColumn = 78;
    const char sbaStreamData[] { SetBufferAddressOrder, startRow, startColumn };
    const QByteArray data = createWriteToDisplayCommandWithOrderLength(9)
                          + ebcdicText
                          + QByteArray::fromRawData(sbaStreamData, 3)
                          + ebcdicText2;

    ON_CALL(displayBuffer, size()).WillByDefault(Return(QSize(80, 25)));
    EXPECT_CALL(displayBuffer, characterAt(_, _)).WillRepeatedly(Return(0x00));
    EXPECT_CALL(displayBuffer, characterAt(1, 1)).WillOnce(Return(ebcdicText.at(0)));
    EXPECT_CALL(displayBuffer, characterAt(2, 1)).WillOnce(Return(ebcdicText.at(1)));
    EXPECT_CALL(displayBuffer, characterAt(3, 1)).WillOnce(Return(ebcdicText.at(2)));
    EXPECT_CALL(displayBuffer, characterAt(startColumn, startRow)).WillOnce(Return(ebcdicText2.at(0)));
    EXPECT_CALL(displayBuffer, characterAt(startColumn+1, startRow)).WillOnce(Return(ebcdicText2.at(1)));
    EXPECT_CALL(displayBuffer, characterAt(startColumn+2, startRow)).WillOnce(Return(ebcdicText2.at(2)));
    EXPECT_CALL(terminalDisplay, displayText(1, 1, ArbitraryText));
    EXPECT_CALL(terminalDisplay, displayText(startColumn, startRow, text2));

    terminal.dataReceived(data);
}

TEST_F(ATerminalEmulator, drawsTextWithSurroundingAttributesInBufferOnDisplay)
{
    const QByteArray ebcdicText = textAsEbcdic(ArbitraryText);
    const QByteArray data = createWriteToDisplayCommandWithOrderLength(5)
                          + QByteArrayLiteral(GreenAttribute)
                          + ebcdicText
                          + QByteArrayLiteral(NonDisplay4Attribute);

    ON_CALL(displayBuffer, size()).WillByDefault(Return(QSize(10, 10)));
    EXPECT_CALL(displayBuffer, characterAt(_, _)).WillRepeatedly(Return(0x00));
    EXPECT_CALL(displayBuffer, characterAt(1, 1)).WillOnce(Return(GreenAttribute));
    EXPECT_CALL(displayBuffer, characterAt(2, 1)).WillOnce(Return(ebcdicText.at(0)));
    EXPECT_CALL(displayBuffer, characterAt(3, 1)).WillOnce(Return(ebcdicText.at(1)));
    EXPECT_CALL(displayBuffer, characterAt(4, 1)).WillOnce(Return(ebcdicText.at(2)));
    EXPECT_CALL(displayBuffer, characterAt(5, 1)).WillOnce(Return(NonDisplay4Attribute));
    EXPECT_CALL(terminalDisplay, displayAttribute(GreenAttribute));
    EXPECT_CALL(terminalDisplay, displayText(2, 1, ArbitraryText));
    EXPECT_CALL(terminalDisplay, displayAttribute(NonDisplay4Attribute));

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
