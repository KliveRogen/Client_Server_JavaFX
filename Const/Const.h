#ifndef CONST_H
#define CONST_H

#include "../CalcElement/CalcElement.h"
#include "../include/IPort.h"

extern "C" ICalcElement* Q_DECL_EXPORT Create();
extern "C" void Q_DECL_EXPORT Release(ICalcElement *block);
extern "C" std::string Q_DECL_EXPORT Type();

class Const : public CalcElement
{
public:
    Const();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);

private:
    double Value;
    IOutputPort *OutPort;
    Signal *ConstSig;
};

#endif // CONST_H
