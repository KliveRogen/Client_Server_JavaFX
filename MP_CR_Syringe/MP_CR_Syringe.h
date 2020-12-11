#ifndef PROJECT_H
#define PROJECT_H

#include "../CalcElement/CalcElement.h"
#include "../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class MP_CR_Syringe : public CalcElement
{
public:
    MP_CR_Syringe();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);

private:
    void setDataNames();
    std::vector<double> dydt();

private:
	// Put your variables here
    IOutputPort *PortOutData, *PortOutLiq;
    IInputPort *PortInControl, *PortInLiq;

    // Params
    double CylinderHeight, CylinderRadius, PistonHeight, PistonRadius, FeedHoleRadius, SensorHoleRadius;

    // Vars
    double LiqVol, PistonLevel, Flow, Temperature, mU, mPu, mNp, mHNO3, dhPiston, PistonLiftTime;
    double SqPiston, VPiston, VCylinder, perFIM;
    double inFlow, inTemp, inCU, inCPu, inCNp, inCHNO3;

    Signal *PistonLevelSig, *LiquidLevelSig;
};

#endif // PROJECT_H
