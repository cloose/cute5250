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

#include <QByteArray>

#include <generaldatastream.h>
using namespace q5250;

TEST(AGeneralDataStream, reportsAtEndIfEmpty)
{
    QByteArray data;
    GeneralDataStream stream(data);
    ASSERT_TRUE(stream.atEnd());
}

TEST(AGeneralDataStream, isValidIfGdsHeaderRecordTypeAndLengthIsCorrect)
{
    const char gdsHeader[] { 0x00, 0x0a, 0x12, (char)0xa0, 0x00, 0x00, 0x04, 0x00, 0x00, 0x03 };
    GeneralDataStream stream(QByteArray::fromRawData(gdsHeader, 10));
    ASSERT_TRUE(stream.isValid());
}

TEST(AGeneralDataStream, startsToReadAfterHeader)
{
    const char gdsHeader[] { 0x00, 0x0c, 0x12, (char)0xa0, 0x00, 0x00, 0x04, 0x00, 0x00, 0x03 };
    QByteArray data = QByteArray::fromRawData(gdsHeader, 10);
    data += 0x04;       // ESC
    data += 0x40;       // CU
    GeneralDataStream stream(data);

    ASSERT_THAT(stream.readByte(), Eq(0x04));
    ASSERT_THAT(stream.readByte(), Eq(0x40));
}
