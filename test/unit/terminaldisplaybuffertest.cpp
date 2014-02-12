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

#include <terminal/terminaldisplaybuffer.h>
using namespace q5250;

class ATerminalDisplayBuffer : public Test
{
public:
    TerminalDisplayBuffer displayBuffer;
};

TEST_F(ATerminalDisplayBuffer, hasSetSize)
{
    QSize newBufferSize(80, 25);

    displayBuffer.setSize(newBufferSize.width(), newBufferSize.height());

    ASSERT_THAT(displayBuffer.size(), Eq(newBufferSize));
}

TEST_F(ATerminalDisplayBuffer, hasSetCharacter)
{
    unsigned char character = 'A';

    displayBuffer.setCharacter(character);

    ASSERT_THAT(displayBuffer.characterAt(1, 1), Eq(character));
}

TEST_F(ATerminalDisplayBuffer, repeatsCharacterFromSetAddressToPassedAddress)
{
    unsigned char character = 'A';
    unsigned char startRow = 2;
    unsigned char startColumn = 5;
    unsigned char endRow = 2;
    unsigned char endColumn = 7;
    displayBuffer.setBufferAddress(startColumn, startRow);

    displayBuffer.repeatCharacterToAddress(endColumn, endRow, character);

    ASSERT_THAT(displayBuffer.characterAt(startColumn, startRow), Eq(character));
    ASSERT_THAT(displayBuffer.characterAt(startColumn+1, startRow), Eq(character));
    ASSERT_THAT(displayBuffer.characterAt(endColumn, endRow), Eq(character));
}
