#ifndef PROJECT_H
#define PROJECT_H

#include "../CalcElement/CalcElement.h"
#include "../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

#include <QVector>

class MP_EX_EXTRACTOR : public CalcElement
{
public:
    MP_EX_EXTRACTOR();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);

private:
    double Y_equilibrium(int ComponentId, int mytyper);
    std::vector<double> dydt(double t, std::vector<double> &y0);
    void ode1(std::vector<double> &y0, std::vector<double> &y, double t, double h);

	// Put your variables here
    IInputPort *InOF, *InVF, *InEng;
    IOutputPort *OutOF, *OutVF, *OutSensors;

    QVector<double> m_Concentrations_X_in;
    QVector<double> m_Concentrations_Y_equilibrium;
    QVector<double> m_Concentrations_Y_in;
    QVector<double> m_Concentrations_X_out;
    QVector<double> m_Concentrations_Y_out;
    QVector<double> m_Concentrations_dX;
    QVector<double> m_Concentrations_dY;

    QVector<double>  m_Ko;
    QVector<double>  m_K;

    QVector<double> m_Concentrations_rk_dX;
    QVector<double> m_Concentrations_rk_dY;
    QVector<double> m_Concentrations_rk_X_out;
    QVector<double> m_Concentrations_rk_Y_out;


    QVector<double> m_input_Components_Mass;
    QVector<double> m_current_Components_Mass;
    QVector<double> m_current_Components_Concentration;

    //  QVector<double> m_equilibrium_Concentrations;
    QVector<double> m_G;

    unsigned int m_temp_miss;

    double m_Qx_in;
    double m_Qx_out;
    double m_Qy_in;
    double m_Qy_out;

    double m_f;

    double m_dQx;
    double m_dQy;
    double m_dVx;
    double m_dVy;

    double m_Vx;
    double m_Vy;
    double m_Vx_stat;
    double m_Vy_stat;

    double m_T;
    double m_T_prev;
    double m_Ty;
    double m_Tx;

    double m_T0;

    double m_py;
    double m_px;
    double m_sigma;
    double m_dk;
    double m_A;
    //double m_Tq;
    double m_Tqx;
    double m_Tqy;
    double m_Tqvx;
    double m_Tqvy;
    double m_hs;

    double Prev_H;
    double H;
    double m_geometry_S;
    double m_geometry_H;

    double m_const_k ;
    double m_geometry_V0;

    double m_dm;
    double m_Vs;
    double m_Vr;
    double m_volume;

    double m_currentvolume;

    double minQ;
    double mint;
    double minV;
};

#endif // PROJECT_H
