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
#include "telnetclient.h"

#include "telnetconnection.h"
#include <QDebug>

namespace q5250 {

void OutputData(const QByteArray &data, const QString &who = QStringLiteral("--- SERVER ---"))
{
    qDebug() << who;
    for (int i = 0; i < data.size(); ++i) {
        qDebug() << data[i]
                 << hex << showbase << uchar(data[i])
                 << QString::number(uchar(data[i]));
    }
}

TelnetClient::TelnetClient(TelnetConnection *conn) :
    connection(conn),
    terminalType("UNKNOWN")
{
    connect(&parser, &TelnetParser::dataReceived,
            this, &TelnetClient::dataReceived);
    connect(&parser, &TelnetParser::optionNegotiationReceived,
            this, &TelnetClient::optionNegotiationReceived);
    connect(&parser, &TelnetParser::subnegotiationReceived,
            this, &TelnetClient::subnegotiationReceived);
}

void TelnetClient::setTerminalType(const QString &type)
{
    terminalType = type;
}

void TelnetClient::readyRead()
{
    QByteArray buffer = connection->readAll();
    OutputData(buffer);
    parser.parse(buffer);
}

void TelnetClient::sendData(const QByteArray &data)
{
    qDebug() << Q_FUNC_INFO;

    QByteArray ba(data);
    ba.append(0xff);    // IAC
    ba.append(0xef);    // EOR

    connection->write(ba);
}

void TelnetClient::optionNegotiationReceived(const OptionNegotiation &optionNegotiation)
{
    bool supported = isOptionSupported(optionNegotiation.option);
    sendCommand(replyFor(optionNegotiation.command, supported), optionNegotiation.option);
}

void TelnetClient::subnegotiationReceived(const Subnegotiation &subnegotiation)
{
    if (subnegotiation.option == TelnetOption::TERMINAL_TYPE && subnegotiation.command == SubnegotiationCommand::SEND) {
        QByteArray reply;

        reply += (char)TelnetCommand::IAC;
        reply += (char)TelnetCommand::SB;
        reply += (char)TelnetOption::TERMINAL_TYPE;
        reply += (char)SubnegotiationCommand::IS;
        reply += terminalType;
        reply += (char)TelnetCommand::IAC;
        reply += (char)TelnetCommand::SE;

        sendCommand(reply);
    }
}

void TelnetClient::sendCommand(TelnetCommand command, TelnetOption option)
{
    QByteArray reply;
    reply.append((char)TelnetCommand::IAC);
    reply.append((char)command);
    reply.append((char)option);

    sendCommand(reply);
}

void TelnetClient::sendCommand(const QByteArray &command)
{
    OutputData(command, "--- CLIENT ---");
    connection->write(command);
}

bool TelnetClient::isOptionSupported(TelnetOption option)
{
    // the following telnet options are supported
    return option == TelnetOption::END_OF_RECORD ||
           option == TelnetOption::NEW_ENVIRON   ||
           option == TelnetOption::TERMINAL_TYPE ||
           option == TelnetOption::TRANSMIT_BINARY;
}

TelnetCommand TelnetClient::replyFor(TelnetCommand command, bool supported)
{
    switch (command) {
    case TelnetCommand::DO:
        return supported ? TelnetCommand::WILL : TelnetCommand::WONT;

    case TelnetCommand::WILL:
        return supported ? TelnetCommand::DO : TelnetCommand::DONT;
    }
}

} // namespace q5250
