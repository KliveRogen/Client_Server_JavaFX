#ifndef VALVE_H
#define VALVE_H

class Valve
{
public:
    Valve();

    void setOpenTime(double value);

    double getState() const;
    void setState(double value);

    void step(double h, double cOn, double cOff);

private:
    double OpenTime, State;
};

#endif // VALVE_H
