#ifndef RECEIVER_H
#define RECEIVER_H

#include "Valve.h"

class Receiver
{
public:
    Receiver();

    Valve V_06, V_07, V_62;

    void step(double h, double v06o, double v06c, double v07o, double v07c,
              double v62o, double v62c, double pMag);

    double getP() const;
    void setP(double value);

    void setV_06state(double s);
    void setV_07state(double s);
    void setV_62state(double s);

private:
    double P;
};

#endif // RECEIVER_H
