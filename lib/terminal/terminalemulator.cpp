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
                formatTable->clear();
                cursor.setPosition(1, 1);
                break;
            }
        }
    }

    update();
}

void TerminalEmulator::update()
{
    QByteArray text;

    unsigned char startColumn = 0;
    unsigned char startRow = 0;

    int bufferWidth = displayBuffer->size().width();
    int bufferHeight = displayBuffer->size().height();

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
    default:
        QByteArray ebcdic = codec->fromUnicode(text);
        displayBuffer->setCharacter(ebcdic.at(0));
        break;
    }

    update();
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

                field->length = stream.readWord();

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

} // namespace q5250
