#ifndef QFREQBIN_H
#define QFREQBIN_H

#include <QObject>
#include <QVector>

class QFreqBin : public QObject
{
    Q_OBJECT
public:
    //explicit QFreqBin(QObject *parent = 0);

    explicit QFreqBin(QObject *parent, double lower, double upper, double width, double interval);

    void addSample(QVector<double> sample);

    QVector<double> data;

protected:
    int lower_idx;
    int upper_idx;

};

#endif // QFREQBIN_H
