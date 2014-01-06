#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QTextCodec>
//#include <QNetworkProxyFactory>

#include <telnetclient.h>
#include <terminalemulation.h>
using q5250::TelnetClient;
using q5250::TerminalEmulation;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    client(new TelnetClient(this)),
    emulation(0)
{
    ui->setupUi(this);

    qDebug() << QTextCodec::availableCodecs();
    emulation = new TerminalEmulation(ui->terminalWidget);

    connect(client, &TelnetClient::dataReceived,
            emulation, &TerminalEmulation::dataReceived);

    client->setTerminalType("IBM-3477-FC");
//    client->connectToHost("pub1.rzkh.de");
    client->connectToHost("asknidev");
}

MainWindow::~MainWindow()
{
    delete ui;
}

