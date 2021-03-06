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
#ifndef Q5250_TELNETCLIENT_H
#define Q5250_TELNETCLIENT_H

#include "q5250_global.h"
#include <QObject>

#include "telnetcommand.h"
#include "telnetoption.h"
#include "telnetparser.h"

namespace q5250 {

class TelnetConnection;

class Q5250SHARED_EXPORT TelnetClient : public QObject
{
    Q_OBJECT

public:
    explicit TelnetClient(TelnetConnection *conn);

    void setTerminalType(const QString &type);

    void readyRead();
    void sendData(const QByteArray &data);

signals:
    void dataReceived(const QByteArray &data);

private slots:
    void optionNegotiationReceived(const q5250::OptionNegotiation &optionNegotiation);
    void subnegotiationReceived(const q5250::Subnegotiation &subnegotiation);

private:
    void sendCommand(TelnetCommand command, TelnetOption option);
    void sendCommand(const QByteArray &command);
    bool isOptionSupported(TelnetOption option);
    TelnetCommand replyFor(TelnetCommand command, bool supported);

    TelnetConnection *connection;
    TelnetParser parser;
    QString terminalType;
};

} // namespace q5250

#endif // Q5250_TELNETCLIENT_H
