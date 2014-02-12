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
#include "terminalemulator.h"

#include "displaybuffer.h"
#include "generaldatastream.h"

namespace q5250 {

void TerminalEmulator::setDisplayBuffer(DisplayBuffer *buffer)
{
    displayBuffer = buffer;
}

void TerminalEmulator::dataReceived(const QByteArray &data)
{
    GeneralDataStream stream(data);

    while (!stream.atEnd()) {
        unsigned char byte = stream.readByte();

        if (byte == 0x04 /*ESC*/) {
            byte = stream.readByte();

            switch (byte) {
            case 0x11 /*WRITE TO DISPLAY*/:
                handleWriteToDisplayCommand(stream);
                break;
            case 0x40 /*CLEAR UNIT*/:
                displayBuffer->setSize(80, 25);
                break;
            }
        }
    }
}

void TerminalEmulator::handleWriteToDisplayCommand(GeneralDataStream &stream)
{
    stream.readByte();
    stream.readByte();

    while (!stream.atEnd()) {
        unsigned char byte = stream.readByte();

        switch (byte) {
        case 0x01 /*START OF HEADER*/:
            stream.readByte();
            stream.readByte();
            stream.readByte();
            stream.readByte();
            stream.readByte();
            break;
        case 0x02 /*REPEAT TO ADDRESS*/:
            {
                unsigned char row = stream.readByte();
                unsigned char column = stream.readByte();
                unsigned char character = stream.readByte();
                displayBuffer->repeatCharacterToAddress(column, row, character);
            }
            break;
        case 0x11 /*SET BUFFER ADDRESS*/:
            {
                unsigned char row = stream.readByte();
                unsigned char column = stream.readByte();
                displayBuffer->setBufferAddress(column, row);
            }
            break;
        default:
            displayBuffer->setCharacter(byte);
            break;
        }
    }
}

} // namespace q5250
