#ifndef DELAY_H
#define DELAY_H

#include "dynmath_global.h"

#include <QVector>

class DYNMATHSHARED_EXPORT Delay
{
public:
    Delay();

    double value() const;

    void init(double tau, double h, double v);
    void step(double in);

private:
    int IndexValue, IndexIn;

    QVector<double> DelayData;
};

#endif // DELAY_H
