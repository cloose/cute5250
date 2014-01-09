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
#ifndef Q5250_TERMINALEMULATION_H
#define Q5250_TERMINALEMULATION_H

#include "q5250_global.h"

#include <memory>

#include <QObject>

namespace q5250 {

class Field;

class Q5250SHARED_EXPORT TerminalEmulation : public QObject
{
    Q_OBJECT

public:
    explicit TerminalEmulation(QObject *parent = 0);
    ~TerminalEmulation();

Q_SIGNALS:
    void clearUnit();
    void displayField(const q5250::Field &field);
    void displayText(const QByteArray &ebcdicText);
    void repeatCharacter(uint col, uint row, uchar character);
    void setBufferAddress(unsigned char column, unsigned char row);
    void setDisplayAttribute(const unsigned char attribute);

public Q_SLOTS:
    void dataReceived(const QByteArray &data);

private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace q5250

#endif // Q5250_TERMINALEMULATION_H
