#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QTextCodec>

#include <field.h>
#include <telnetclient.h>
#include <terminalemulation.h>
#include <terminalwidget.h>
using q5250::Field;
using q5250::TelnetClient;
using q5250::TerminalEmulation;
using q5250::TerminalWidget;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    client(new TelnetClient(this)),
    emulation(0)
{
    ui->setupUi(this);

    qDebug() << QTextCodec::availableCodecs();
    emulation = new TerminalEmulation();

    connect(client, &TelnetClient::dataReceived,
            emulation, &TerminalEmulation::dataReceived);

    connect(emulation, &TerminalEmulation::clearUnit,
            ui->terminalWidget, &TerminalWidget::clearUnit);
    connect(emulation, &TerminalEmulation::displayField,
            ui->terminalWidget, &TerminalWidget::displayField);
    connect(emulation, &TerminalEmulation::displayText,
            ui->terminalWidget, &TerminalWidget::displayText);
    connect(emulation, &TerminalEmulation::repeatCharacter,
            ui->terminalWidget, &TerminalWidget::repeatCharacter);
    connect(emulation, &TerminalEmulation::setBufferAddress,
            ui->terminalWidget, &TerminalWidget::positionCursor);
    connect(emulation, &TerminalEmulation::setDisplayAttribute,
            ui->terminalWidget, &TerminalWidget::setDisplayAttribute);

    client->setTerminalType("IBM-3477-FC");
//    client->connectToHost("pub1.rzkh.de");
    client->connectToHost("asknidev");
}

MainWindow::~MainWindow()
{
    delete ui;
}

