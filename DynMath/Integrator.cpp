#include "Integrator.h"

Integrator::Integrator()
{
    Value = 0;
    K = 1;
    T = 1;
}

double Integrator::value() const
{
    return Value;
}

void Integrator::step(double in, double h)
{
    Value += h*in*K/T;
}

void Integrator::init(double v, double k, double t)
{
    Value = v;
    K = k;
    T = t;
}
