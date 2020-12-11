#include "Receiver.h"

Receiver::Receiver()
{

}

void Receiver::step(double h, double v06o, double v06c, double v07o, double v07c, double v62o, double v62c, double pMag)
{
    V_06.step(h, v06o, v06c);
    V_07.step(h, v07o, v07c);
    V_62.step(h, v62o, v62c);

    double dup06, dup07, dup62;
    dup06 = V_06.getState();
    dup07 = V_07.getState();
    dup62 = V_62.getState();
    double dp;

    if(dup06 == 0 && dup07 == 0) dp = 0;
    else if(dup07 == 0) dp = -P / (2000 - 19.8*dup06);
    else dp = (pMag - P) / (2000 - 19.8*dup06);

    P += h*dp;
}

double Receiver::getP() const
{
    return P;
}

void Receiver::setP(double value)
{
    P = value;
}

void Receiver::setV_06state(double s)
{
    V_06.setState(s);
}

void Receiver::setV_07state(double s)
{
    V_07.setState(s);
}

void Receiver::setV_62state(double s)
{
    V_62.setState(s);
}

