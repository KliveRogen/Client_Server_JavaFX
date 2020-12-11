#ifndef PROJECT_H
#define PROJECT_H

#include "../../CalcElement/CalcElement.h"
#include "../../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class MP_Ref_TankColl6310 : public CalcElement
{
public:
    MP_Ref_TankColl6310();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);
	void setDataNames();
    std::vector<double> dydt();
	
private:
	// Put your variables here
    IInputPort *PortIn1;
    IOutputPort *PortOut1, *PortOut2;
    std::vector<double> inVect1;
    double Qin, Tin, Cin, kavar, F, GF, P, I, Qout, Tout, Cout, kraz, L;
    int Power;
};

#endif // PROJECT_H
