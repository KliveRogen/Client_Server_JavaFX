#include "EX_EXTRACTOR_2_4.h"
#include "math.h"

EX_EXTRACTOR_2_4::EX_EXTRACTOR_2_4()
{
	// Расчетный тип блока
    BlockCalcType = E_UNKNOWN;

    // Параметры блока
	createParameter("K1", "0.937");
	createParameter("K2", "1.711");
	createParameter("A", "104");
	createParameter("B", "-1.205");
	createParameter("C", "-4");
	createParameter("EX_D_NOZ", "1.8");
	createParameter("EX_D_SZN", "1.28");
	createParameter("EX_D_SZV", "1.28");
	createParameter("EX_D_VOZ", "1.6");
	createParameter("T_CONC", "50");
	createParameter("TAU_CONC", "200");
	createParameter("VOR_H", "0.6");

	// Сигналы блока
	createSignal("S_PK", Signal::ST_DOUBLE);
	createSignal("S_T", Signal::ST_DOUBLE);
	createSignal("S_GRF", Signal::ST_DOUBLE);
	createSignal("S_DUP62", Signal::ST_DOUBLE);
	createSignal("S_DUP47", Signal::ST_DOUBLE);

	// Порты блока
	createInputPort(0, "Пр.60а", "INFO");
	createInputPort(1, "Пр.222", "INFO");
	createInputPort(2, "Пр.222", "INFO");
	createInputPort(3, "Давл. ресивера", "INFO");
	createInputPort(4, "Давл. насоса", "INFO");
	createInputPort(5, "Клапан 47", "INFO");
	createOutputPort(6, "Пр.210", "INFO");
	createOutputPort(7, "Пр.220", "INFO");
	createOutputPort(8, "Уровни", "INFO");

	// Отказы блока

}

