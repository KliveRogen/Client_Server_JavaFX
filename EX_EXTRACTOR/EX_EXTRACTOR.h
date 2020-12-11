#ifndef PROJECT_H
#define PROJECT_H

#include "../CalcElement/CalcElement.h"
#include "../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class InertOne;
class Integrator;
class Delay;

class EX_EXTRACTOR : public CalcElement
{
public:
    EX_EXTRACTOR();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);

private:
	// Put your variables here
    Signal *S_PKV, *S_PKN, *S_TV, *S_TN, *S_GRF, *S_DUP62, *S_DUP42, *S_DUP47;
//    IInputPort *IN_P_201, *IN_P_200, *IN_P_222, *IN_P_REC, *IN_P_PUMP, *IN_P_V47;
//    IOutputPort *OUT_P_210, *OUT_P_213, *OUT_P_LEVELS;
    IInputPort *IN_P_REC, *IN_P_PUMP, *IN_P_V47;
    IInputPort *IN_OF, *IN_VF, *IN_VF_add;
    IOutputPort *OUT_OF, *OUT_VF, *OUT_LEVELS;

//    double V_Q210,V_C210,V_T210,V_Q213,V_C213,V_T213,V_L_PKN,V_L_PKV,V_L_VOZ,V_GRF,V_47;
    InertOne *W_U_OF, *W_T_OF, *W_U_VF, *W_T_VF, *W_T_PK;
    Delay *D_U_OF, *D_T_OF, *D_U_VF, *D_T_VF, *D_T_PK;
    Integrator *W_L_VOZ, *W_L_NOZ, *W_L_PK;

    bool NeedInit;
};

#endif // PROJECT_H
