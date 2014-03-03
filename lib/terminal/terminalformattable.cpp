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
#include "terminalformattable.h"

#include "cursor.h"
#include "field.h"

namespace q5250 {

void TerminalFormatTable::clear()
{
    qDeleteAll(fieldList);
    fieldList.clear();
}

void TerminalFormatTable::append(Field *field)
{
    fieldList.append(field);
}

Field *TerminalFormatTable::fieldAt(const Cursor &cursor, int displayWidth) const
{
    unsigned short cursorAddress = cursor.address();

    Field *resultField = 0;
    foreach (Field *field, fieldList) {
        unsigned startFieldAddress = field->startRow * displayWidth + field->startColumn;
        unsigned endFieldAddress = startFieldAddress + field->length - 1;
        if (cursorAddress >= startFieldAddress && cursorAddress <= endFieldAddress) {
            resultField = field;
            break;
        }
    }

    return resultField;
}

bool TerminalFormatTable::isEmpty() const
{
    return fieldList.isEmpty();
}

} // namespace q5250
