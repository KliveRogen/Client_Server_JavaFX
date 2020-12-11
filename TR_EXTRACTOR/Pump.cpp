#include "Pump.h"

Pump::Pump()
{

}

void Pump::step(double h, double gf, double pumpIn)
{
    if(pumpIn == 0) gf = 0;

    PumpInert.step(gf, h);
}

void Pump::setCoeffs(double a0, double a1, double a2,
                     double b0, double b1, double b2)
{
    A0 = a0;
    A1 = a1;
    A2 = a2;

    B0 = b0;
    B1 = b1;
    B2 = b2;
}

void Pump::init(double f, double tinert)
{
    PumpInert.init(f, 1, tinert);
}

double Pump::getF() const
{
    return PumpInert.value();
}

double Pump::getP() const
{
    double f = getF();
    return A2*f*f + A1*f + A0;
}

double Pump::getI() const
{
    double f = getF();
    return B2*f*f + B1*f + B0;
}

