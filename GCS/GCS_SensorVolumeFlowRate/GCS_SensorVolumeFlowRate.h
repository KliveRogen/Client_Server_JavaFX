#ifndef PROJECT_H
#define PROJECT_H

#include "../../CalcElement/CalcElement.h"
#include "../../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class GCS_SensorVolumeFlowRate : public CalcElement
{
public:
    GCS_SensorVolumeFlowRate();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);
	void setDataNames();
	
private:
	// Put your variables here
    IInputPort *inPort;
    IOutputPort *outPort;
    Signal *flowRateValue, *sensorFlowRateUnit;
    double gasInputFlowRate, valueCoef;
    int situationValueRandomPrev;

};

#endif // PROJECT_H