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

#include <QDebug>
#include <QEvent>
#include <QTextCodec>

#include "displaybuffer.h"
#include "field.h"
#include "formattable.h"
#include "generaldatastream.h"
#include "terminaldisplay.h"

namespace q5250 {

TerminalEmulator::TerminalEmulator(QObject *parent) :
    QObject(parent)
{
    codec = QTextCodec::codecForName("IBM500");
}

void TerminalEmulator::setDisplayBuffer(DisplayBuffer *buffer)
{
    displayBuffer = buffer;
}

void TerminalEmulator::setFormatTable(FormatTable *table)
{
    formatTable = table;
}

void TerminalEmulator::setTerminalDisplay(TerminalDisplay *display)
{
    terminalDisplay = display;
}

Cursor TerminalEmulator::cursorPosition() const
{
    return cursor;
}

void TerminalEmulator::parseStreamData(const QByteArray &data)
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
                handleClearUnitCommand();
                break;
            case 0xf3 /*WRITE STRUCTURED FIELD*/:
                handleWriteStructuredFieldCommand(stream);
                break;
            }
        }
    }
}

void TerminalEmulator::handleKeypress(int key, const QString &text)
{
    switch (key) {
    case Qt::Key_Up:
        cursor.moveUp();
        break;
    case Qt::Key_Down:
        cursor.moveDown();
        break;
    case Qt::Key_Left:
        cursor.moveLeft();
        break;
    case Qt::Key_Right:
        cursor.moveRight();
        break;
    case Qt::Key_Return:
        {
            GeneralDataStream stream;

            stream << cursor.row() << cursor.column() << 0xf1 /*AID*/;

            formatTable->map([&](Field* field) {
                stream << 0x11
                       << field->startRow
                       << field->startColumn;

                for (int i = 0; i < field->content.size(); ++i) {
                    stream << field->content.at(i);
                }
            });

            emit sendData(stream.toByteArray());
        }
        break;
    default:
        if (!text.isEmpty()) {
            Field *currentField = formatTable->fieldAt(cursor, displayBuffer->size().width());

            if (currentField && !currentField->isBypassField()) {
                QByteArray ebcdic = codec->fromUnicode(text);
                displayBuffer->setCharacterAt(cursor.column(), cursor.row(), ebcdic.at(0));
                currentField->setContent(cursor, displayBuffer->size().width(), ebcdic);
                cursor.moveRight();
            }
        }
        break;
    }
}

void TerminalEmulator::dataReceived(const QByteArray &data)
{
    parseStreamData(data);
    update();
}

void TerminalEmulator::update()
{
    QByteArray text;

    unsigned char startColumn = 0;
    unsigned char startRow = 0;

    int bufferWidth = displayBuffer->size().width();
    int bufferHeight = displayBuffer->size().height();

    terminalDisplay->clear();

    for (int row = 0; row < bufferHeight; ++row) {
        for (int column = 0; column < bufferWidth; ++column) {
            unsigned char character = displayBuffer->characterAt(column+1, row+1);
            if (character >= 0x20 && character <= 0x3f) {
                if (text.length() > 0) {
                    terminalDisplay->displayText(startColumn, startRow, codec->toUnicode(text));
                    text.clear();
                }
                terminalDisplay->displayAttribute(character);
            } else {
                if (text.isEmpty()) {
                    startColumn = column+1;
                    startRow = row+1;
                }
                text += (character == '\0' ? '\x40': character);
            }
        }

        if (text.length() > 0) {
            terminalDisplay->displayText(startColumn, startRow, codec->toUnicode(text));
            text.clear();
        }
    }

    terminalDisplay->displayCursor(cursor.column(), cursor.row());

    emit updateFinished();
}

void TerminalEmulator::keyPressed(int key, const QString &text)
{
    handleKeypress(key, text);
    update();
}

void TerminalEmulator::handleClearUnitCommand()
{
    displayBuffer->setSize(80, 25);
    formatTable->clear();
    cursor.setPosition(1, 1);
}

void TerminalEmulator::handleWriteToDisplayCommand(GeneralDataStream &stream)
{
    unsigned char cc1 = stream.readByte();
    unsigned char cc2 = stream.readByte();

    qDebug() << bin << showbase << cc1 << cc2;

    while (!stream.atEnd()) {
        unsigned char byte = stream.readByte();

        switch (byte) {
        case 0x01 /*START OF HEADER*/:
            {
                unsigned dataLength = stream.readByte();
                formatTable->clear();
            }
            break;
        case 0x02 /*REPEAT TO ADDRESS*/:
            {
                unsigned char row = stream.readByte();
                unsigned char column = stream.readByte();
                unsigned char character = stream.readByte();
                displayBuffer->repeatCharacterToAddress(column, row, character);
            }
            break;
        case 0x04 /*ESC*/:
            stream.seekToPreviousByte();
            return;
        case 0x11 /*SET BUFFER ADDRESS*/:
            {
                unsigned char row = stream.readByte();
                unsigned char column = stream.readByte();
                displayBuffer->setBufferAddress(column, row);
            }
            break;
        case 0x1d /*START OF FIELD*/:
            {
                Field *field = new Field();

                unsigned char byte = stream.readByte();
                if (byte & 0x40 /*is input field?*/) {
                    unsigned char ffw2 = stream.readByte();
                    field->format = (byte << 8) | ffw2;
                    field->attribute = stream.readByte();
                } else {
                    field->attribute = byte;
                }

                field->setLength(stream.readWord());

                displayBuffer->addField(field);

                if (field->isInputField()) {
                    formatTable->append(field);
                }
            }
            break;
        default:
            displayBuffer->setCharacter(byte);
            break;
        }
    }
}

