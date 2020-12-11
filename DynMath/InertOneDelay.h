#ifndef INERTONEDELAY_H
#define INERTONEDELAY_H

#include "dynmath_global.h"

#include "Delay.h"
#include "InertOne.h"

class DYNMATHSHARED_EXPORT InertOneDelay
{
public:
    InertOneDelay();

    void init(double tau, double h, double v, double k, double t);
    void step(double h, double in);

    double value() const;

private:
    Delay D;
    InertOne I;
};

#endif // INERTONEDELAY_H
