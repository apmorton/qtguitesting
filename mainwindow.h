#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "portaudio.h"
#include "hidapi/hidapi.h"

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
    void writeColor();

private slots:
    void on_actionOpen_Stream_triggered();

    void on_actionPlotEnabled_toggled(bool arg1);

    void on_actionHanning_Window_toggled(bool arg1);

    void on_actionOpen_HID_toggled(bool arg1);

private:
    Ui::MainWindow *ui;
    QTimer *drawTimer;
    QTimer *devTimer;

    PaStream *stream;
    hid_device *device;

    int v;
};

#endif // MAINWINDOW_H
