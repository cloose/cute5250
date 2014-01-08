#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

#include <telnetclient.h>
#include <terminalemulation.h>
#include <terminalwidget.h>
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

    emulation = new TerminalEmulation();

    connect(client, &TelnetClient::dataReceived,
            emulation, &TerminalEmulation::dataReceived);

    connect(emulation, &TerminalEmulation::clearUnit,
            ui->terminalWidget, &TerminalWidget::clearUnit);
    connect(emulation, &TerminalEmulation::setBufferAddress,
            ui->terminalWidget, &TerminalWidget::positionCursor);

    client->setTerminalType("IBM-3477-FC");
    client->connectToHost("pub1.rzkh.de");
}

MainWindow::~MainWindow()
{
    delete ui;
}

