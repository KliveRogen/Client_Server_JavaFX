#include "Delay.h"

Delay::Delay()
{

}

double Delay::value() const
{
    return DelayData[IndexValue];
}

void Delay::init(double tau, double h, double v)
{
    int size = tau / h;

    DelayData.resize(size * 2);
    DelayData.fill(v);
    IndexValue = 0;
    IndexIn = size;
}

void Delay::step(double in)
{
    DelayData[IndexIn] = in;
    IndexIn++;
    if(IndexIn >= DelayData.size()) IndexIn = 0;
    IndexValue++;
    if(IndexValue >= DelayData.size()) IndexValue = 0;
}

