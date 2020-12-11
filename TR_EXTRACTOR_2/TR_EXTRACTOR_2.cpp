#include "TR_EXTRACTOR_2.h"

#include <math.h>

TR_EXTRACTOR_2::TR_EXTRACTOR_2()
{
    BlockCalcType = E_INITVALUES;

    r_n = 0.9;
    h_n = 1.25;
    r_v = 0.8;
    h_v = 1.2;
    h_vor = 0.6;
    h_tn = 3;
    h_tv = 7.5;
    jkl = 1;

    createPorts();
    NeedInitDelays = true;
}

ICalcElement *Create()
{
    return new TR_EXTRACTOR_2();
}

void Release(ICalcElement *block)
{
    if(block->type() == "TR_HEADEXTRACTOR") delete block;
}

std::string Type()
{
    return "TR_EXTRACTOR_2";
}


bool TR_EXTRACTOR_2::process(double t, double h, std::string &error)
{
    if(NeedInitDelays) {
        NeedInitDelays = false;
        IOD.init(10, h, 0, 1, 5);
    }

    double inVal = IP_Debug->getInput()[0];
    IOD.step(h, inVal);
    OP_Debug->setNewOut(0, IOD.value());

    // Входные данные
    // Давление магистрали
    double pMag = 0.00;
    if(IP_Mag->hasConnection()) pMag = IP_Mag->getInput()[0];

    // Внешний насос
    double extPumpOnOff = 0.00, extPumpGF = 0.00;
    if(IP_ExtPump->hasConnection()) {
        extPumpOnOff = IP_ExtPump->getInput()[0];
        extPumpGF    = IP_ExtPump->getInput()[1];
    }

    // Пр.60A
    if(IP_F60A->hasConnection()) {
        F60A.Q = IP_F60A->getInput()[0];
        F60A.T = IP_F60A->getInput()[1];
        F60A.C = IP_F60A->getInput()[2];
    }
    else {
        F60A.Q = 0.00;
        F60A.T = 0.00;
        F60A.C = 0.00;
    }

    // Пр.210
    if(IP_F210->hasConnection()) {
        F210.Q = IP_F210->getInput()[0];
        F210.T = IP_F210->getInput()[1];
        F210.C = IP_F210->getInput()[2];
    }
    else {
        F210.Q = 0.00;
        F210.T = 0.00;
        F210.C = 0.00;
    }

    // Клапаны ресивера
    // 07
    double v07o = 0.00, v07c = 0.00;
    if(IP_V07->inputDataSize() >= 2) {
        v07o = IP_V07->getInput()[0];
        v07c = IP_V07->getInput()[1];
    }

    // 06
    double v06o = 0.00, v06c = 0.00;
    if(IP_V06->inputDataSize() >= 2) {
        v06o = IP_V06->getInput()[0];
        v06c = IP_V06->getInput()[1];
    }

    // 42
    double v42o = 0.00, v42c = 0.00;
    if(IP_V42->inputDataSize() >= 2) {
        v42o = IP_V42->getInput()[0];
        v42c = IP_V42->getInput()[1];
    }

    // Клапан емкости
    double vto = 0.00, vtc = 0.00;
    if(IP_VTank->inputDataSize() >= 2) {
        vto = IP_VTank->getInput()[0];
        vtc = IP_VTank->getInput()[1];
    }

    // Насос емкости
    double tankPumpOnOff = 0.00, tankPumpGF = 0.00;
    if(IP_PumpTank->inputDataSize() >= 2) {
        tankPumpOnOff = IP_PumpTank->getInput()[0];
        tankPumpGF    = IP_PumpTank->getInput()[1];
    }

    /*// Расчет выносного центробежного насоса
    double gf, onoff;
    if(onoff == 0) gf = 0;
    Pump.F += h*(gf - Pump.F) / 5.00;
    Pump.I = 2.007e-06*Pump.F*Pump.F + 0.0009803*Pump.F + 2.992e-15;
    Pump.P = 4.774e-07*Pump.F*Pump.F + 0.001568*Pump.F + 8.347e-16;

    // Расчет ресивера
    // Расчет клапанов
    double v06o, v06c, v07o, v07c, v62o, v62c;
    double pMag;
    Rec.V_06.step(h, v06o, v06c);
    Rec.V_07.step(h, v07o, v07c);
    Rec.V_62.step(h, v62o, v62c);

    // Расчет давления в ресивере
    double dP = 0;
    if(Rec.V_06.getState() <= 1.00 && Rec.V_07.getState() <= 1.00) dP = 0.00;
    else if(Rec.V_07.getState() <= 1.00) dP = (0 - Rec.P) / (0.001*Rec.V_06.getState()*Rec.V_06.getState() + 0.05*Rec.V_06.getState() + 5);
    else dP = (pMag - Rec.P) / (0.001*Rec.V_06.getState()*Rec.V_06.getState() + 0.05*Rec.V_06.getState() + 5);
    Rec.P += h*dP;

    // Расчет бака-сборника
    // Расчет насоса
    double nk_gf, nk_onoff;
    if(nk_onoff == 0.00) nk_gf = 0.00;
    Tank.Pump.F += h*(nk_gf - Tank.Pump.F) / 5.00;
    Tank.Pump.P = 4.83e-05*exp(0.004045*Tank.Pump.F);
    Tank.Pump.I = -1.312e-06*Tank.Pump.F*Tank.Pump.F + 0.00727*Tank.Pump.F + 2.892e-15;

    // Расчет клапана
    double tank_vo, tank_vc;
    Tank.Val.step(h, tank_vo, tank_vc);

    // Расчет выходного расхода
    if(Tank.Val.getState() == 0.00 || Tank.Pump.P < 0.1) Tank.FlowOut.Q = 0;
    else Tank.FlowOut.Q = -8.102e-16 + 0.6075*Tank.Pump.P + Tank.Val.getState();

    // Расчет уровня, концентрации и температуры
    double dL, dC, dT;
    dL = (Tank.FlowIn.Q - Tank.FlowOut.Q) / 3600.00;
    if(L == 0) {
        dC = 0.00;
        dT = 0.00;
    }
    else {
        dC = Tank.FlowIn.Q * (Tank.FlowIn.C - Tank.C) / Tank.L / 3600.00;
        dT = Tank.FlowIn.Q * (Tank.FlowIn.T - Tank.T) / Tank.L / 3600.00;
    }
    Tank.L += h*dL;
    Tank.C += h*dC;
    Tank.T += h*dT;

    Tank.FlowOut.C = Tank.C;
    Tank.FlowOut.T = Tank.T;

    // Расчет экстракционной колонны
    // Расчет клапанов 42, 47
    double v42o, v42c, v47o, v47c;
    V_42.step(h, v42o, v42c);
    V_47.step(h, v47o, v47c);

    // Расчет входного потока 201
    double C201in, T201in;
    F201.Q = -0.0003077*V_42.getState()*V_42.getState() + 0.09077*V_42.getState()-1.16e-15;
    if(F201.Q > 0.00) {
        F201.C = C201in;
        F201.T = T201in;
    }
    else {
        F201.C = 0.00;
        F201.T = 0.00;
    }

    // Расчет потока 213
    if(V_47.getState() == 0.00 || Pump.P < 0.1) F213.Q = 0;
    else F213.Q = -5.128e-16 + 0.3218*Pump.P + 0.03103*V_47.getState();

    // Расчет ГРФ
    double dGRF = 0;
    dGRF = 100*((F201.Q - F213.Q) / (M_PI*r_v*r_v) / 3600)/h_n;
    GRF += h*dGRF;

    // Расчет потока 210
    if(L_voz >= h_vor) F210.Q = Tank.FlowOut.Q;
    else F210.Q = 0.00;

    // Расчет ВОЗ
    double dL_voz = 0.00;
    dL_voz = (Tank.FlowOut.Q - F210.Q)/(M_PI*r_v*r_v)/3600.00;
    L_voz += h*dL_voz;

    T_vpk = -11.13 + 3.111*h_tv + 1.004*((Tank.FlowOut.Q*Tank.FlowOut.T + F201.Q*F201.T)/(Tank.FlowOut.Q+F201.Q));
    T_npk = -11.13 + 3.111*h_tn + 1.004*((Tank.FlowOut.Q*Tank.FlowOut.T + F201.Q*F201.T)/(Tank.FlowOut.Q+F201.Q));
    F210.T = T_vpk;

    if(k_pit > 0) {
        L_vpk = k_pyl*(-0.005575*pow(2*Rec.V_07.getState()-(Rec.V_62.getState()/81)*Rec.V_07.getState(), 2) -0.3824*(2*Rec.V_07.getState()-(Rec.V_62.getState()/81)*Rec.V_07.getState()) + 99.75);
        L_npk = k_pyl*(-0.002646*pow(2*Rec.V_07.getState()-(Rec.V_62.getState()/81)*Rec.V_07.getState(), 2) -0.6691*(2*Rec.V_07.getState()-(Rec.V_62.getState()/81)*Rec.V_07.getState()) + 99.52);
    }
    else {
        double flag = 1;
        double dp = Rec.V_07.getState() + jkl*Rec.V_07.getState()/500.00;
        L_vpk = -0.005575*flag*dp*dp -0.3824*flag*dp + 99.75 + (0.25*(1-flag));
        L_npk = -0.002646*flag*dp*dp -0.6691*flag*dp + 99.52 + (0.48*(1-flag));
        jkl++;
    }

    if(L_vpk < 0) L_vpk = 0;
    if(L_npk < 0) L_npk = 0;
    if(Rec.V_06.getState() == 0) {
        L_vpk = 100;
        L_npk = 100;
    }

    k_L_pk = exp(-pow(((L_vpk+L_npk)/2-40)/23.35, 2));
    k_C_U = k_L_pk*(1/(1+84*exp(-9.8*F201.Q/Tank.FlowOut.Q)));
    F210.C = k_C_U*F201.C;
    */

    return true;
}

