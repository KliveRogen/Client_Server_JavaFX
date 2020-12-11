#include "EX_EXTRACTOR.h"
#include "math.h"

#include "../DynMath/Delay.h"
#include "../DynMath/Integrator.h"
#include "../DynMath/InertOne.h"

EX_EXTRACTOR::EX_EXTRACTOR()
{
    // Расчетный тип блока
    BlockCalcType = E_INITVALUES;
    NeedInit = true;

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
    createParameter("VOR_H", "0.6");

    createParameter("T_cof", "260");
    createParameter("T_tof", "235");
    createParameter("T_cvf", "210");
    createParameter("T_tvf", "185");
    createParameter("T_tpk", "105");
    createParameter("Tau_cof", "7200");
    createParameter("Tau_tof", "2000");
    createParameter("Tau_cvf", "7300");
    createParameter("Tau_tvf", "2300");
    createParameter("Tau_tpk", "600");

    // Сигналы блока
    S_PKV   = createSignal("S_PKV", Signal::ST_DOUBLE);
    S_PKN   = createSignal("S_PKN", Signal::ST_DOUBLE);
    S_TV    = createSignal("S_TV", Signal::ST_DOUBLE);
    S_TN    = createSignal("S_TN", Signal::ST_DOUBLE);
    S_GRF   = createSignal("S_GRF", Signal::ST_DOUBLE);
    S_DUP62 = createSignal("S_DUP62", Signal::ST_DOUBLE);
    S_DUP42 = createSignal("S_DUP42", Signal::ST_DOUBLE);
    S_DUP47 = createSignal("S_DUP47", Signal::ST_DOUBLE);

    // Порты блока
    IN_VF      = createInputPort(0, "Пр.201", "INFO");
    IN_OF      = createInputPort(1, "Пр.200", "INFO");
    IN_VF_add  = createInputPort(2, "Пр.222", "INFO");
    IN_P_REC   = createInputPort(3, "Давл. ресивера", "INFO");
    IN_P_PUMP  = createInputPort(4, "Давл. насоса", "INFO");
    IN_P_V47   = createInputPort(5, "Клапан 47", "INFO");

    OUT_OF     = createOutputPort(6, "Пр.210", "INFO");
    OUT_VF     = createOutputPort(7, "Пр.213", "INFO");
    OUT_LEVELS = createOutputPort(8, "Уровни", "INFO");

    // Отказы блока

}

bool EX_EXTRACTOR::process(double t, double h, std::string &error)
{
    if(NeedInit) {
        NeedInit = false;

        // Динамические звенья модели
        // Концентрация урана в ОФ
        W_U_OF = new InertOne();
        W_U_OF->init(0, 1, paramToDouble("T_cof"));
        D_U_OF = new Delay();
        D_U_OF->init(paramToDouble("Tau_cof"), h, 0);

        // Температура ОФ
        W_T_OF = new InertOne();
        W_T_OF->init(0, 1, paramToDouble("T_tof"));
        D_T_OF = new Delay();
        D_T_OF->init(paramToDouble("Tau_tof"), h, 0);

        // Концентрация урана в ВФ
        W_U_VF = new InertOne();
        W_U_VF->init(0, 1, paramToDouble("T_cvf"));
        D_U_VF = new Delay();
        D_U_VF->init(paramToDouble("Tau_cvf"), h, 0);

        // Температура ВФ
        W_T_VF = new InertOne();
        W_T_VF->init(0, 1, paramToDouble("T_tvf"));
        D_T_VF = new Delay();
        D_T_VF->init(paramToDouble("Tau_tvf"), h, 0);

        // Уровень ВОЗ
        W_L_VOZ = new Integrator();
        W_L_VOZ->init(0, 1, 1);

        // Уровень ПК
        W_L_PK = new Integrator();
        W_L_PK->init(0, 1, 1);

        // Уровень НОЗ
        W_L_NOZ = new Integrator();
        W_L_NOZ->init(0, 1, 1);

        // Температура ПК
        W_T_PK = new InertOne();
        W_T_PK->init(0, 1, paramToDouble("T_tpk"));
        D_T_PK = new Delay();
        D_T_PK->init(paramToDouble("Tau_tpk"), h, 0);
    }

    // Получение входных значений
    // ОФ
    double Q_OF_IN = 0, C_OF_IN = 0, T_OF_IN = 0;
    if(IN_OF->inputDataSize() >= 3) {
        Q_OF_IN = IN_OF->getInput()[0];
        T_OF_IN = IN_OF->getInput()[1];
        C_OF_IN = IN_OF->getInput()[2];
    }

    // ВФ
    double Q_VF_IN = 0, C_VF_IN = 0, T_VF_IN = 0;
    if(IN_VF->inputDataSize() >= 3) {
        Q_VF_IN = IN_VF->getInput()[0];
        T_VF_IN = IN_VF->getInput()[1];
        C_VF_IN = IN_VF->getInput()[2];
    }

    // ВФ для головного экстрактора
    double Q_VF2_IN = 0, C_VF2_IN = 0, T_VF2_IN = 0;
    if(IN_VF_add->inputDataSize() >= 3) {
        Q_VF2_IN = IN_VF_add->getInput()[0];
        T_VF2_IN = IN_VF_add->getInput()[1];
        C_VF2_IN = IN_VF_add->getInput()[2];
    }

    return true;
}

bool EX_EXTRACTOR::init(std::string &error, double h)
{
    // Put your initialization here
    std::vector<std::string> dn;
    dn.resize(3);
    dn[0] = "Расход, м3/ч";
    dn[1] = "Температура, оС";
    dn[2] = "Конц. урана, г/л";
    OUT_OF->setDataNames(dn);
    OUT_VF->setDataNames(dn);

    dn.resize(4);
    dn[0] = "ВОЗ, %";
    dn[1] = "ПКВ, %";
    dn[2] = "ПКН, %";
    dn[3] = "ГРФ, %";
    OUT_LEVELS->setDataNames(dn);

    return true;
}

ICalcElement *Create()
{
    return new EX_EXTRACTOR();
}

void Release(ICalcElement *block)
{
    if(block->type() == "EX_EXTRACTOR") delete block;
}

std::string Type()
{
    return "EX_EXTRACTOR";
}
