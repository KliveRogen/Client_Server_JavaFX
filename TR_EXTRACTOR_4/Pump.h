#ifndef PUMP_H
#define PUMP_H

#include "../DynMath/InertOne.h"

class Pump
{
public:
    Pump();

    void step(double h, double gf, double pumpIn);
    void setCoeffs(double a0, double a1, double a2,
                   double b0, double b1, double b2);

    void init(double f, double tinert);

    double getF() const;
    double getP() const;
    double getI() const;

private:
    double F, P, I;
    double A2, A1, A0, B2, B1, B0;

    InertOne PumpInert;
};

#endif // PUMP_H