bool EX_EXTRACTOR_2_4::process(double t, double h, std::string &error)
{
    // Put your calculations here
    // Инициализация входных переменных
    double Q201 = 0, Q200 = 0, Q222 = 0;
    double T201 = 0, T200 = 0, T222 = 0;
    double C201 = 0, C200 = 0, C222 = 0;

    double P_REC = 0, P_NAS = 0;
    double DYP_47 = 100;

    double P_K1, P_K2, P_A, P_B, P_C, P_D_NOZ, P_D_SZN, P_D_SZV, P_D_VOZ, P_T_CONC, P_TAU_CONC, P_VOR_H;
    P_K1 = paramToDouble("K1");
    P_K2 = paramToDouble("K2");
    P_A = paramToDouble("A");
    P_B = paramToDouble("B");
    P_C = paramToDouble("C");
    P_D_NOZ = paramToDouble("EX_D_NOZ");
    P_D_SZN = paramToDouble("EX_D_SZN");
    P_D_SZV = paramToDouble("EX_D_SZV");
    P_D_VOZ = paramToDouble("EX_D_VOZ");
    P_T_CONC = paramToDouble("T_CONC");
    P_TAU_CONC = paramToDouble("TAU_CONC");
    P_VOR_H = paramToDouble("VOR_H");

    // Инициализация выходных переменных
    double Q210 = 0, T210 = 0, C210 = 0;
    double Q213 = 0, T213 = 0, C213 = 0;
    double L_pkn, L_pkv, L_voz;

    // Получение входных значений
    if(IN_P_201->inputDataSize() >= 3) {
        Q201 = IN_P_201->getInput()[0];
        T201 = IN_P_201->getInput()[1];
        C201 = IN_P_201->getInput()[2];
    }

    if(IN_P_200->inputDataSize() >= 3) {
        Q200 = IN_P_200->getInput()[0];
        T200 = IN_P_200->getInput()[1];
        C200 = IN_P_200->getInput()[2];
    }

    if(IN_P_222->inputDataSize() >= 3) {
        Q222 = IN_P_222->getInput()[0];
        T222 = IN_P_222->getInput()[1];
        C222 = IN_P_222->getInput()[2];
    }

    if(IN_P_REC->inputDataSize() >= 1) {
        P_REC = IN_P_REC->getInput()[0];
    }

    if(IN_P_PUMP->inputDataSize() >= 1) {
        P_NAS = IN_P_PUMP->getInput()[0];
    }

    double v47open = 0, v47close = 0;
    if(IN_P_V47->inputDataSize() >= 2) {
        v47open = IN_P_V47->getInput()[0];
        v47close = IN_P_V47->getInput()[1];
    }

    // Расчет статической модели
    double bb = Q200/(Q201 + Q222);
    double k3 = 1/(P_A*exp(P_B*bb));

    if(Q201 == 0 && Q222 == 0) Q213 = 0;
    else Q213 = P_K1*P_NAS*V_47/100;

    L_pkn = ((Q201/4 + Q222/4 + Q200/2) / 3600.00 - (650/(1+C201+C222)*P_REC*P_K2/3600))/(M_PI*(P_D_SZN*P_D_SZN)/4);
    L_pkv = ((Q201/4 + Q222/4 + Q200/2) / 3600.00 - (650/(1+C201+C222)*P_REC*P_K2/3600))/(M_PI*(P_D_SZV*P_D_SZV)/4);
    L_voz = (2*(650/(1+C201+C222))*P_REC*P_K2/3600.00 - V_Q210/3600.00) / (M_PI*P_D_VOZ*P_D_VOZ/4);

    if(V_L_VOZ < P_VOR_H) Q210 = 0;
    else if(Q200 == 0) Q210 = 0;
    else {
        double dh = V_L_VOZ - P_VOR_H;
        Q210 = (28479*dh*dh*dh) - 8655.9*dh*dh + 1052.1*dh + 1.4392;
    }

    // double GRF = (Q201/2 + Q222/2) / 3600.00 - (V_Q213/3600.00)/(M_PI*P_D_NOZ*P_D_NOZ/4);
    double GRF = ((Q201/2.0 + Q222/2.0 - Q213) / 3600.00) / (M_PI*P_D_NOZ*P_D_NOZ/4);
    double T_sr = (Q201*T201 + Q222*T222+Q200*T200) / (Q201+Q222+Q200);

    if(Q210 == 0) {
        T210 = 0;
        C210 = 0;
    }
    else {
        T210 = T_sr+10;
        C210 = (C201*Q201 + C222*Q222) * k3 / (Q201+Q222);
    }

    if(Q213 == 0) {
        C213 = 0;
        T213 = 0;
    }
    else {
        T213 = T_sr-2;
        C213 = (C201*Q201 + C222*Q222) * (1-k3) / (Q201+Q222);
    }

    // Расчет динамики
    // Уровни
    V_L_PKN += h*L_pkn;
    V_L_PKV += h*L_pkv;
    V_GRF += h*GRF;
    V_L_VOZ += h*L_voz;

    V_47 += h*(v47open - v47close)/40.0;
    if(V_47 > 100.0) V_47 = 100.0;
    if(V_47 < 0) V_47 = 0;

    // Расходы
    V_Q210 += h*(Q210 - V_Q210) / P_T_CONC;
    V_Q213 += h*(Q213 - V_Q213) / P_T_CONC;

    // Концентрации
    V_C210 += h*(C210 - V_C210) / P_T_CONC;
    V_C213 += h*(C213 - V_C213) / P_T_CONC;

    // Температуры
    V_T210 += h*(T210 - V_T210) / P_T_CONC;
    V_T213 += h*(T213 - V_T213) / P_T_CONC;

    // Выходные значения
    OUT_P_210->setNewOut(0, V_Q210);
    OUT_P_210->setNewOut(1, V_T210);
    OUT_P_210->setNewOut(2, V_C210);
//    OUT_P_210->NewOut[0] = V_Q210;
//    OUT_P_210->NewOut[1] = V_T210;
//    OUT_P_210->NewOut[2] = V_C210;

    OUT_P_213->setNewOut(0, V_Q213);
    OUT_P_213->setNewOut(1, V_T213);
    OUT_P_213->setNewOut(2, V_C213);
//    OUT_P_213->NewOut[0] = V_Q213;
//    OUT_P_213->NewOut[1] = V_T213;
//    OUT_P_213->NewOut[2] = V_C213;

    OUT_P_LEVELS->setNewOut(0, V_L_VOZ);
    OUT_P_LEVELS->setNewOut(1, V_L_PKN);
    OUT_P_LEVELS->setNewOut(2, V_L_PKV);
    OUT_P_LEVELS->setNewOut(3, V_GRF);
//    OUT_P_LEVELS->NewOut[0] = V_L_VOZ;
//    OUT_P_LEVELS->NewOut[1] = V_L_PKN;
//    OUT_P_LEVELS->NewOut[2] = V_L_PKV;
//    OUT_P_LEVELS->NewOut[3] = V_GRF;

    return true;
}

bool EX_EXTRACTOR_2_4::init(std::string &error, double h)
{
    // Put your initialization here
    std::vector<std::string> dn;
    dn.resize(3);
    dn[0] = "Расход, м3/ч";
    dn[1] = "Температура, оС";
    dn[2] = "Конц. урана, г/л";
    OUT_P_210->setDataNames(dn);
    OUT_P_213->setDataNames(dn);

    dn.resize(4);
    dn[0] = "ВОЗ, %";
    dn[1] = "ПКВ, %";
    dn[2] = "ПКН, %";
    dn[3] = "ГРФ, %";
    OUT_P_LEVELS->setDataNames(dn);

    return true;
}

ICalcElement *Create()
{
    return new EX_EXTRACTOR_2_4();
}

void Release(ICalcElement *block)
{
    if(block->type() == "EX_EXTRACTOR_2_4") delete block;
}

std::string Type()
{
    return "EX_EXTRACTOR_2_4";
}
