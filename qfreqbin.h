#ifndef QFREQBIN_H
#define QFREQBIN_H

#include <QObject>
#include <QVector>

class QFreqBin : public QObject
{
    Q_OBJECT
public:
    //explicit QFreqBin(QObject *parent = 0);

    explicit QFreqBin(QObject *parent, int channel, double lower, double upper, int width, double interval);

    double addSample(QVector<double> sample);

    QVector<double> data;
    QVector<double> edge_data;

    int hue, v;
    int channel;

protected:
    int lower_idx;
    int upper_idx;
    int width;

};

#endif // QFREQBIN_H
