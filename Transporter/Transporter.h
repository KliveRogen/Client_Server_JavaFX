#ifndef PROJECT_H
#define PROJECT_H

#include <vector>
#include <deque>

#include "../CalcElement/CalcElement.h"
#include "../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class Transporter : public CalcElement
{
public:
    Transporter();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);

private:
	// Put your variables here
    Signal *ProgressSig, *ElectroSig, *MechSig;

    IInputPort *InPort, *ConPort;
    IOutputPort *OutPort;

    std::deque< std::vector<double> > TransportVect;
    std::deque<double> TransportDelays;
    std::vector<double> OutVect;

    double TransportTime;
    bool NewItem;
};

#endif // PROJECT_H