bool TR_EXTRACTOR_2::init(std::string &error, double h)
{
    std::vector<std::string> dn;
    dn.resize(3);
    dn[0] = "Расход";
    dn[1] = "Температура";
    dn[2] = "Концентрация";

    OP_F220->setDataNames(dn);
    OP_F222->setDataNames(dn);

    // Отладочный порт
    dn.resize(1);
    dn[0] = "Выход";
    OP_Debug->setDataNames(dn);

    return true;
}

void TR_EXTRACTOR_2::createPorts()
{
    // Входные порты
    IP_Mag      = createInputPort(0, "Магистраль", "INFO");
    IP_ExtPump  = createInputPort(1, "Выносной насос", "INFO");
    IP_F210     = createInputPort(2, "Вх. пр. 210", "INFO");
    IP_F60A     = createInputPort(3, "Вх. пр. 60A", "INFO");
    // IP_F222     = createInputPort(4, "Вх. пр. 222", "INFO");
    IP_V07      = createInputPort(4, "Кл. 07/1", "INFO");
    IP_V06      = createInputPort(5, "Кл. 06/1", "INFO");
    IP_V42     = createInputPort(6, "Кл. 42/2", "INFO");
    IP_VTank    = createInputPort(7, "Кл. емкости", "INFO");
    IP_PumpTank = createInputPort(8, "Насос", "INFO");
    IP_Debug    = createInputPort(9, "Отладочный вход", "INFO");

    // Выходные порты
    OP_F220  = createOutputPort(10, "Вых. пр. 220", "INFO");
    OP_F222  = createOutputPort(11, "Вых. пр. 222", "INFO");
    OP_Debug = createOutputPort(12, "Отладочный выход", "INFO");
}
