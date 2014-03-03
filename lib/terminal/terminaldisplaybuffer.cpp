/*
 * Copyright (c) 2014, Christian Loose
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
#include "terminaldisplaybuffer.h"

#include <QByteArray>

#include "field.h"

namespace q5250 {

TerminalDisplayBuffer::TerminalDisplayBuffer() :
    addressColumn(1),
    addressRow(1),
    buffer(0)
{
    setSize(80, 25);
}

TerminalDisplayBuffer::~TerminalDisplayBuffer()
{
    delete buffer;
}

QSize TerminalDisplayBuffer::size() const
{
    return bufferSize;
}

void TerminalDisplayBuffer::setSize(unsigned char columns, unsigned char rows)
{
    delete buffer;

    bufferSize.setWidth(columns);
    bufferSize.setHeight(rows);

    buffer = new QByteArray(columns*rows, '\0');
}

void TerminalDisplayBuffer::setBufferAddress(unsigned char column, unsigned char row)
{
    addressColumn = column;
    addressRow = row;
}

unsigned char TerminalDisplayBuffer::characterAt(unsigned char column, unsigned char row) const
{
    unsigned int address = convertToAddress(column, row);
    return buffer->at(address);
}

void TerminalDisplayBuffer::setCharacter(unsigned char character)
{
    setCharacterAt(addressColumn, addressRow, character);
}

void TerminalDisplayBuffer::setCharacterAt(unsigned char column, unsigned char row, unsigned char character)
{
    unsigned int address = convertToAddress(column, row);
    (*buffer)[address] = character;
    increaseBufferAddress();
}

void TerminalDisplayBuffer::repeatCharacterToAddress(unsigned char column, unsigned char row, unsigned char character)
{
    unsigned int fromAddress = convertToAddress(addressColumn, addressRow);
    unsigned int toAddress = convertToAddress(column, row);
    unsigned int numberOfCharacters = toAddress - fromAddress + 1;

    for (int i = 0; i < numberOfCharacters; ++i) {
        setCharacter(character);
    }
}

void TerminalDisplayBuffer::addField(Field *field)
{
    setCharacter(field->attribute);

    field->startColumn = addressColumn;
    field->startRow    = addressRow;

    increaseBufferAddress(field->length);

    // FIXME: replace with enum
    setCharacter(0x20);
}

unsigned int TerminalDisplayBuffer::convertToAddress(unsigned char column, unsigned char row) const
{
    return (row-1) * bufferSize.width() + (column-1);
}

void TerminalDisplayBuffer::increaseBufferAddress(unsigned char increment)
{
    addressColumn += increment;

    if (addressColumn > bufferSize.width()) {
        addressColumn = 1;
        addressRow += 1;
    }

    if (addressRow > bufferSize.height()) {
        addressRow = 1;
    }
}

} // namespace q5250
