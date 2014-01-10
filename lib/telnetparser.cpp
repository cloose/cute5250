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

namespace q5250 {

TelnetParser::TelnetParser(QObject *parent) :
    QObject(parent)
{
}

void TelnetParser::parse(const QByteArray &data)
{
    int currentPos = 0;

    while (currentPos < data.size()) {
        const char byte = data.at(currentPos);

        if (byte == '\xff') {
            emit optionCommandReceived(data.at(currentPos+1), data.at(currentPos+2));
            currentPos += 3;
        } else {
            QByteArray plainData = data.mid(currentPos);
            currentPos += plainData.size();
            emit dataReceived(replaceEscapedIACBytes(plainData));
        }
    }
}

QByteArray TelnetParser::replaceEscapedIACBytes(const QByteArray &data)
{
    QByteArray result(data);
    result.replace("\xff\xff", "\xff");
    return result;
}

} // namespace q5250
