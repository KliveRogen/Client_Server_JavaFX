#ifndef PROJECT_H
#define PROJECT_H

#include "../CalcElement/CalcElement.h"
#include "../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class EX_RECEIVER : public CalcElement
{
public:
    EX_RECEIVER();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);

private:
	// Put your variables here
    IInputPort *PORT_IN_MAG, *PORT_IN_K1, *PORT_IN_K2;
    IOutputPort *PORT_OUT_SENSORS, *PORT_OUT_PRESSURE;

    double REC_PRESSURE, K1_STATE, K2_STATE;
};

#endif // PROJECT_H
