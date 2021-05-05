#ifndef PROJECT_H
#define PROJECT_H

#include "../../CalcElement/CalcElement.h"
#include "../../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class GCS_AeroFilter : public CalcElement
{
public:
    GCS_AeroFilter();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);
	void setDataNames();
	
private:
	// Put your variables here
    IInputPort *inGasAeroFilter, *inFeedbackFilter;
    IOutputPort *outGasAeroFilter, *outAeroFilterParameters, *outFeedbackFilter;

    double signumFunc(double argVal);
    double inertionFunc(double operationTime, double valueGiven, double valuePrev,  double step);
    double filterParticleNumberPrev, filterActivityCurrent, situationFilterBrakePrev, timeCounter, particleMassPrev, gasPressurePrev, gasFlowRatePrev, gasParticleFractionPrev, gasActivityPrev, filterActivityPrev, timeCounterFilterBreak, timeCounterFilterClog;
    double gasVolumeFlowRateCurrent, gasOutputPressureCurrent, gasTemperatureCurrent, gasActivityCurrent, gasParticleFractionCurrent, particleMassPrevWork, filterActivityPrevWork,
    situationFilterClogPrev, filterResistantPrev, outFilterResistance, filterResistantCurrent;
    int flagFilterBreak, flagFilterClog;


};

#endif // PROJECT_H
