#ifndef PROJECT_H
#define PROJECT_H

#include "../CalcElement/CalcElement.h"
#include "../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class MP_VOL_VOLOX : public CalcElement
{
public:
    MP_VOL_VOLOX();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);
	void setDataNames();
	
private:
    // Порты
    IInputPort *InPortQs, *InPortOmega, *InPortQG;
    IOutputPort *OutPortDO2, *OutPortV, *OutPortT;

    std::vector<double> nd_O2, nT, nV;

    // Параметры реторты
    double L, D, theta, mu, Tret;

    // Параметры модели
    double Nx, dx, dt;

    // Параметры ТВЭЛа
    double l, Num_tabl, h_tabl, d_tabl;

    // Параметры ОЯТ
    double rho_INF, M_INF, delta_UN, M_UN, rho_MUPNF;

    // Параметры газа
    double rho_G, M_G, Cair, Tw, delta_O2, rho_G_st;

    // Кинетич., т/динамич. параметры
    double k, Q1, Q2, beta;

    //константы
    double pi;
};

#endif // PROJECT_H
