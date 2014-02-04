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
#include <QSignalSpy>

#include <telnet/telnetcommand.h>
#include <telnet/telnetoption.h>
#include <telnet/telnetparser.h>
using namespace q5250;

class ATelnetParser : public Test
{
public:
    TelnetParser parser;
    QByteArray ArbitraryRawData{"A"};
    QByteArray ArbitraryTelnetCommand{"\xff\xf1"};
    static const char IAC = '\xff';

    QByteArray doOption(TelnetOption option) {
        QByteArray optionNegotiation;

        optionNegotiation.append((char)TelnetCommand::IAC);
        optionNegotiation.append((char)TelnetCommand::DO);
        optionNegotiation.append((char)option);

        return optionNegotiation;
    }
};

TEST_F(ATelnetParser, emitsDataReceivedWhenParsingRawData)
{
    QSignalSpy spy(&parser, SIGNAL(dataReceived(QByteArray)));

    parser.parse(ArbitraryRawData);

    ASSERT_THAT(spy.count(), Eq(1));
    ASSERT_THAT(spy[0][0].toByteArray(), Eq(ArbitraryRawData));
}

TEST_F(ATelnetParser, replacesEscapedIACBytesInRawData)
{
    QSignalSpy spy(&parser, SIGNAL(dataReceived(QByteArray)));
    const char rawData[]{IAC, IAC, 'A', 'B'};
    const char expectedData[]{IAC, 'A', 'B'};

    parser.parse(QByteArray::fromRawData(rawData, 4));

    ASSERT_THAT(spy.count(), Eq(1));
    ASSERT_THAT(spy[0][0].toByteArray(), Eq(QByteArray::fromRawData(expectedData, 3)));
}

TEST_F(ATelnetParser, doesNotEmitDataReceivedForCommands)
{
    QSignalSpy spy(&parser, SIGNAL(dataReceived(QByteArray)));

    parser.parse(ArbitraryTelnetCommand);

    ASSERT_THAT(spy.count(), Eq(0));
}

TEST_F(ATelnetParser, emitsOptionNegotiationReceivedForAnOptionCommand)
{
    QSignalSpy spy(&parser, SIGNAL(optionNegotiationReceived(q5250::TelnetCommand, q5250::TelnetOption)));
    const QByteArray data = doOption(TelnetOption::TRANSMIT_BINARY);

    parser.parse(data);

    ASSERT_THAT(spy.count(), Eq(1));
    ASSERT_THAT(spy[0][0].value<TelnetCommand>(), Eq(TelnetCommand::DO));
    ASSERT_THAT(spy[0][1].value<TelnetOption>(), Eq(TelnetOption::TRANSMIT_BINARY));
}
