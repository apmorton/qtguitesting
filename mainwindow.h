#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "portaudio.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void drawChart();

private slots:
    void on_actionOpen_Stream_triggered();

    void on_actionPlotEnabled_toggled(bool arg1);

    void on_actionHanning_Window_toggled(bool arg1);

private:
    Ui::MainWindow *ui;
    PaStream *stream;
    QTimer *drawTimer;
};

#endif // MAINWINDOW_H
