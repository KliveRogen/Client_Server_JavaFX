#ifndef PROJECT_H
#define PROJECT_H

#include "../CalcElement/CalcElement.h"
#include "../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class MP_EV_EVAPORATOR : public CalcElement
{
public:
    MP_EV_EVAPORATOR();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);

private:
    std::vector<double> dydt(std::vector<double> y0, double h);
private:
    // Put your variables here

    IInputPort *PortInPump, *PortInLiq;
    IOutputPort *PortOutLiq, *PortOutSensors;

    //constants

    double  A;
    double  Cf;
    double  Cp;
    double Cw ;
    double  pr;
    double a[5];
    double pf;
    double lambda;

    double dy[5];

    double h; //%%уровень
    double pp;//  %плотность раствора на выходе

    double mp; // %масса раствора на выходе
    double Tkip;
    double XX;
    double FF;
    double c_U;// %концентрация урана
    double c_HNO3; // %концентрация азотной кислоты
    double Ws; // %концентрация азотной кислоты

    double m_Ts;
    double m_Ps;

    std::vector<double> m_input_Components_Mass;
    std::vector<double> m_current_Components_Mass;
    std::vector<double> m_current_Components_Concentration;
    std::vector<double> m_Concentrations_X_in;
    std::vector<double> m_Concentrations_X_out;
    std::vector<double> m_Concentrations_dX;

    double m_T;
    double m_h;
    double m_Ty;
    double m_T_in;

    bool m_not_initalized;

    double m_Diametr;
    double m_Height;

    double m_dLdt ;

    double m_pp;

    double m_dX_out ;

    double m_dP ;

    double m_F_in;
    double m_F_out;
    double m_F_steam;
    double m_F_condesat;

    double m_P;

    double m_M;
    double m_c;

    double m_X_in;
    double m_X_out;

    double T_out;
    double T_steam ;

    double m_Qx_in;
    double m_Qx_out;

};

#endif // PROJECT_H
