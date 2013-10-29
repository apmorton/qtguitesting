#include "qfreqbin.h"
#include <math.h>

QFreqBin::QFreqBin(QObject *parent, double lower, double upper, double width, double interval) : data(width), QObject(parent) {
    lower_idx = lower / interval;
    upper_idx = upper / interval;
}

void QFreqBin::addSample(QVector<double> sample) {
    double sum = 0.0;

    // sum the channels in our range
    for (int i = lower_idx; i < upper_idx; i++) {
        sum += sample[i];
    }

    // convert to decibels
    sum = 20 * log10(sum);

    // rotate and insert
    data.pop_front();
    data.push_back(sum);
}
