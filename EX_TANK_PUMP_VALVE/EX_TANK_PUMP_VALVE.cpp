#include "EX_TANK_PUMP_VALVE.h"

#include "../DynMath/Integrator.h"
#include "../DynMath/InertOne.h"

#include <math.h>

double valveFlowrate(double in) {
    return in / 3600;
}

EX_TANK_PUMP_VALVE::EX_TANK_PUMP_VALVE()
{
	// Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
	createParameter("HEIGHT", "1");
    createParameter("RADUIS", "1");
	createParameter("INIT_VOLUME", "100");
	createParameter("SU_HEIGHT", "0.9");
	createParameter("NU_HEIGHT", "0.1");
	createParameter("INIT_C_U", "0");
	createParameter("INIT_C_HNO3", "0");
	createParameter("INIT_T", "20");
    createParameter("K1", "0.003");
    createParameter("K2", "0.007");
    createParameter("Kpr", "1.11");
    createParameter("Tq", "6");
    createParameter("Tu", "9");
    createParameter("Thno3", "9");
    createParameter("Tt", "7");
    createParameter("Tf", "4");
    createParameter("Tp", "5");
    createParameter("Ti", "5");

    Int_L = new Integrator();
    IO_Q = new InertOne();
    IO_CU = new InertOne();
    IO_CHNO3 = new InertOne();
    IO_T = new InertOne();
    IO_F = new InertOne();
    IO_P = new InertOne();
    IO_I = new InertOne();

	// Сигналы блока
    SIG_VOLUME = createSignal("SIG_VOLUME", Signal::ST_DOUBLE);
    SIG_SU     = createSignal("SIG_SU", Signal::ST_BOOL);
    SIG_NU     = createSignal("SIG_NU", Signal::ST_BOOL);
    SIG_VALVE  = createSignal("SIG_VALVE", Signal::ST_DOUBLE);
    SIG_PUMP   = createSignal("SIG_PUMP", Signal::ST_BOOL);

	// Порты блока
    VALVE_PORT = createInputPort(0, "Управление клапаном", "INFO");
    PUMP_PORT  = createInputPort(1, "Управление насосом", "INFO");
    FLOW_IN    = createInputPort(2, "Входной поток", "INFO");
    FLOW_OUT   = createOutputPort(3, "Выходной поток", "INFO");

	// Отказы блока

}

bool EX_TANK_PUMP_VALVE::process(double t, double h, std::string &error)
{
    // Put your calculations here
    double valve_in = 0, valve_out = 0;
    double pump_in = 0, gf = 0;

    double Qin = 0, Tin = 0, Cuin = 0;

    if(VALVE_PORT->inputDataSize() >= 2) {
        valve_in = VALVE_PORT->getInput()[0];
        valve_out = VALVE_PORT->getInput()[1];
    }

    if(PUMP_PORT->inputDataSize() >= 2) {
        pump_in = PUMP_PORT->getInput()[0];
        gf = PUMP_PORT->getInput()[1];
    }

    if(FLOW_IN->inputDataSize() >= 3) {
        Qin = FLOW_IN->getInput()[0];
        Tin = FLOW_IN->getInput()[1];
        Cuin = FLOW_IN->getInput()[2];
        // Chno3in = FLOW_IN->getInput()[3];
    }

    // Расчет клапана
    double Tvalve = 10;
    VALVE_STATE += h * Tvalve * (valve_in - valve_out);
    if(VALVE_STATE > 100) VALVE_STATE = 100;
    if(VALVE_STATE < 0) VALVE_STATE = 0;

    double f, p, i, l, q, cu, temp;

    // Статическая модель
    if(pump_in != 0) f = gf;
    else f = 0;

    p = f * paramToDouble("K1");
    i = f * paramToDouble("K2");

    l = (Qin - IO_Q->value()) / (M_PI * paramToDouble("RADIUS") * paramToDouble("RADIUS"));
    q = p * paramToDouble("Kpr") * VALVE_STATE / 100.00;

    double den = (Int_L->value() * M_PI * paramToDouble("RADIUS") * paramToDouble("RADIUS"));
    cu = (Cuin * Qin - IO_CU->value() * IO_Q->value()) / den;
    temp = (Tin * Qin - IO_T->value() * IO_Q->value()) / den;

    // Динамика
    IO_Q->step(q, h);
    IO_F->step(f, h);
    IO_P->step(p, h);
    IO_I->step(i, h);
    IO_CU->step(cu, h);
    IO_T->step(temp, h);

    Int_L->step(l, h);

    FLOW_OUT->setNewOut(0, IO_Q->value());
    FLOW_OUT->setNewOut(1, IO_T->value());
    FLOW_OUT->setNewOut(2, IO_CU->value());

//    // Расчет компонентов
//    if(VOLUME == 0) {
//        M_U = 0;
//        M_HNO3 = 0;
//    }
//    else {
//        M_U += h * (Qin*Cuin - OUT_Q * M_U / VOLUME) / 3600.00;
//        M_HNO3 += h * (Qin*Chno3in - OUT_Q * M_HNO3 / VOLUME) / 3600.00;
//        VOLUME += h * (Qin - OUT_Q) / 3600.00;
//    }

//    if(M_U < 0) M_U = 0;
//    if(M_HNO3 < 0) M_HNO3 = 0;
//    if(VOLUME < 0) VOLUME = 0;

//    double Cu = 0, Chno3 = 0;
//    if(VOLUME != 0) {
//        Cu = M_U / VOLUME;
//        Chno3 = M_HNO3 / VOLUME;
//    }

//    FLOW_OUT->setNewOut(0, OUT_Q);
//    FLOW_OUT->setNewOut(1, TEMPERATURE);
//    FLOW_OUT->setNewOut(2, Cu);
//    FLOW_OUT->setNewOut(3, Chno3);

//    FLOW_OUT->NewOut[0] = OUT_Q;
//    FLOW_OUT->NewOut[1] = TEMPERATURE;
//    FLOW_OUT->NewOut[2] = Cu;
//    FLOW_OUT->NewOut[3] = Chno3;

//    if(VOLUME != 0) OUT_Q = valveFlowrate(VALVE_STATE) * pump_in;
//    else OUT_Q = 0;

    return true;
}

