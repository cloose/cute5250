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
#include <QDataStream>
#include <QEvent>
#include <QTextCodec>

#include "displaybuffer.h"
#include "field.h"
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

void TerminalEmulator::setTerminalDisplay(TerminalDisplay *display)
{
    terminalDisplay = display;
}

void TerminalEmulator::dataReceived(const QByteArray &data)
{
    static const unsigned short HEADER_SIZE = 10;
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
                displayBuffer->clearFormatTable();
                qDeleteAll(fieldList);
                fieldList.clear();
                break;
            case 0x52 /*READ MDT FIELDS*/:
                {
                    foreach (const Field *field, fieldList) {
                        if (!field->isBypassField()) {
                            cursor.setPosition(field->startColumn, field->startRow);
                            break;
                        }
                    }
                }
                break;
            case 0xf3 /*WRITE STRUCTURED FIELD*/:
                {
                    unsigned char length1 = stream.readByte();
                    unsigned char length2 = stream.readByte();
                    unsigned short length = (length1 << 8) | length2;
                    unsigned char commandClass = stream.readByte();
                    unsigned char commandType = stream.readByte();
                    unsigned char flag = stream.readByte();
                    qDebug() << "WSF" << length << hex << showbase << commandClass << commandType << flag;

                    QByteArray reply;
                    QDataStream out(&reply, QIODevice::WriteOnly);

                    out << (quint8)0x00     // cursor row
                        << (quint8)0x00     // cursor column
                        << (quint8)0x88     // Inbound Write Structured Field Aid
                        << (quint8)0x00     // length of query reply
                        << (quint8)0x3a
                        << (quint8)0xd9     // command class
                        << (quint8)0x70     // command type: QUERY
                        << (quint8)0x80     // flag byte
                        << (quint8)0x06
                        << (quint8)0x00
                        << (quint8)0x01
                        << (quint8)0x01
                        << (quint8)0x00
                        << (quint8)0x00     // reserved
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x01;     // display or printer

                    QByteArray term = codec->fromUnicode("5251011");
                    for (int i = 0; i < term.size(); ++i) {
                        out << (quint8)term.at(i);
                    }

                    out << (quint8)0x02     // standard keyboard
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x61
                        << (quint8)0x50
                        << (quint8)0x00
                        << (quint8)0xff
                        << (quint8)0xff
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x23
                        << (quint8)0x31
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00
                        << (quint8)0x00;


                    quint16 dataLength = HEADER_SIZE + reply.size();

                    QByteArray gdsData;
                    QDataStream gds(&gdsData, QIODevice::WriteOnly);

                    // write GDS header (see RFC 1205 section 3)
                    gds << dataLength;              // length
                    gds << (quint16)0x12a0;     // record type GDS
                    gds << (quint16)0x0000;     // reserved
                    gds << (quint8)0x04;        // variable header length
                    gds << (quint16)0x0000;     // flags
                    gds << (quint8)0x00;        // opcode

                    gdsData.append(reply);

                    emit sendData(gdsData);
                }
                break;
            default:
                qWarning() << "UNKNOWN COMMAND" << hex << showbase << byte;
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
    static const unsigned short HEADER_SIZE = 10;

    qDebug() << "KEY PRESSED" << key << text << cursor.column() << cursor.row();

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
            QByteArray buffer;
            QDataStream out(&buffer, QIODevice::WriteOnly);

            out << (quint8)cursor.row() << (quint8)cursor.column() << (quint8)0xf1 /*AID*/;

            foreach (Field *field, fieldList) {
                qDebug() << codec->toUnicode(field->content);
                if (field->format & 0x800 /*is modified?*/) {
                    out << (quint8)0x11 /*SBA*/
                        << (quint8)field->startRow
                        << (quint8)field->startColumn;

                    for (int i = 0; i < field->content.size(); ++i) {
                        out << (quint8)field->content.at(i);
                    }
                }
            }

            quint16 length = HEADER_SIZE + buffer.size();

            QByteArray gdsData;
            QDataStream gds(&gdsData, QIODevice::WriteOnly);

            // write GDS header (see RFC 1205 section 3)
            gds << length;              // length
            gds << (quint16)0x12a0;     // record type GDS
            gds << (quint16)0x0000;     // reserved
            gds << (quint8)0x04;        // variable header length
            gds << (quint16)0x0000;     // flags
            gds << (quint8)0x00;        // opcode

            gdsData.append(buffer);

            emit sendData(gdsData);
        }
        break;
    default:
        if (!text.isEmpty()) {
            unsigned address = cursor.row() * displayBuffer->size().width() + cursor.column();

            Field *currentField = 0;
            foreach (Field *field, fieldList) {
                unsigned startFieldAddress = field->startRow * displayBuffer->size().width() + field->startColumn;
                unsigned endFieldAddress = startFieldAddress + field->length - 1;
                if (address >= startFieldAddress && address <= endFieldAddress) {
                    currentField = field;
                    break;
                }
            }

            if (currentField && !currentField->isBypassField()) {
                QByteArray ebcdic = codec->fromUnicode(text);
                displayBuffer->setBufferAddress(cursor.column(), cursor.row());
                displayBuffer->setCharacter(ebcdic.at(0));
                currentField->setContent(cursor.column(), cursor.row(), ebcdic);
                cursor.moveRight();
            }
        }
        break;
    }

    update();
}

void TerminalEmulator::handleWriteToDisplayCommand(GeneralDataStream &stream)
{
    stream.readByte();
    stream.readByte();

    while (!stream.atEnd()) {
        unsigned char byte = stream.readByte();

        switch (byte) {
        case 0x01 /*START OF HEADER*/:
            {
                unsigned dataLength = stream.readByte();
                qDebug() << "SOH" << dataLength;
                displayBuffer->clearFormatTable();
            }
            break;
        case 0x02 /*REPEAT TO ADDRESS*/:
            {
                unsigned char row = stream.readByte();
                unsigned char column = stream.readByte();
                unsigned char character = stream.readByte();
                qDebug() << "RA" << row << column << (char)character;
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
                qDebug() << "SBA" << row << column;
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

                unsigned char fieldLength1 = stream.readByte();
                unsigned char fieldLength2 = stream.readByte();
                field->setLength((fieldLength1 << 8) | fieldLength2);

                qDebug() << "SF" << hex << showbase << field->format
                                 << field->attribute
                                 << dec << field->length;
                displayBuffer->addField(field);

                if (field->format > 0) {
                    fieldList.append(field);
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
