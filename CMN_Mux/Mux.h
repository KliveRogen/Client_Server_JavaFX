#ifndef PROJECT_H
#define PROJECT_H

#include "../CalcElement/CalcElement.h"
#include "../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class Mux : public CalcElement
{
public:
    Mux();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);
    virtual void loadParameters(QDomElement &domParams);

private:
	// Put your variables here
    IOutputPort *OutPort;
    std::vector<IInputPort*> InPortsVect;
    std::vector<double> OutVect;
};

#endif // PROJECT_H
