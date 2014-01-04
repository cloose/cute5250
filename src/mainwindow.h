#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}
namespace q5250 {
class TelnetClient;
class TerminalEmulation;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    q5250::TelnetClient *client;
    q5250::TerminalEmulation *emulation;
};

#endif // MAINWINDOW_H
