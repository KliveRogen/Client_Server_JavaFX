#ifndef PROJECT_H
#define PROJECT_H

#include "../../CalcElement/CalcElement.h"
#include "../../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class MP_Ref_Receiver6381 : public CalcElement
{
public:
    MP_Ref_Receiver6381();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);
	void setDataNames();
    std::vector<double> dydt();
	
private:
	// Put your variables here
    IInputPort *PortIn1,*PortIn2;
    IOutputPort *PortOut1;
    std::vector<double> inVect1,inVect2;
    double DYP06, DYP07,P,Pres,Pl,kmag;
};

#endif // PROJECT_H
