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

#include <terminal/field.h>
#include <terminal/terminaldisplaybuffer.h>
using namespace q5250;

class ATerminalDisplayBuffer : public Test
{
public:
    ATerminalDisplayBuffer() : displayBuffer(new TerminalDisplayBuffer()) { }
    ~ATerminalDisplayBuffer() { delete displayBuffer; }

    TerminalDisplayBuffer *displayBuffer;
    static const unsigned char ArbitraryCharacter{'A'};
    static const char NormalAttribute = 0x20;
    static const char UnderlineAttribute = 0x24;
};

TEST_F(ATerminalDisplayBuffer, hasSetSize)
{
    QSize newBufferSize(80, 25);

    displayBuffer->setSize(newBufferSize.width(), newBufferSize.height());

    ASSERT_THAT(displayBuffer->size(), Eq(newBufferSize));
}

TEST_F(ATerminalDisplayBuffer, hasSetCharacter)
{
    displayBuffer->setCharacter(ArbitraryCharacter);

    ASSERT_THAT(displayBuffer->characterAt(1, 1), Eq(ArbitraryCharacter));
}

TEST_F(ATerminalDisplayBuffer, incrementsAddressForEachSetCharacter)
{
    displayBuffer->setCharacter(ArbitraryCharacter);
    displayBuffer->setCharacter(ArbitraryCharacter);

    ASSERT_THAT(displayBuffer->characterAt(2, 1), Eq(ArbitraryCharacter));
}

TEST_F(ATerminalDisplayBuffer, handlesAddressOverflowingHeightAndWidth)
{
    displayBuffer->setBufferAddress(80, 25);

    displayBuffer->setCharacter(ArbitraryCharacter);
    displayBuffer->setCharacter(ArbitraryCharacter);

    ASSERT_THAT(displayBuffer->characterAt(1, 1), Eq(ArbitraryCharacter));
}

TEST_F(ATerminalDisplayBuffer, repeatsCharacterFromSetAddressToPassedAddress)
{
    unsigned char startRow = 2;
    unsigned char startColumn = 5;
    unsigned char endRow = 2;
    unsigned char endColumn = 7;
    displayBuffer->setBufferAddress(startColumn, startRow);

    displayBuffer->repeatCharacterToAddress(endColumn, endRow, ArbitraryCharacter);

    ASSERT_THAT(displayBuffer->characterAt(startColumn, startRow), Eq(ArbitraryCharacter));
    ASSERT_THAT(displayBuffer->characterAt(startColumn+1, startRow), Eq(ArbitraryCharacter));
    ASSERT_THAT(displayBuffer->characterAt(endColumn, endRow), Eq(ArbitraryCharacter));
}

TEST_F(ATerminalDisplayBuffer, writesAttributesOfOutputField)
{
    const unsigned short fieldLength = 5;
    const unsigned char startRow = 2;
    const unsigned char startColumn = 5;
    const unsigned char endFieldColumn = startColumn + fieldLength;
    displayBuffer->setBufferAddress(startColumn, startRow);
    q5250::Field outputField = { .format = 0, .attribute = UnderlineAttribute, .length = fieldLength };

    displayBuffer->addField(&outputField);

    ASSERT_THAT(displayBuffer->characterAt(startColumn, startRow), Eq(UnderlineAttribute));
    ASSERT_THAT(displayBuffer->characterAt(endFieldColumn+1, startRow), Eq(NormalAttribute));
}

TEST_F(ATerminalDisplayBuffer, setsStartPositionOfFieldOnAddField)
{
    const unsigned short fieldLength = 5;
    const unsigned char startRow = 2;
    const unsigned char startColumn = 5;
    displayBuffer->setBufferAddress(startColumn, startRow);
    q5250::Field outputField = { .format = 0, .attribute = UnderlineAttribute, .length = fieldLength };

    displayBuffer->addField(&outputField);

    ASSERT_THAT(outputField.startColumn, Eq(startColumn+1));
    ASSERT_THAT(outputField.startRow, Eq(startRow));
}
