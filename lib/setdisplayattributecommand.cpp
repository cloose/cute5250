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
    static const QMap<ScreenAttribute::ScreenAttributes, QPair<int, int>> colorMap {
        { ScreenAttribute::GREEN, QPair<int, int>(Qt::green, Qt::black) },
        { ScreenAttribute::GREEN_RI, QPair<int, int>(Qt::black, Qt::green) },
        { ScreenAttribute::WHITE, QPair<int, int>(Qt::white, Qt::black) },
        { ScreenAttribute::WHITE_RI, QPair<int, int>(Qt::black, Qt::white) },
        { ScreenAttribute::RED, QPair<int, int>(Qt::red, Qt::black) },
        { ScreenAttribute::RED_RI, QPair<int, int>(Qt::black, Qt::red) },
        { ScreenAttribute::BLUE, QPair<int, int>(Qt::blue, Qt::black) },
        { ScreenAttribute::BLUE_RI, QPair<int, int>(Qt::black, Qt::blue) }
    };

    qDebug() << "PAINT: SET DISPLAY ATTRIBUTE" << QString::number(displayAttribute, 16);

    // en-/disable underline
    qDebug () << "PAINT: SET UNDERLINE TO" << ScreenAttribute::ShowUnderline(displayAttribute);
    QFont font = p->font();
    font.setUnderline(ScreenAttribute::ShowUnderline(displayAttribute));
    p->setFont(font);

    ScreenAttribute::ScreenAttributes attribute = (ScreenAttribute::ScreenAttributes)displayAttribute;
    if (colorMap.contains(attribute)) {
        p->setBrush((Qt::GlobalColor)colorMap.value(attribute).second);
        p->setPen((Qt::GlobalColor)colorMap.value(attribute).first);
    }

    BufferAddress bufferAddress;
    bufferAddress += 1;
}


} // namespace q5250
