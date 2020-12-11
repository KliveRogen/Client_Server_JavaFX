#ifndef PROJECT_H
#define PROJECT_H

#include "../CalcElement/CalcElement.h"
#include "../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class MP_CR_PeristalticPump : public CalcElement
{
public:
    MP_CR_PeristalticPump();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);

private:
    void setDataNames();

private:
	// Put your variables here
    IOutputPort *PortOut;
    IInputPort *PortInFlow;
    IInputPort *PortInLiq;
};

#endif // PROJECT_H