bool EX_TANK_PUMP_VALVE::init(std::string &error, double h)
{
    // Put your initialization here
    std::vector<std::string> dn;
    dn.resize(3);
    dn[0] = "Расход, м3/ч";
    dn[1] = "Температура, оС";
    dn[2] = "Конц. U, г/л";
    // dn[3] = "Конц. HNO3, г/л";
    FLOW_OUT->setDataNames(dn);

    IO_Q->init(0, 1, paramToDouble("Tq"));
    IO_CU->init(paramToDouble("INIT_C_U"), 1, paramToDouble("Tu"));
    IO_CHNO3->init(paramToDouble("INIT_C_HNO3"), 1, paramToDouble("Thno3"));
    IO_T->init(paramToDouble("INIT_T"), 1, paramToDouble("Tt"));
    IO_F->init(0, 1, paramToDouble("Tf"));
    IO_P->init(0, 1, paramToDouble("Tp"));
    IO_I->init(0, 1, paramToDouble("Ti"));

    Int_L->init(paramToDouble("INIT_VOLUME"), 1, 1);

//    double P_HEIGHT, P_SECTION, P_INIT_VOLUME, P_INIT_C_U, P_INIT_C_HNO3, P_INIT_T;
//    P_HEIGHT = paramToDouble("HEIGHT");
//    P_SECTION = paramToDouble("SECTION");
//    P_INIT_VOLUME = paramToDouble("INIT_VOLUME");
//    P_INIT_C_U = paramToDouble("INIT_C_U");
//    P_INIT_C_HNO3 = paramToDouble("INIT_C_HNO3");
//    P_INIT_T = paramToDouble("INIT_T");

//    VOLUME = P_HEIGHT * P_SECTION * P_INIT_VOLUME / 100.00;
//    if(VOLUME > P_HEIGHT * P_SECTION) VOLUME = P_HEIGHT * P_SECTION;
//    if(VOLUME <= 0) {
//        error = "Объем раствора не может быть меньше нуля!";
//        return false;
//    }

//    M_U = P_INIT_C_U * VOLUME;
//    M_HNO3 = P_INIT_C_HNO3 * VOLUME;

//    TEMPERATURE = P_INIT_T;
//    OUT_Q = 0;

//    double Cu = 0, Chno3 = 0;
//    if(VOLUME != 0) {
//        Cu = M_U / VOLUME;
//        Chno3 = M_HNO3 / VOLUME;
//    }

    FLOW_OUT->setOut(0, IO_Q->value());
    FLOW_OUT->setOut(1, IO_T->value());
    FLOW_OUT->setOut(2, IO_CU->value());
    FLOW_OUT->setOut(3, IO_CHNO3->value());

    return true;
}

ICalcElement *Create()
{
    return new EX_TANK_PUMP_VALVE();
}

void Release(ICalcElement *block)
{
    if(block->type() == "EX_TANK_PUMP_VALVE") delete block;
}

std::string Type()
{
    return "EX_TANK_PUMP_VALVE";
}
