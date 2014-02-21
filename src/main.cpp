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
#include <QDateTime>
#include <QFile>
#include <QKeyEvent>
#include <QMap>
#include <QPainter>
#include <QWidget>

#include <generaldatastream.h>
#include <telnet/tcpsockettelnetconnection.h>
#include <telnet/telnetclient.h>
#include <terminal/terminaldisplaybuffer.h>
#include <terminal/terminaldisplay.h>
#include <terminal/terminalemulator.h>
using namespace q5250;

static QMap<unsigned char, QPair<int, int>> InitColorMap()
{
    QMap<unsigned char, QPair<int, int>> colorMap;

    colorMap.insert(0x20, QPair<int, int>(Qt::green, Qt::black));
    colorMap.insert(0x21, QPair<int, int>(Qt::black, Qt::green));
    colorMap.insert(0x22, QPair<int, int>(Qt::white, Qt::black));
    colorMap.insert(0x23, QPair<int, int>(Qt::black, Qt::white));
    colorMap.insert(0x24, QPair<int, int>(Qt::green, Qt::black));
    colorMap.insert(0x25, QPair<int, int>(Qt::black, Qt::green));
    colorMap.insert(0x26, QPair<int, int>(Qt::white, Qt::black));
    colorMap.insert(0x28, QPair<int, int>(Qt::red, Qt::black));
    colorMap.insert(0x29, QPair<int, int>(Qt::black, Qt::red));
    colorMap.insert(0x2a, QPair<int, int>(Qt::red, Qt::black));
    colorMap.insert(0x2b, QPair<int, int>(Qt::black, Qt::red));
    colorMap.insert(0x2c, QPair<int, int>(Qt::red, Qt::black));
    colorMap.insert(0x2d, QPair<int, int>(Qt::black, Qt::red));
    colorMap.insert(0x2e, QPair<int, int>(Qt::red, Qt::black));
    colorMap.insert(0x30, QPair<int, int>(Qt::cyan, Qt::black));
    colorMap.insert(0x31, QPair<int, int>(Qt::black, Qt::cyan));
    colorMap.insert(0x3a, QPair<int, int>(Qt::blue, Qt::black));
    colorMap.insert(0x3b, QPair<int, int>(Qt::black, Qt::blue));

    return colorMap;
}

static const QMap<unsigned char, QPair<int, int>> ColorMap = InitColorMap();

class TerminalDisplayWidget : public QWidget, public TerminalDisplay
{
    Q_OBJECT

public:
    TerminalDisplayWidget();

    void displayText(unsigned char column, unsigned char row, const QString &text);
    void displayAttribute(unsigned char attribute);

signals:
    void sizeChanged();
    void keyPressed(int key, const QString &text);

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    bool showUnderline(unsigned char attribute);
    bool isNonDisplay(unsigned char attribute);

    QPixmap *screen;
    QPainter *painter;
    unsigned char lastAttribute;
};

TerminalDisplayWidget::TerminalDisplayWidget() :
    screen(new QPixmap(size())),
    painter(new QPainter(screen)),
    lastAttribute(0x20)
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);

    QFont font("Monospace", 12);
    font.setStyleHint(QFont::TypeWriter);
    painter->setFont(font);

    painter->setPen(Qt::green);
}

void TerminalDisplayWidget::displayText(unsigned char column, unsigned char row, const QString &text)
{
//    qDebug() << Q_FUNC_INFO << column << row << text;

    if (isNonDisplay(lastAttribute)) return;

    QFontMetrics fm = painter->fontMetrics();
    unsigned int x = column * fm.width('X');
    unsigned int y = row * fm.height();

    // draw background
    painter->setBackground(painter->brush());
    painter->setBackgroundMode(Qt::OpaqueMode);

    painter->drawText(x, y, text);
}

void TerminalDisplayWidget::displayAttribute(unsigned char attribute)
{
//    qDebug() << Q_FUNC_INFO << QString::number(attribute, 16);

    // en-/disable underline
    QFont font = painter->font();
    font.setUnderline(showUnderline(attribute));
    painter->setFont(font);

    if (ColorMap.contains(attribute)) {
        painter->setBrush((Qt::GlobalColor)ColorMap.value(attribute).second);
        painter->setPen((Qt::GlobalColor)ColorMap.value(attribute).first);
    }

    lastAttribute = attribute;
}

void TerminalDisplayWidget::paintEvent(QPaintEvent *event)
{
    qDebug() << Q_FUNC_INFO;
    QPainter p(this);
    p.drawPixmap(0, 0, *screen);
}

void TerminalDisplayWidget::resizeEvent(QResizeEvent *event)
{
    qDebug() << Q_FUNC_INFO << size();
    delete painter;
    delete screen;

    screen = new QPixmap(size());
    painter = new QPainter(screen);

    QFont font("Monospace", 12);
    font.setStyleHint(QFont::TypeWriter);
    painter->setFont(font);

    painter->setPen(Qt::green);

    emit sizeChanged();
}

void TerminalDisplayWidget::keyPressEvent(QKeyEvent *event)
{
    emit keyPressed(event->key(), event->text());
}

bool TerminalDisplayWidget::showUnderline(unsigned char attribute)
{
    static const unsigned short UNDERLINE_MASK = 0x04;
    return attribute & UNDERLINE_MASK;
}

bool TerminalDisplayWidget::isNonDisplay(unsigned char attribute)
{
    static const unsigned short NON_DISPLAY_MASK = 0x07;
    return (attribute & NON_DISPLAY_MASK) == NON_DISPLAY_MASK;
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
    connect(display, &TerminalDisplayWidget::sizeChanged,
            terminal, &TerminalEmulator::update);
    connect(display, &TerminalDisplayWidget::keyPressed,
            terminal, &TerminalEmulator::keyPressed);
    connect(terminal, &TerminalEmulator::updateFinished,
            display, static_cast<void (QWidget::*)()>(&QWidget::update));

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

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
   Q_UNUSED(context);

   QString dt = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
   QString txt = QString("[%1] ").arg(dt);

   switch (type)
   {
      case QtDebugMsg:
         txt += QString("{Debug} \t\t %1").arg(msg);
         break;
      case QtWarningMsg:
         txt += QString("{Warning} \t %1").arg(msg);
         break;
      case QtCriticalMsg:
         txt += QString("{Critical} \t %1").arg(msg);
         break;
      case QtFatalMsg:
         txt += QString("{Fatal} \t\t %1").arg(msg);
         abort();
         break;
   }

   QFile outFile("LogFile.log");
   outFile.open(QIODevice::WriteOnly | QIODevice::Append);

   QTextStream textStream(&outFile);
   textStream << txt << endl;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qInstallMessageHandler(customMessageHandler);

    Main main;

    return app.exec();
}

#include "main.moc"
