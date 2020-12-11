#ifndef PROJECT_H
#define PROJECT_H

#include "../../CalcElement/CalcElement.h"
#include "../../include/IPort.h"
#include "../TR_VAPE_CALC/tr_vape_calc.h"

#include <iostream>
#include <stdio.h>
#include <vector>

/*
#define NUM_ACT_ANAL 0
#define NUM_SEN_ANAL 1
#define NUM_ACT_DIG 0
#define NUM_SEN_DIG 0
*/

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();


/*
using namespace typc;
namespace uniqueConsts
{
    const double    MAX_MASS = 237*dens;
    const std::string     blockName = "01)АД-6359(емк.)";
}

using namespace uniqueConsts;
*/

class TR_VAPE_01T_AD6359 : public CalcElement, private Tank
{
public:
    TR_VAPE_01T_AD6359();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);
	void setDataNames();

private:
    IInputPort *inp1,*inp2,*ins;
    IOutputPort *outp1, *outp2, *outsen;
    Signal *signAnalSenLvl,*signAnalActValveOpen,*signAnalActValveClose;
    std::vector<analUnit> analAct,analSen;
    int ctrlValve;
    double valveRate,valvePos;

    void getInputs(double h);
    void getInitValues();
    void getParams();
    void setAct(double h);
    void setSen();
    void setOutputs();
    /*
    calcPrep();
    tempMix();
    massCalc();
    /*
    Signal *sigMonLvl;
    actOrSen senAnal[NUM_SEN_ANAL];
    char errorCode;
    double  mass[el],temp;
    */

};

#endif // PROJECT_H
