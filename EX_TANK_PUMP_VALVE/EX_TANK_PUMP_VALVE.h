#ifndef PROJECT_H
#define PROJECT_H

#include "../CalcElement/CalcElement.h"
#include "../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class InertOne;
class Integrator;

class EX_TANK_PUMP_VALVE : public CalcElement
{
public:
    EX_TANK_PUMP_VALVE();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);

private:
	// Put your variables here
    Signal *SIG_VOLUME, *SIG_SU, *SIG_NU, *SIG_VALVE, *SIG_PUMP;
    IInputPort *VALVE_PORT, *PUMP_PORT, *FLOW_IN;
    IOutputPort *FLOW_OUT;

    double M_U, M_HNO3, VOLUME, OUT_Q, VALVE_STATE, TEMPERATURE;

    Integrator *Int_L;
    InertOne *IO_Q, *IO_CU, *IO_T, *IO_F, *IO_P, *IO_I, *IO_CHNO3;
};

#endif // PROJECT_H
