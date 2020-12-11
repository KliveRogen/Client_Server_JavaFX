#ifndef PROJECT_H
#define PROJECT_H

#include "../CalcElement/CalcElement.h"
#include "../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class MP_CR_FeedController : public CalcElement
{
public:
    MP_CR_FeedController();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);

private:
    void setDataNames();

private:
	// Put your variables here
    double setup;
    IOutputPort *PortOut;
    IInputPort *PortIn;

    double per_FIM1_p, per_FIM2_p, per_FIM1_o, per_FIM2_o, Q_pump1, Q_pump2;
};

#endif // PROJECT_H
