/*
 * Copyright (c) 2014, Christian Loose
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
#include <QApplication>

#include <QDebug>
#include <QPainter>
#include <QWidget>

#include <generaldatastream.h>
#include <telnet/tcpsockettelnetconnection.h>
#include <telnet/telnetclient.h>
#include <terminal/terminaldisplaybuffer.h>
#include <terminal/terminaldisplay.h>
#include <terminal/terminalemulator.h>
using namespace q5250;

class TerminalDisplayWidget : public QWidget, public TerminalDisplay
{
public:
    TerminalDisplayWidget();

    void displayText(unsigned char column, unsigned char row, const QString &text);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QPixmap *screen;
    QPainter *painter;
};

TerminalDisplayWidget::TerminalDisplayWidget() :
    screen(new QPixmap(size())),
    painter(new QPainter(screen))
{
    painter->setPen(Qt::green);
}

void TerminalDisplayWidget::displayText(unsigned char column, unsigned char row, const QString &text)
{
    qDebug() << Q_FUNC_INFO << text;
    QFontMetrics fm = painter->fontMetrics();
    unsigned int x = column * fm.width('X');
    unsigned int y = row * fm.height();
    painter->drawText(x, y, text);
}

void TerminalDisplayWidget::paintEvent(QPaintEvent *event)
{
    qDebug() << Q_FUNC_INFO;
    QPainter p(this);
    p.drawPixmap(0, 0, *screen);
}

class Main : public QObject
{
    Q_OBJECT

public:
    Main(QObject *parent = 0);

private slots:
    void dataReceived(const QByteArray &data);

private:
    TcpSocketTelnetConnection *connection;
    TelnetClient *client;
    TerminalEmulator *terminal;
    TerminalDisplayWidget *display;
};

Main::Main(QObject *parent) :
    QObject(parent),
    connection(new TcpSocketTelnetConnection(this)),
    client(new TelnetClient(connection)),
    terminal(new TerminalEmulator()),
    display(new TerminalDisplayWidget())
{
    connect(connection, &TcpSocketTelnetConnection::readyRead,
            client, &TelnetClient::readyRead);
    connect(client, &TelnetClient::dataReceived,
            terminal, &TerminalEmulator::dataReceived);

    client->setTerminalType("IBM-3477-FC");
    terminal->setDisplayBuffer(new TerminalDisplayBuffer());
    terminal->setTerminalDisplay(display);
    connection->connectToHost(QStringLiteral("ASKNIDEV"), 23);

    display->show();
    terminal->update();
}

void Main::dataReceived(const QByteArray &data)
{
//    qDebug() << "--- SERVER ---";
//    for (int i = 0; i < data.size(); ++i) {
//        qDebug() << QString::number(uchar(data[i]), 16)
//                 << QString::number(uchar(data[i]))
//                 << data[i];
//    }

//    GeneralDataStream stream(data);
//    qDebug() << "Valid?" << stream.isValid();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Main main;

    return app.exec();
}

#include "main.moc"
