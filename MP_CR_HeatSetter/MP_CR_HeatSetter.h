#ifndef PROJECT_H
#define PROJECT_H

#include "../CalcElement/CalcElement.h"
#include "../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class MP_CR_HeatSetter : public CalcElement
{
public:
    MP_CR_HeatSetter();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);

private:
    void setDataNames();

private:
	// Put your variables here
    IOutputPort *PortOut;
    std::vector<double> Out;
    double HeatUpCryst, HeatDownCryst, HeatUpWash, HeatDownWash;
};

#endif // PROJECT_H
