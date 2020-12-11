#include "Valve.h"

Valve::Valve()
{
    OpenTime = 1;
    State = 0;
}

void Valve::setOpenTime(double value)
{
    OpenTime = value;
}

double Valve::getState() const
{
    return State;
}

void Valve::step(double h, double cOn, double cOff)
{
    if(cOn != 0)  cOn = 1.00;
    if(cOff != 0) cOff = 1.00;

    State += h*OpenTime*(cOn - cOff);
    if(State >= 100.00) State = 100.00;
    if(State <= 0.00)   State = 0.00;
}

void Valve::setState(double value)
{
    State = value;
}

