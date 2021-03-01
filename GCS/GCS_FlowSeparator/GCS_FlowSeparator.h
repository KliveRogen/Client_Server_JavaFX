#ifndef PROJECT_H
#define PROJECT_H

#include "../../CalcElement/CalcElement.h"
#include "../../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class GCS_FlowSeparator : public CalcElement
{
public:
    GCS_FlowSeparator();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);
	void setDataNames();
	
private:
	// Put your variables here
    IInputPort *inGas;
    IOutputPort *outGas1, *outGas2;
    double gas1VolumeFlowRateCurrent, gas1OutputPressureCurrent, gas1TemperatureCurrent, gas1ActivityCurrent, gas1ParticleFractionCurrent,
    gas2VolumeFlowRateCurrent, gas2OutputPressureCurrent, gas2TemperatureCurrent, gas2ActivityCurrent, gas2ParticleFractionCurrent;
};

#endif // PROJECT_H
