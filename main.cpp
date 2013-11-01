#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include "portaudio.h"
#include "hidapi/hidapi.h"

int main(int argc, char *argv[])
{
    QString title("Port Audio Error");
    QString message("%1 Port Audio (%2) Failed! %3");
    QApplication a(argc, argv);
    MainWindow w;

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        return QMessageBox::critical(&w, title, message.arg("Initializing").arg(Pa_GetVersionText()).arg(Pa_GetErrorText(err)), QMessageBox::Ok, QMessageBox::NoButton);
    }

    if (hid_init() != 0) {
        return QMessageBox::critical(&w, QString("HID API Error"), QString("Error Initializing HID API"), QMessageBox::Ok, QMessageBox::NoButton);
    }

    w.show();

    int ret = a.exec();

    err = Pa_Terminate();
    if (err != paNoError) {
        return QMessageBox::critical(&w, title, message.arg("Terminating").arg(Pa_GetVersionText()).arg(Pa_GetErrorText(err)), QMessageBox::Ok, QMessageBox::NoButton);
    }

    if (hid_exit() != 0) {
        return QMessageBox::critical(&w, QString("HID API Error"), QString("Error Terminating HID API"), QMessageBox::Ok, QMessageBox::NoButton);
    }

    return ret;
}
