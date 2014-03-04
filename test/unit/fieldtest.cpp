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

TEST(AField, reportsIsInputFieldIfMostSignificantBitsEqual01)
{
    q5250::Field field;

    field.format = 0x0000;
    ASSERT_FALSE(field.isInputField());

    field.format = 0xc000;
    ASSERT_FALSE(field.isInputField());

    field.format = 0x4000;
    ASSERT_TRUE(field.isInputField());
}

TEST(AField, reportsIsBypassFieldIfBit13Set)
{
    q5250::Field field;

    field.format = 0x0000;
    ASSERT_FALSE(field.isBypassField());

    field.format = 0x2000;
    ASSERT_TRUE(field.isBypassField());
}

TEST(AField, setsLengthToPassedValue)
{
    q5250::Field field;
    field.length = 0;

    field.setLength(10);

    ASSERT_THAT(field.length, Eq(10));
}
