#include "InertOne.h"

InertOne::InertOne()
{
    Value = 0;
    K = 1;
    T = 1;
}

double InertOne::value() const
{
    return Value;
}

void InertOne::init(double v, double k, double t)
{
    Value = v;
    K = k;
    T = t;
}

void InertOne::step(double in, double h)
{
    Value += h*(K*in - Value) / T;
}

