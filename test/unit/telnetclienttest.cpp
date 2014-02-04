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

#include <telnet/telnetclient.h>
#include <telnet/telnetconnection.h>
using namespace q5250;

class TelnetConnectionMock : public TelnetConnection
{
public:
    MOCK_METHOD0(readAll, QByteArray());
};

class ATelnetClient : public Test
{
public:
    QByteArray ArbitraryRawData{"A"};
    static const char IAC = '\xff';
};

TEST_F(ATelnetClient, readsDataFromConnectionWhenReceivedReadyRead)
{
    TelnetConnectionMock connection;
    TelnetClient client(&connection);
    EXPECT_CALL(connection, readAll()).WillOnce(Return(ArbitraryRawData));

    client.readyRead();
}

TEST_F(ATelnetClient, emitsDataReceivedForRawData)
{
    TelnetConnectionMock connection;
    TelnetClient client(&connection);
    QSignalSpy spy(&client, SIGNAL(dataReceived(QByteArray)));
    EXPECT_CALL(connection, readAll()).WillOnce(Return(ArbitraryRawData));

    client.readyRead();

    ASSERT_THAT(spy.count(), Eq(1));
    ASSERT_THAT(spy[0][0].toByteArray(), Eq(ArbitraryRawData));
}
