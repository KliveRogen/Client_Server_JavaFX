#include "Tank.h"

Tank::Tank()
{

}

void Tank::step(double h, double Qin, double Cin, double Tin, double vo, double vc, double gf, double pumpIn)
{
    TankValve.step(h, vo, vc);
    TankPump.step(h, gf, pumpIn);

    double Qout = k1*TankPump.getP()*TankValve.getState() / 100.00;
    double dV = Qin - Qout;

    double muin = Qin*Cin*h;
    double muout = Qout*Conc*h;

    double totalM = Volume*Conc + muin - muout;
    if(totalM < 0.00) totalM = 0.00;

    Volume += h*dV;
    if(Volume <= 0) {
        Volume = 0;
        Conc = 0.00;
        Temp = 0.00;
    }
    else {
        Conc = totalM / Volume;
        Temp = 0.00;
    }
}

void Tank::setValveState(double s)
{
    TankValve.setState(s);
}

void Tank::setVolume(double v)
{
    Volume = v;
}

void Tank::setConc(double c)
{
    Conc = c;
}

void Tank::setTemp(double t)
{
    Temp = t;
}

