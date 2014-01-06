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
#include "terminalwidget.h"

#include <QDebug>
#include <QPainter>
#include <QQueue>

#include "changepositioncommand.h"
#include "clearunitcommand.h"
#include "paintercommand.h"

namespace q5250 {

class TerminalWidget::Private : public QObject
{
    Q_OBJECT

public:
    explicit Private();

    QQueue<PainterCommand*> paintQueue;
};

TerminalWidget::Private::Private()
{
}


TerminalWidget::TerminalWidget(QWidget *parent) :
    QWidget(parent),
    d(new Private())
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);
}

TerminalWidget::~TerminalWidget()
{
}

void TerminalWidget::clearUnit()
{
    qDebug() << "TERMINAL: CLEAR UNIT";
    d->paintQueue.enqueue(new ClearUnitCommand(rect()));
}

void TerminalWidget::positionCursor(uint column, uint row)
{
    qDebug() << "TERMINAL: POSITION CURSOR" << row << column;
    d->paintQueue.enqueue(new ChangePositionCommand(column, row));
}

void TerminalWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);

    while (!d->paintQueue.isEmpty()) {
        PainterCommand *command = d->paintQueue.dequeue();
        command->execute(&p);
    }
}

} // namespace q5250

#include "terminalwidget.moc"
