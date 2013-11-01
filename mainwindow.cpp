#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ffft/FFTRealFixLen.h"
#include "hidapi/hidapi.h"

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

QVector<double> aud_x_data(NUM_SAMPLES, 0);
QVector<double> mag_x_data(NUM_SAMPLES/2, 0);
QVector<double> db_x_data(DB_WIDTH, 0);
QVector<double> aud_data(NUM_SAMPLES, 0);
QVector<double> spec_data(NUM_SAMPLES, 0);
QVector<double> fft_data(NUM_SAMPLES, 0);
QVector<double> mag_data(NUM_SAMPLES/2, 0);
QVector<double> hist_data(DB_WIDTH, 0);
QVector<double> hist_v_data(DB_WIDTH, 0);

bool useHanningWindow = true;
double interval = (((double)SAMPLE_RATE)/2) / (NUM_SAMPLES / 2);

QColor colors[] = {Qt::blue, Qt::red, Qt::darkGreen, Qt::darkMagenta, Qt::darkYellow, Qt::blue};

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

    ui->levelPlot->xAxis->setLabel("Time (Samples)");
    ui->levelPlot->xAxis->setRange(-DB_WIDTH, 0);
    ui->levelPlot->yAxis->setLabel("Magnitude (dB)");
    ui->levelPlot->yAxis->setRange(1, 100);
    ui->levelPlot->yAxis2->setLabel("Brightness (V)");
    ui->levelPlot->yAxis2->setRange(0, 255);
    ui->levelPlot->yAxis2->setVisible(true);
    ui->levelPlot->addGraph();
    ui->levelPlot->addGraph(ui->levelPlot->xAxis, ui->levelPlot->yAxis2);
    ui->levelPlot->graph(1)->setPen(QPen(colors[1]));

    // real bins
    bins.append(new QFreqBin(this, 0,     0,   100, DB_WIDTH, interval));
    bins.append(new QFreqBin(this, 2,    80,   500, DB_WIDTH, interval));
    bins.append(new QFreqBin(this, 1,   400,  2000, DB_WIDTH, interval));
    //bins.append(new QFreqBin(this, 1,  1000, 14000, DB_WIDTH, interval));
    bins.append(new QFreqBin(this, 3, 13000, 20000, DB_WIDTH, interval));

    // total
    //bins.append(new QFreqBin(this, 0, SAMPLE_RATE/2, DB_WIDTH, interval));

    for (int i = 0; i < bins.count(); i++) {
        ui->dbPlot->addGraph();
        ui->dbPlot->graph(i*2)->setPen(QPen(colors[i]));

        ui->dbPlot->addGraph();
        ui->dbPlot->graph(i*2+1)->setLineStyle(QCPGraph::lsNone);
        ui->dbPlot->graph(i*2+1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, colors[i+1], colors[i+1], 3.0));
    }

    ui->dbPlot->xAxis->setLabel("Time (Samples)");
    ui->dbPlot->yAxis->setLabel("Magnitude (dB)");
    //ui->dbPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    //ui->dbPlot->yAxis->setScaleLogBase(10.0);
    ui->dbPlot->yAxis->setRange(1, 100);
    ui->dbPlot->xAxis->setRange(-DB_WIDTH, 0);

    drawTimer = new QTimer(this);
    devTimer = new QTimer(this);
    drawTimer->setInterval(30);
    devTimer->setInterval(10);
    connect(drawTimer, SIGNAL(timeout()), this, SLOT(drawChart()));
    connect(devTimer, SIGNAL(timeout()), this, SLOT(writeColor()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::drawChart() {
    mutex.lock();

    ui->fftPlot->graph(0)->setData(aud_x_data, spec_data);

    for (int i = 0; i < bins.size(); i++) {
        ui->dbPlot->graph(i*2)->setData(db_x_data, bins[i]->data);
        ui->dbPlot->graph(i*2+1)->setData(db_x_data, bins[i]->edge_data);
    }

    ui->levelPlot->graph(0)->setData(db_x_data, hist_data);
    ui->levelPlot->graph(1)->setData(db_x_data, hist_v_data);


    mutex.unlock();

    ui->fftPlot->replot();
    ui->dbPlot->replot();
    ui->levelPlot->replot();
}

typedef struct {
    double mean;
    double stddev;
} stddev;

stddev std_dev(QVector<double> a, int n = -1) {
    stddev ret;
    ret.mean = 0;
    ret.stddev = 0;

    if (n < 0)
        n = a.size();

    int end = a.size() - 1;
    int start = end - n;

    if(n == 0 || start < 0)
        return ret;

    double sum = 0;
    double sq_sum = 0;
    for(int i = start; i < end; i++) {
       sum += a[i];
       sq_sum += a[i] * a[i];
    }
    ret.mean = sum / n;
    double variance = sq_sum / n - ret.mean * ret.mean;
    ret.stddev = sqrt(variance);
    return ret;
}

void MainWindow::writeColor() {
    QColor hsv, rgb;
    int r, g, b, v, pos = 0;
    unsigned char data[64] = {0};

    mutex.lock();
    v = hist_v_data.last();
    mutex.unlock();



    data[pos++] = 'a'; // magic
    data[pos++] = 'l'; // magic
    data[pos++] = bins.size() * 5; // packet length

    for (int i = 0; i < bins.size(); i++) {
        data[pos++] = 5;
        data[pos++] = (unsigned char)bins[i]->channel;
        hsv.setHsv(bins[i]->hue, 255, v);
        rgb = hsv.toRgb();
        rgb.getRgb(&r, &g, &b);
        data[pos++] = (unsigned char)r;
        data[pos++] = (unsigned char)g;
        data[pos++] = (unsigned char)b;
    }

    hid_write(device, data, sizeof(data));
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
    double sum = 0.0;

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
        mag_data[i] = mag;
        sum += mag;
    }

    sum = 20 * log10(sum);

    hist_data.pop_front();
    hist_data.push_back(sum);

    stddev sdev = std_dev(hist_data, 50);
    double current = hist_data.last();

    double max = sdev.mean + sdev.stddev * 1.5;
    double min = sdev.mean / (3 + sdev.stddev);

    int v = ((current - min) * 255 / max) - 50;
    if (v > 255) v = 255;
    if (v < 0) v = 0;

    hist_v_data.pop_front();
    hist_v_data.push_back(v);

    for (int i = 0; i < bins.size(); i++) {
        bins[i]->addSample(mag_data);
    }
    mutex.unlock();

    return paContinue;
}

void MainWindow::on_actionOpen_Stream_triggered()
{
    QString title("Port Audio Error");
    QString message("Port Audio (%2) Failed! %1 %3");
    PaError err = Pa_OpenDefaultStream(&stream, 1, 0, paFloat32, SAMPLE_RATE, NUM_SAMPLES, paCallBack, device);
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

void MainWindow::on_actionOpen_HID_toggled(bool arg1)
{
    if (arg1) {
        device = hid_open(0x16c0, 0x486, NULL);
        if (device) {
            ui->actionOpen_HID->setEnabled(false);
            devTimer->start();
        }
    }
}
