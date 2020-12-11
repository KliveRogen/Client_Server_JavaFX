#ifndef PROJECT_H
#define PROJECT_H

#define NUM_ACT_ANAL 1
#define NUM_SEN_ANAL 1
#define NUM_ACT_DIG 0
#define NUM_SEN_DIG 0

#include "../../CalcElement/CalcElement.h"
#include "../../include/IPort.h"
#include "../TR_VAPE_CALC/tr_vape_calc.h"
//#include "string.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

namespace uniqueConsts
{
    const double    coeffPowFlow = 10/1e2,
                    coeffPowPress = 9/1e2,
                    timePowInert = 5;
    const std::string     blockName = "02)АД-3742/1(нас.)";
}

using namespace uniqueConsts;
using namespace typc;
class TR_VAPE_02P_AD37421 : public CalcElement
{
public:
    TR_VAPE_02P_AD37421();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);
	void setDataNames();

private:
    IInputPort *ins;
    IOutputPort *outp, *outs, *outinfo, *outinfosen;
    Signal *sigMonPress,*sigCtrlPow;
    actOrSen    actAnal[NUM_ACT_ANAL],
                senAnal[NUM_SEN_ANAL];
    double powSet;
    char errorCode;
};

#endif // PROJECT_H
