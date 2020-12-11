#ifndef PROJECT_H
#define PROJECT_H

#include "../CalcElement/CalcElement.h"
#include "../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class Amplifier : public CalcElement
{
public:
    Amplifier();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);

private:
	// Put your variables here
    IInputPort *InPort;
    IOutputPort *OutPort;

    double Gain;
};

#endif // PROJECT_H
