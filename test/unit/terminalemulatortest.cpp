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
#include <terminal/terminalemulator.h>
using namespace q5250;

class DisplayBufferMock : public DisplayBuffer
{
public:
    MOCK_METHOD2(setSize, void(unsigned char, unsigned char));
    MOCK_METHOD2(setBufferAddress, void(unsigned char, unsigned char));
    MOCK_METHOD1(setCharacter, void(unsigned char));
    MOCK_METHOD3(repeatCharacterToAddress, void(unsigned char, unsigned char, unsigned char));
};


class ATerminalEmulator : public Test
{
public:
    DisplayBufferMock displayBuffer;
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
    }

    QByteArray createGdsHeaderWithLength(char length)
    {
        char fullLength = 0x0a + length;
        const char gdsHeader[] { 0x00, fullLength, 0x12, (char)0xa0, 0x00, 0x00, 0x04, 0x00, 0x00, 0x03 };
        return QByteArray(gdsHeader, 10);
    }

    QByteArray textAsEbcdic(const QString &text)
    {
        static QTextCodec *codec = QTextCodec::codecForName("IBM500");
        return codec->fromUnicode(text);
    }
};

TEST_F(ATerminalEmulator, setDisplayBufferToDefaultSizeOnReceivingClearUnit)
{
    EXPECT_CALL(displayBuffer, setSize(80, 25));
    const char streamData[]{ESC, ClearUnitCommand};
    QByteArray data = createGdsHeaderWithLength(2) + QByteArray::fromRawData(streamData, 2);

    terminal.dataReceived(data);
}

TEST_F(ATerminalEmulator, setDisplayBufferToReceivedAddress)
{
    EXPECT_CALL(displayBuffer, setBufferAddress(5, 2));
    const char streamData[]{ESC, WriteToDisplayCommand, 0x00, 0x18, StartOfHeaderOrder, 0x04, 0x00, 0x00, 0x00, 0x00, SetBufferAddressOrder, 0x02, 0x05};
    QByteArray data = createGdsHeaderWithLength(13) + QByteArray::fromRawData(streamData, 13);

    terminal.dataReceived(data);
}

TEST_F(ATerminalEmulator, writesCharactersToDisplayBuffer)
{
    QByteArray ebcdicText = textAsEbcdic("ABC");
    EXPECT_CALL(displayBuffer, setCharacter(ebcdicText.at(0)));
    EXPECT_CALL(displayBuffer, setCharacter(ebcdicText.at(1)));
    EXPECT_CALL(displayBuffer, setCharacter(ebcdicText.at(2)));
    const char streamData[]{ESC, WriteToDisplayCommand, 0x00, 0x18};
    QByteArray data = createGdsHeaderWithLength(7) + QByteArray::fromRawData(streamData, 4) + ebcdicText;

    terminal.dataReceived(data);
}

TEST_F(ATerminalEmulator, writesAttributesToDisplayBuffer)
{
    EXPECT_CALL(displayBuffer, setCharacter(GreenAttribute));
    EXPECT_CALL(displayBuffer, setCharacter(NonDisplay4Attribute));
    const char streamData[]{ESC, WriteToDisplayCommand, 0x00, 0x18, GreenAttribute, NonDisplay4Attribute};
    QByteArray data = createGdsHeaderWithLength(6) + QByteArray::fromRawData(streamData, 6);

    terminal.dataReceived(data);
}

TEST_F(ATerminalEmulator, repeatsCharactersToReceivedAddress)
{
    QByteArray ebcdicText = textAsEbcdic("-");
    const char startRow = 5;
    const char startColumn = 2;
    const char endRow = 5;
    const char endColumn = 4;
    const char streamData[]{ESC, WriteToDisplayCommand, 0x00, 0x18, StartOfHeaderOrder, 0x04, 0x00, 0x00, 0x00, 0x00, SetBufferAddressOrder, startRow, startColumn, RepeatToAddressOrder, endRow, endColumn};
    QByteArray data = createGdsHeaderWithLength(17) + QByteArray::fromRawData(streamData, 16) + ebcdicText;
    EXPECT_CALL(displayBuffer, setBufferAddress(startColumn, startRow));
    EXPECT_CALL(displayBuffer, repeatCharacterToAddress(endColumn, endRow, ebcdicText.at(0)));

    terminal.dataReceived(data);
}
