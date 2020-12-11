#ifndef PROJECT_H
#define PROJECT_H

#include "../CalcElement/CalcElement.h"
#include "../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class InertOne;

class EX_PUMP : public CalcElement
{
public:
    EX_PUMP();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);

private:
	// Put your variables here
    IInputPort *OnOffPort, *SetFlowPort;
    IOutputPort *FlowPort, *DataPort;

    double FREQ;
    Signal *SP_FREQ, *SP_I, *SP_P;

    InertOne *I_F, *I_P, *I_I;
};

#endif // PROJECT_H
