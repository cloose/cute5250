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
#ifndef Q5250_TERMINALDISPLAYBUFFER_H
#define Q5250_TERMINALDISPLAYBUFFER_H

#include "q5250_global.h"
#include "displaybuffer.h"

class QByteArray;

namespace q5250 {

class Q5250SHARED_EXPORT TerminalDisplayBuffer : public DisplayBuffer
{
public:
    TerminalDisplayBuffer();
    ~TerminalDisplayBuffer();

    QSize size() const;
    void setSize(unsigned char columns, unsigned char rows);

    void setBufferAddress(unsigned char column, unsigned char row);

    unsigned char characterAt(unsigned char column, unsigned char row) const;
    void setCharacter(unsigned char character);
    void repeatCharacterToAddress(unsigned char column, unsigned char row, unsigned char character);

    void clearFormatTable();
    void addOutputField(unsigned char attribute, unsigned short length);

private:
    unsigned int convertToAddress(unsigned char column, unsigned char row) const;
    void increaseBufferAddress(unsigned char increment = 1);

    unsigned char addressColumn;
    unsigned char addressRow;
    QSize bufferSize;
    QByteArray *buffer;
};

} // namespace q5250

#endif // Q5250_TERMINALDISPLAYBUFFER_H
