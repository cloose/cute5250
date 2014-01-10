/*
 * Copyright (c) 2013, Christian Loose
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
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

#include "telnetcommands.h"

namespace q5250 {

TelnetParser::TelnetParser(QObject *parent) :
    QObject(parent)
{
}

void TelnetParser::parse(const QByteArray &buffer)
{
    int currentPos = 0;

    while (currentPos < buffer.size()) {
        const char byte = buffer.at(currentPos);

        if (Command::isInterpretAsCommand(byte)) {
            currentPos += parseCommand(buffer.mid(currentPos));
        } else {
            QByteArray data = buffer.mid(currentPos);
            currentPos += data.size();
            emit dataReceived(replaceEscapedIACBytes(data));
        }
    }
}

int TelnetParser::parseCommand(const QByteArray &buffer)
{
    if (buffer.isEmpty() && buffer.size() == 1)
        return buffer.size();

    uchar command = buffer.at(1);

    if (buffer.size() >= 4 && command == Command::SB) {
        uchar option = buffer.at(2);
        uchar subnegotationCommand = buffer.at(3);
        emit subnegotationReceived(option, subnegotationCommand);
        return 6;
    }

    if (buffer.size() >= 3 && Command::isOptionCommand(command)) {
        uchar option = buffer.at(2);
        emit optionCommandReceived(command, option);
        return 3;
    }

    return 1;
}

QByteArray TelnetParser::replaceEscapedIACBytes(const QByteArray &data)
{
    QByteArray result(data);
    result.replace("\xff\xff", "\xff");
    return result;
}

} // namespace q5250
