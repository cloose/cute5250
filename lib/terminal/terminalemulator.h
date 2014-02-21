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
#ifndef Q5250_TERMINALEMULATOR_H
#define Q5250_TERMINALEMULATOR_H

#include "q5250_global.h"
#include <QObject>

class QTextCodec;

namespace q5250 {

class DisplayBuffer;
class GeneralDataStream;
class TerminalDisplay;

class Q5250SHARED_EXPORT TerminalEmulator : public QObject
{
    Q_OBJECT

public:
    explicit TerminalEmulator(QObject *parent = 0);

    void setDisplayBuffer(DisplayBuffer *buffer);
    void setTerminalDisplay(TerminalDisplay *display);

signals:
    void updateFinished();

public slots:
    void dataReceived(const QByteArray &data);
    void update();

private:
    void handleWriteToDisplayCommand(GeneralDataStream &stream);

    DisplayBuffer *displayBuffer;
    TerminalDisplay *terminalDisplay;
    QTextCodec *codec;
};

} // namespace q5250

#endif // Q5250_TERMINALEMULATOR_H
