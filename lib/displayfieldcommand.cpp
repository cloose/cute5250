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
#include "displayfieldcommand.h"

#include <QDebug>
#include <QPainter>

#include "bufferaddress.h"
#include "screenattributes.h"

namespace q5250 {

DisplayFieldCommand::DisplayFieldCommand(const Field& f) :
    field(f)
{
}

void DisplayFieldCommand::execute(QPainter* p)
{
    BufferAddress bufferAddress;

    if (ScreenAttribute::IsNonDisplay(field.attribute())) {
        qDebug() << "PAINT: NON DISPLAY FIELD" << QString::number(field.attribute(), 16);
        return;
    }

    qDebug() << "PAINT: DISPLAY FIELD AT" << bufferAddress.column() << bufferAddress.row() << "WITH ATTRIBUTE" << QString::number(field.attribute(), 16);

//    if (ScreenAttribute::ShowUnderline(field.attribute())) {
//        // convert buffer address to pixel
//        QFontMetrics fm = p->fontMetrics();
//        unsigned int x = bufferAddress.column() * fm.width('X'); //* fm.maxWidth();
//        unsigned int y = bufferAddress.row() * fm.height();
//        unsigned int width = field.length() * fm.width('X');

//        p->drawLine(x, y+2, x+width, y+2);
//    }

//    // screen attribute
//    bufferAddress += 1;

    // convert buffer address to pixel
    QFontMetrics fm = p->fontMetrics();
    unsigned int x = bufferAddress.column() * fm.width('X'); //* fm.maxWidth();
    unsigned int y = bufferAddress.row() * fm.height();

//    p->save();

//    if (ScreenAttribute::ShowUnderline(field.attribute())) {
//        QFont font = p->font();
//        font.setUnderline(true);
//        p->setFont(font);
//    }

    p->drawText(x, y, QString(field.length(), QLatin1Char(' ')));
//    p->restore();
}

} // namespace q5250
