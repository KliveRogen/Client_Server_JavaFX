#include "InertOneDelay.h"

InertOneDelay::InertOneDelay()
{

}

void InertOneDelay::init(double tau, double h, double v, double k, double t)
{
    D.init(tau, h, v);
    I.init(v, k, t);
}

void InertOneDelay::step(double h, double in)
{
    D.step(in);
    I.step(D.value(), h);
}

double InertOneDelay::value() const
{
    return I.value();
}

