#ifndef PROJECT_H
#define PROJECT_H

#include "../CalcElement/CalcElement.h"
#include "../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class MP_EX_TankPump : public CalcElement
{
public:
    MP_EX_TankPump();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);

private:
	// Put your variables here
    IOutputPort *PortFlowOut, *PortSensors;
    IInputPort *PortFlowRate, *PortFlowIn;

    double TankVolume, SolVolume, M_U, M_Pu, M_HNO3, M_Np;
    double C_U, C_Pu, C_HNO3, C_Np;
    double Temp;
};

#endif // PROJECT_H
