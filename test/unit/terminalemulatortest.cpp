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

#include <generaldatastream.h>
using namespace q5250;

class DisplayBuffer
{
public:
    virtual ~DisplayBuffer() {}
    virtual void setSize(int columns, int rows) = 0;
    virtual void setBufferAddress(int column, int row) = 0;
};


class DisplayBufferMock : public DisplayBuffer
{
public:
    MOCK_METHOD2(setSize, void(int, int));
    MOCK_METHOD2(setBufferAddress, void(int, int));
};

class TerminalEmulator
{
public:
    void setDisplayBuffer(DisplayBuffer *buffer) { displayBuffer = buffer; }

    void dataReceived(const QByteArray &data)
    {
        GeneralDataStream stream(data);

        while (!stream.atEnd()) {
            unsigned char byte = stream.readByte();

            if (byte == 0x04 /*ESC*/) {
                byte = stream.readByte();

                switch (byte) {
                case 0x11 /*WRITE TO DISPLAY*/:
                    {
                        stream.readByte();
                        stream.readByte();
                        stream.readByte();
                        stream.readByte();
                        stream.readByte();
                        stream.readByte();
                        stream.readByte();
                        stream.readByte();
                        stream.readByte();
                        unsigned char row = stream.readByte();
                        unsigned char column = stream.readByte();
                        displayBuffer->setBufferAddress(column, row);
                    }
                    break;
                case 0x40 /*CLEAR UNIT*/:
                    displayBuffer->setSize(80, 25);
                    break;
                default:
                    break;
                }
            }
        }
    }

private:
    DisplayBuffer *displayBuffer;
};

class ATerminalEmulator : public Test
{
public:
    static const char ESC = 0x04;
    static const char ClearUnitCommand = 0x40;
    static const char WriteToDisplayCommand = 0x11;
    static const char StartOfHeaderOrder = 0x01;
    static const char SetBufferAddressOrder = 0x11;

    QByteArray createGdsHeaderWithLength(char length)
    {
        char fullLength = 0x0a + length;
        const char gdsHeader[] { 0x00, fullLength, 0x12, (char)0xa0, 0x00, 0x00, 0x04, 0x00, 0x00, 0x03 };
        return QByteArray(gdsHeader, 10);
    }
};

TEST_F(ATerminalEmulator, setDisplayBufferToDefaultSizeOnReceivingClearUnit)
{
    DisplayBufferMock displayBuffer;
    TerminalEmulator terminal;
    terminal.setDisplayBuffer(&displayBuffer);
    EXPECT_CALL(displayBuffer, setSize(80, 25));
    const char streamData[]{ESC, ClearUnitCommand};
    QByteArray data = createGdsHeaderWithLength(2) + QByteArray::fromRawData(streamData, 2);

    terminal.dataReceived(data);
}

TEST_F(ATerminalEmulator, setDisplayBufferToReceivedAddress)
{
    DisplayBufferMock displayBuffer;
    TerminalEmulator terminal;
    terminal.setDisplayBuffer(&displayBuffer);
    EXPECT_CALL(displayBuffer, setBufferAddress(5, 2));
    const char streamData[]{ESC, WriteToDisplayCommand, 0x00, 0x18, StartOfHeaderOrder, 0x04, 0x00, 0x00, 0x00, 0x00, SetBufferAddressOrder, 0x02, 0x05};
    QByteArray data = createGdsHeaderWithLength(13) + QByteArray::fromRawData(streamData, 13);

    terminal.dataReceived(data);
}
