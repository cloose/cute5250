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
#include "telnetparser.h"

namespace q5250 {

void TelnetParser::parse(const QByteArray &data)
{
    if (data.isEmpty()) {
        return;
    }

    if (isCommand(data)) {
        int commandLength = parseCommand(data);
        parse(data.mid(commandLength));
    } else {
        emit dataReceived(replaceEscapedIACBytes(data));
    }
}

int TelnetParser::parseCommand(const QByteArray &data)
{
    if (isSubnegotiation(data)) {
        Subnegotiation subnegotiation {
            (TelnetOption)data.at(2),
            (SubnegotiationCommand)data.at(3),
            subnegotiationParameters(data.mid(4))
        };
        emit subnegotiationReceived(subnegotiation);
        return 6;
    }

    if (isOptionNegotiation(data)) {
        OptionNegotiation optionNegotiation {
            (TelnetCommand)data.at(1),
            (TelnetOption)data.at(2)
        };
        emit optionNegotiationReceived(optionNegotiation);
        return 3;
    }

    return 2;
}

QByteArray TelnetParser::replaceEscapedIACBytes(const QByteArray &data)
{
    QByteArray result(data);
    result.replace("\xff\xff", "\xff");
    return result;
}

QByteArray TelnetParser::subnegotiationParameters(const QByteArray &data)
{
    for (int i = 0; i < data.size(); ++i) {
        if (isInterpretAsCommand(data[i]) && isSubnegotiationEndCommand(data[i+1])) {
            return data.mid(0, i);
        }
    }

    return QByteArray();
}

bool TelnetParser::isCommand(const QByteArray &data)
{
    // All TELNET commands consist of at least a two byte sequence:  the
    // "Interpret as Command" (IAC) escape character followed by the code
    // for the command.
    return data.size() >= 2 &&
           isInterpretAsCommand(data.at(0)) &&
           !isInterpretAsCommand(data.at(1));
}

bool TelnetParser::isOptionNegotiation(const QByteArray &data)
{
    // The commands dealing with option negotiation are
    // three byte sequences, the third byte being the code for the option
    // referenced.
    return data.size() >= 3 &&
           isInterpretAsCommand(data.at(0)) &&
           isOptionCommand(data.at(1));
}

bool TelnetParser::isSubnegotiation(const QByteArray &data)
{
    return data.size() >= 6 &&
           isInterpretAsCommand(data.at(0)) &&
           isSubnegotiationBeginCommand(data.at(1));
}

bool TelnetParser::isInterpretAsCommand(unsigned char byte)
{
    return (TelnetCommand)byte == TelnetCommand::IAC;
}

bool TelnetParser::isOptionCommand(unsigned char byte)
{
    TelnetCommand command = (TelnetCommand)byte;
    return command == TelnetCommand::WILL ||
           command == TelnetCommand::WONT ||
           command == TelnetCommand::DO   ||
           command == TelnetCommand::DONT;
}

bool TelnetParser::isSubnegotiationBeginCommand(unsigned char byte)
{
    return (TelnetCommand)byte == TelnetCommand::SB;
}

bool TelnetParser::isSubnegotiationEndCommand(unsigned char byte)
{
    return (TelnetCommand)byte == TelnetCommand::SE;
}

} // namespace q5250
