#include "qfreqbin.h"
#include <math.h>

int random_in_range(unsigned int min, unsigned int max);

int random_in_range(unsigned int min, unsigned int max)
{
  int base_random = rand(); /* in [0, RAND_MAX] */
  if (RAND_MAX == base_random) return random_in_range(min, max);
  /* now guaranteed to be in [0, RAND_MAX) */
  int range       = max - min,
      remainder   = RAND_MAX % range,
      bucket      = RAND_MAX / range;
  /* There are range buckets, plus one smaller interval
     within remainder of RAND_MAX */
  if (base_random < RAND_MAX - remainder) {
    return min + base_random/bucket;
  } else {
    return random_in_range (min, max);
  }
}

QFreqBin::QFreqBin(QObject *parent, int channel, double lower, double upper, int width, double interval) : data(width, 0.0), edge_data(width, -10.0), QObject(parent) {
    lower_idx = lower / interval;
    upper_idx = upper / interval;
    this->width = width;
    this->channel = channel;
}

double QFreqBin::addSample(QVector<double> sample) {
    double sum = 0.0;
    double edge = -10.0;
    double min = 0.0;
    double max = 0.0;
    double dif = 0.0;
    int i, maxi;

    // sum the channels in our range
    for (int i = lower_idx; i < upper_idx; i++) {
        sum += sample[i];
    }

    // convert to decibels
    sum = 20 * log10(sum);

    // rotate and insert
    data.pop_front();
    data.push_back(sum);
    edge_data.pop_front();
    edge_data.push_back(edge);

    // no edge
    if (sum <= 0) {
        v = 0;
        return 0.0;
    }

    // start at the last sample
    i = width - 2;
    max = sum;

    // find the maximum
    while (data[i] > max) {
        max = data[i--];
    }

    // check for level edge
    if (max == sum) {
        // no edge
        if (max - data[--i] > max * 0.05) return 0.0;

        // level edge
        max = data[i];
    }

    // record the max position
    maxi = i + 1;
    min = max;

    // find the minimum
    while (data[i] < min) {
        min = data[i--];
    }

    // find the difference
    dif = max - min;

    // no edge if less than 20% difference
    if (dif < max * 0.2) return 0.0;

    // we have an edge
    edge_data[maxi] = data[maxi];

    // generate a random hue
    hue = random_in_range(0, 360);

    // generate v
    v = (int)((((dif * 100) / max) * 255) / 100) & 0xFF;


    return dif;
}
