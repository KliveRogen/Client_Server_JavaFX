#ifndef INERTONE_H
#define INERTONE_H

#include "dynmath_global.h"

class DYNMATHSHARED_EXPORT InertOne
{
public:
    InertOne();

    double value() const;

    void init(double v, double k, double t);
    void step(double in, double h);

private:
    double Value, T, K;
};

#endif // INERTONE_H
