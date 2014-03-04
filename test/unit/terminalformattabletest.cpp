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

#include <terminal/cursor.h>
#include <terminal/field.h>
#include <terminal/terminalformattable.h>
using namespace q5250;

class ATerminalFormatTable : public Test
{
public:
    TerminalFormatTable formatTable;

    const unsigned int ArbitraryDisplayWidth = 80;
};

TEST_F(ATerminalFormatTable, isEmptyAfterClear)
{
    formatTable.clear();

    ASSERT_TRUE(formatTable.isEmpty());
}

TEST_F(ATerminalFormatTable, isNotEmptyAfterFieldAppended)
{
    q5250::Field field;

    formatTable.append(&field);

    ASSERT_FALSE(formatTable.isEmpty());
}

TEST_F(ATerminalFormatTable, returnsNullForFieldAtIfEmpty)
{
    Cursor cursor;
    formatTable.clear();

    ASSERT_THAT(formatTable.fieldAt(cursor, ArbitraryDisplayWidth), IsNull());
}

TEST_F(ATerminalFormatTable, returnsNullForFieldAtIfCursorNotInside)
{
    Cursor cursor;
    q5250::Field field = { .format = 0x0000, .attribute = 0x00, .length = 1, .startColumn = 5, .startRow = 5 };
    formatTable.append(&field);

    ASSERT_THAT(formatTable.fieldAt(cursor, ArbitraryDisplayWidth), IsNull());
}

TEST_F(ATerminalFormatTable, returnsFieldAtCursorPosition)
{
    const unsigned char column = 5;
    const unsigned char row = 5;
    Cursor cursor; cursor.setPosition(column, row);
    q5250::Field field = { .format = 0x0000, .attribute = 0x00, .length = 1, .startColumn = column, .startRow = row };
    formatTable.append(&field);

    ASSERT_THAT(formatTable.fieldAt(cursor, ArbitraryDisplayWidth), Eq(&field));
}

TEST_F(ATerminalFormatTable, appliesPassedFunctionToEachElementOfFieldList)
{
    unsigned count = 0;
    q5250::Field field1, field2;
    formatTable.append(&field1);
    formatTable.append(&field2);

    formatTable.map([&](q5250::Field *field) {
        ++count;
    });

    ASSERT_THAT(count, Eq(2));
}
