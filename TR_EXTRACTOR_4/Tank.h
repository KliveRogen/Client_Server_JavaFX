#ifndef TANK_H
#define TANK_H

#include "Valve.h"
#include "Pump.h"

class Tank
{
public:
    Tank();

    void step(double h, double Qin, double Cin, double Tin, double vo, double vc, double gf, double pumpIn);
    void setValveState(double s);
    void setVolume(double v);
    void setConc(double c);
    void setTemp(double t);

private:
    Pump TankPump;
    Valve TankValve;

    double Volume;
    double Conc, Temp;
    double k1;
};

#endif // TANK_H
