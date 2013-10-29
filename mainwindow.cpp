#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ffft/FFTRealFixLen.h"

#include <iostream>
#include <math.h>
#include <QMessageBox>
#include <QMutex>
#include <QtCore>
#include <QColor>
#include "qfreqbin.h"

#define SAMPLE_RATE 44100

#define NUM_SAMPLES 1024
#define NUM_SAMPLE_SQUARE 10

#define DB_WIDTH 500

ffft::FFTRealFixLen<NUM_SAMPLE_SQUARE> fft;

QVector<double> aud_x_data(NUM_SAMPLES);
QVector<double> mag_x_data(NUM_SAMPLES/2);
QVector<double> db_x_data(DB_WIDTH);
QVector<double> aud_data(NUM_SAMPLES);
QVector<double> spec_data(NUM_SAMPLES);
QVector<double> fft_data(NUM_SAMPLES);
QVector<double> mag_data(NUM_SAMPLES/2);
bool useHanningWindow = true;
double interval = (((double)SAMPLE_RATE)/2) / (NUM_SAMPLES / 2);

QColor colors[] = {Qt::blue, Qt::red, Qt::darkGreen, Qt::darkMagenta, Qt::darkYellow};

QVector<QFreqBin*> bins;
QMutex mutex;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    double i = 0.0;
    for(QVector<double>::iterator it = mag_x_data.begin(); it < mag_x_data.end(); it++) {
        *it = i;
        i += interval;
    }

    for (int i = 0; i < NUM_SAMPLES; i++) {
        aud_x_data[i] = i;
    }

    int v = -DB_WIDTH;
    for (int i = 0; i < DB_WIDTH; i++) {
        db_x_data[i] = v++;
    }

    ui->fftPlot->addGraph();
    ui->fftPlot->graph(0)->setLineStyle(QCPGraph::lsImpulse);
    ui->fftPlot->xAxis->setLabel("Audio");
    ui->fftPlot->yAxis->setLabel("Amplitude");
    ui->fftPlot->yAxis->setRange(-1, 1);
    ui->fftPlot->xAxis->setRange(0, NUM_SAMPLES);
/*
    ui->magPlot->addGraph();
    ui->magPlot->graph(0)->setLineStyle(QCPGraph::lsImpulse);
    ui->magPlot->xAxis->setLabel("Frequency (Hz)");
    ui->magPlot->yAxis->setLabel("Magnitude (dB)");
    ui->magPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->magPlot->yAxis->setScaleLogBase(10.0);
    ui->magPlot->yAxis->setRange(1, 100);
    ui->magPlot->xAxis->setRange(0, i);
*/
    // real bins
    bins.append(new QFreqBin(this,     0,   100, DB_WIDTH, interval));
    bins.append(new QFreqBin(this,    80,   500, DB_WIDTH, interval));
    bins.append(new QFreqBin(this,   400,  2000, DB_WIDTH, interval));
    bins.append(new QFreqBin(this,  1000, 14000, DB_WIDTH, interval));
    bins.append(new QFreqBin(this, 13000, 20000, DB_WIDTH, interval));

    // total
    //bins.append(new QFreqBin(this, 0, SAMPLE_RATE/2, DB_WIDTH, interval));

    for (int i = 0; i < bins.count(); i++) {
        ui->dbPlot->addGraph();
        ui->dbPlot->graph(i)->setPen(QPen(colors[i]));
    }

    ui->dbPlot->xAxis->setLabel("Time (Samples)");
    ui->dbPlot->yAxis->setLabel("Magnitude (dB)");
    ui->dbPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->dbPlot->yAxis->setScaleLogBase(10.0);
    ui->dbPlot->yAxis->setRange(1, 100);
    ui->dbPlot->xAxis->setRange(-DB_WIDTH, 0);

    drawTimer = new QTimer(this);
    drawTimer->setInterval(30);
    connect(drawTimer, SIGNAL(timeout()), this, SLOT(drawChart()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::drawChart() {
    mutex.lock();

    ui->fftPlot->graph(0)->setData(aud_x_data, spec_data);
    //ui->magPlot->graph(0)->setData(mag_x_data, mag_data);

    for (int i = 0; i < bins.count(); i++) {
        bins[i]->addSample(mag_data);
        ui->dbPlot->graph(i)->setData(db_x_data, bins[i]->data);
    }

    mutex.unlock();

    ui->fftPlot->replot();
    //ui->magPlot->replot();
    ui->dbPlot->replot();
}

int paCallBack(const void *input, void *output,
               unsigned long frameCount,
               const PaStreamCallbackTimeInfo* timeInfo,
               PaStreamCallbackFlags statusFlags, void *userData) {

    const float *inFloats = (const float *)input;

    mutex.lock();

    for(int i = 0; i < NUM_SAMPLES; i++) {
        double data = (double)inFloats[i];
        double multiplier = 0.5 * (1 - cos(2*M_PI*i/(NUM_SAMPLES-1)));
        aud_data[i] = useHanningWindow ? multiplier * data : data;
        spec_data[i] = data;
    }

    fft.do_fft(fft_data.data(), aud_data.data());

    int imstart = NUM_SAMPLES/2;

    for(int i = 0; i < NUM_SAMPLES/2; i++) {
        double re,im,mag;
        re = fft_data[i];
        im = fft_data[imstart+i];
        if (i == 0) im = 0.0;

        if (re == 0.0) {
            mag_data[i] = 0.0;
            continue;
        }

        mag = sqrt(pow(re, 2) + pow(im, 2));
        mag_data[i] = mag;//20*log10(mag);
    }

    mutex.unlock();

    return paContinue;
}

void MainWindow::on_actionOpen_Stream_triggered()
{
    QString title("Port Audio Error");
    QString message("Port Audio (%2) Failed! %1 %3");
    PaError err = Pa_OpenDefaultStream(&stream, 1, 0, paFloat32, SAMPLE_RATE, NUM_SAMPLES, paCallBack, this);
    if (err != paNoError) {
        QMessageBox::critical(this, title, message.arg("Opening Stream").arg(Pa_GetVersionText()).arg(Pa_GetErrorText(err)), QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        Pa_CloseStream(stream);
        QMessageBox::critical(this, title, message.arg("Starting Stream").arg(Pa_GetVersionText()).arg(Pa_GetErrorText(err)), QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }

    drawTimer->start();
    ui->actionOpen_Stream->setEnabled(false);
}

void MainWindow::on_actionPlotEnabled_toggled(bool arg1)
{
    if (arg1 && !drawTimer->isActive())
        drawTimer->start();
    else
        drawTimer->stop();
}

void MainWindow::on_actionHanning_Window_toggled(bool arg1)
{
    useHanningWindow = arg1;
}
