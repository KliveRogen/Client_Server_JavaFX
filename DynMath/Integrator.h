#ifndef INTEGRATOR_H
#define INTEGRATOR_H

#include "dynmath_global.h"

class DYNMATHSHARED_EXPORT Integrator
{
public:
    Integrator();

    double value() const;

    void step(double in, double h);
    void init(double v, double k, double t);

private:
    double Value, T, K;
};

#endif // INTEGRATOR_H
