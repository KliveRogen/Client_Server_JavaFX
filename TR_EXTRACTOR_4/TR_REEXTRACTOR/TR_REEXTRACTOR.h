#ifndef TR_REEXTRACTOR_H
#define TR_REEXTRACTOR_H

#include "../CalcElement/CalcElement.h"
#include "../include/IPort.h"

#include "../DynMath/InertOne.h"
#include "../DynMath/Delay.h"
#include "../DynMath/InertOneDelay.h"
#include "Valve.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

struct Flow {
    double Q;
    double T;
    double C;
};

struct ExtPump {
    double F;
    double P;
    double I;
};

struct ExtTank {
    ExtPump Pump;
    Valve Val;

    Flow FlowIn;
    Flow FlowOut;

    double R, H, V_pol, V_rab;
    double L, C, T;
};

struct Receiver {
    Valve V_06, V_07, V_62;

    double P;
};

class TR_REEXTRACTOR : public CalcElement
{

public:
    TR_REEXTRACTOR();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);

private:
    void createPorts();

    Receiver Rec;
    ExtTank Tank;
    ExtPump Pump;

    Flow F269, F220;
    Valve V_42, V_47;

    double r_n, h_n, r_v, h_v, h_vor, h_tn, h_tv;
    double GRF, L_voz, T_vpk, T_npk, L_vpk, L_npk, k_pit, k_pyl;
    double jkl, k_L_pk, k_C_U, L;

    IInputPort *IP_Mag, *IP_ExtPump, *IP_F220, *IP_F269, *IP_V07,
               *IP_V06, *IP_V42, *IP_VTank, *IP_PumpTank, *IP_Debug;

    IOutputPort *OP_F230, *OP_F235, *OP_Debug;

    InertOneDelay IOD;
    bool NeedInitDelays;
};

#endif // TR_REEXTRACTOR_H
