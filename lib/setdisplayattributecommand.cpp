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
#include "setdisplayattributecommand.h"

#include <QDebug>
#include <QPainter>

#include "bufferaddress.h"
#include "screenattributes.h"

namespace q5250 {

SetDisplayAttributeCommand::SetDisplayAttributeCommand(unsigned char attribute) :
    displayAttribute(attribute)
{
}

void SetDisplayAttributeCommand::execute(QPainter *p)
{
    qDebug() << "PAINT: SET DISPLAY ATTRIBUTE" << QString::number(displayAttribute, 16);

    // en-/disable underline
    qDebug () << "PAINT: SET UNDERLINE TO" << ScreenAttribute::ShowUnderline(displayAttribute);
    QFont font = p->font();
    font.setUnderline(ScreenAttribute::ShowUnderline(displayAttribute));
    p->setFont(font);

    switch( displayAttribute )
    {
        case ScreenAttribute::GREEN:
            p->setBrush(Qt::black);
            p->setPen(Qt::green);
            break;
        case ScreenAttribute::GREEN_RI:
            p->setBrush(Qt::green);
            p->setPen(Qt::black);
            break;
        case ScreenAttribute::WHITE:
            p->setBrush(Qt::black);
            p->setPen(Qt::white);
            break;
        case ScreenAttribute::WHITE_RI:
            p->setBrush(Qt::white);
            p->setPen(Qt::black);
            break;
        case 0x27:  // non display
            p->setBrush(Qt::black);
            p->setPen(Qt::black);
            break;
        case ScreenAttribute::RED:
            p->setBrush(Qt::black);
            p->setPen(Qt::red);
            break;
        case ScreenAttribute::RED_RI:
            p->setBrush(Qt::red);
            p->setPen(Qt::black);
            break;
        case ScreenAttribute::BLUE:
            p->setBrush(Qt::black);
            p->setPen(Qt::blue);
            break;
        case ScreenAttribute::BLUE_RI:
            p->setBrush(Qt::blue);
            p->setPen(Qt::black);
            break;
        default:
            p->setBrush(Qt::black);
            p->setPen(Qt::green);
            break;
    }

    BufferAddress bufferAddress;
    bufferAddress += 1;
}


} // namespace q5250
