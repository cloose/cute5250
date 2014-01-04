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
#include "terminalemulation.h"

#include <QDebug>

#include "generaldatastream.h"
#include "terminalwidget.h"

namespace q5250 {

class TerminalEmulation::Private : public QObject
{
    Q_OBJECT

public:
    explicit Private();

    TerminalWidget *terminal;
};

TerminalEmulation::Private::Private()
{
}


TerminalEmulation::TerminalEmulation(TerminalWidget *terminal, QObject *parent) :
    QObject(parent),
    d(new Private())
{
    d->terminal = terminal;
}

TerminalEmulation::~TerminalEmulation()
{
}

void TerminalEmulation::dataReceived(const QByteArray &data)
{
    GeneralDataStream dataStream(data);

    while (!dataStream.atEnd()) {
        unsigned char byte = dataStream.readByte();
//        qDebug() << Q_FUNC_INFO << byte;

        if (byte == 0x04 /*ESC*/) {
            byte = dataStream.readByte();
            qDebug() << "SERVER: [GDS] ESC" << QString::number(byte, 16);

            switch (byte) {
            case 0x11:
                qDebug() << "SERVER: [GDS] WRITE TO DISPLAY";
                break;

            case 0x40:
                qDebug() << "SERVER: [GDS] CLEAR UNIT";
                d->terminal->clearUnit();
                break;

            default:
                qDebug() << "** UNKNOWN **";
                break;
            }
        }
    }
}

} // namespace q5250

#include "terminalemulation.moc"