void TerminalEmulator::handleWriteStructuredFieldCommand(GeneralDataStream &stream)
{
    unsigned short length = stream.readWord();
    unsigned char commandClass = stream.readByte();
    unsigned char commandType = stream.readByte();
    unsigned char flags = stream.readByte();

    qDebug() << "[WSF] len =" << length
             << "class =" << hex << showbase << commandClass
             << "type =" << hex << showbase << commandType
             << "flags =" << bin << showbase << flags;

    // 5250 QUERY command
    if (commandClass == 0xd9 && commandType == 0x70) {
        GeneralDataStream stream;

        // [ROW] [COLUMN] [AID] [Structured Field]
        stream << 0x00                          // cursor row
               << 0x00                          // cursor column
               << 0x88;                         // 5250 QUERY reply

        // [LL] [C] [T] [F1] [Data Field]
        stream << 0x00 << 0x44                  // Total length of structured field
               << 0xd9                          // Command Class
               << 0x70                          // Command Type: 5250 QUERY
               << 0x80;                         // Fixed flag byte for QUERY response

        stream << 0x06 << 0x00                  // Workstation Control Unit
               << 0x01 << 0x01 << 0x00          // Code Level
               << 0x00 << 0x00 << 0x00 << 0x00  // Reserved (16 bytes)
               << 0x00 << 0x00 << 0x00 << 0x00
               << 0x00 << 0x00 << 0x00 << 0x00
               << 0x00 << 0x00 << 0x00 << 0x00
               << 0x01;                         // Workstation Type: Display

        QByteArray machineType = codec->fromUnicode(QStringLiteral("5251"));
        for (int i = 0; i < machineType.size(); ++i) {
            stream << machineType.at(i);
        }

        QByteArray modelNumber = codec->fromUnicode(QStringLiteral("011"));
        for (int i = 0; i < modelNumber.size(); ++i) {
            stream << modelNumber.at(i);
        }

        stream << 0x02                          // Keyboard ID: Standard
               << 0x00                          // Extended Keyboard ID
               << 0x00                          // Reserved
               << 0x00 << 0x00 << 0x00 << 0x00  // Serial Number: None
               << 0x01 << 0x00                  // Maximum Number of Input Fields: 256
               << 0x00                          // Control Unit Customization
                                                //  Bit 7   : host can send a 5250 WSC CUSTOMIZATION command
                                                //  Bit 6   : host can send a 5250 QUERY STATION STATE command
                                                //  Bit 5   : host can send a 5250 WORKSTATION CUSTOMIZATION
                                                //            command to select the SBA code returned in READ
                                                //            commands for displays with ideographic extended
                                                //            attributes
                                                //  Bit 4   : 5250 WORKSTATION CUSTOMIZATION command may
                                                //           be 6 bytes or greater than 8 bytes in length
                                                //  Bits 3-0: Reserved
               << 0x00 << 0x00                  // Reserved (2 bytes)
               // Device Capabilities (22 bytes)
               << 0x23                          // Byte 0 - Operating Capabilities:  0b00100011
                                                //  Bits 7-6: Row 1/Column 1 support (00=No, 01=Limited)
                                                //  Bit 5   : READ MDT ALTERNATE command is supported
                                                //  Bit 4   : Workstation has PA1 and PA2 support
                                                //  Bit 3   : Workstation has PA3 support
                                                //  Bit 2   : Workstation has cursor select support
                                                //  Bit 1   : Move Cursor order, Transparent Data order and
                                                //            Transparent entry field FCW support
                                                //  Bit 0   : READ MODIFIED IMMEDIATE ALTERNATE command
                                                //            is supported
               << 0x31                          // Byte 1 - Display Screen Capabilities: 0b00110001
                                                //  Bits 7-6: Reserved
                                                //  Bit 5   : 27 x 132 screen size is supported
                                                //  Bit 4   : 24 x 80 screen size is supported
                                                //  Bit 3   : SLP is supported
                                                //  Bit 2   : MSR is supported
                                                //  Bits 1-0: color support (00=Monochrome, 01=Color)
               << 0x00                          // Byte 2
               << 0x00                          // Byte 3
               << 0x00                          // Byte 4
               << 0x00                          // Byte 5
               << 0x00                          // Byte 6 - Reserved
               << 0x00                          // Byte 7 - 5250 Image/Fax Support
               << 0x00                          // Byte 8 - 5250 Image/Fax Support
               << 0x00                          // Byte 9
               << 0x00                          // Byte 10
               << 0x00                          // Byte 11 - Reserved
               << 0x00                          // Byte 12 - Number of Grid Line Buffers supported: None
               << 0x00                          // Byte 13 - Type of Grid Line Support: No Support
               << 0x00                          // Byte 14 - Reserved
               << 0x00                          // Byte 15 - Number of Fax or Images: No Support
               << 0x00                          // Byte 16 - Image/Fax Scaling Granularity: No Support
               << 0x00                          // Byte 17 - Image/Fax Rotating Granularity: No Support
               << 0x00                          // Byte 18 - 5250 Image/Fax Support: No Support
               << 0x00 << 0x00 << 0x00;         // Bytes 19-21 - Reserved

        emit sendData(stream.toByteArray());
    }
}

} // namespace q5250
