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
#include "cursor.h"

namespace q5250 {

Cursor::Cursor() :
    cursorColumn(1),
    cursorRow(1)
{
}

void Cursor::setPosition(unsigned column, unsigned row)
{
    cursorColumn = column;
    cursorRow = row;
}

void Cursor::moveUp()
{
    --cursorRow;

    if (cursorRow < 1)
    {
        cursorRow = 24;
    }
}

void Cursor::moveDown()
{
    ++cursorRow;

    if (cursorRow > 24)
    {
        cursorRow = 1;
    }
}

void Cursor::moveLeft()
{
    --cursorColumn;

    if (cursorColumn < 1)
    {
        cursorColumn = 80;
        moveUp();
    }
}

void Cursor::moveRight()
{
    cursorColumn += 1;

    if (cursorColumn > 80)
    {
        cursorColumn = 1;
        moveDown();
    }
}

} // namespace q5250