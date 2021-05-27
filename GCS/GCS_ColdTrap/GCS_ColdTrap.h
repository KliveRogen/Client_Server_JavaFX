#ifndef PROJECT_H
#define PROJECT_H

#include "../../CalcElement/CalcElement.h"
#include "../../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class GCS_ColdTrap : public CalcElement
{
public:
    GCS_ColdTrap();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);
	void setDataNames();
	
private:
	// Put your variables here
    IInputPort *inGasColdTrap, *inFeedback;
    IOutputPort *outGasColdTrap, *outColdTrapParameters, *outFeedback;

    double signumFunc(double argVal);
    double inertionFunc(double operationTime, double valueGiven, double valuePrev,  double step);
    double coldTrapParticleNumberPrev, coldTrapActivityCurrent, situationColdTrapLeakPrev, timeCounter, particleMassPrev, gasPressurePrev, gasFlowRatePrev, gasParticleFractionPrev, gasActivityPrev, coldTrapActivityPrev, timeCounterColdTrapLeak, timeCounterColdTrapClog;
    double gasVolumeFlowRateCurrent, gasOutputPressureCurrent, gasTemperatureCurrent, gasActivityCurrent, gasParticleFractionCurrent, particleMassPrevWork, coldTrapActivityPrevWork,
    situationColdTrapClogPrev, coldTrapResistantPrev, outColdTrapResistance, coldTrapResistantCurrent, gasTemperaturePrev;
    int flagColdTrapLeak, flagColdTrapClog;

};

#endif // PROJECT_H
