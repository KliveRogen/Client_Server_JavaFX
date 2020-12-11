#include "EX_RECEIVER.h"

EX_RECEIVER::EX_RECEIVER()
{
    // Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока

    // Сигналы блока

    // Порты блока
    PORT_IN_MAG = createInputPort(0, "Магистраль", "INFO");
    PORT_IN_K1  = createInputPort(1, "К1", "INFO");
    PORT_IN_K2  = createInputPort(2, "К2", "INFO");

    PORT_OUT_SENSORS  = createOutputPort(3, "Датчики", "INFO");
    PORT_OUT_PRESSURE = createOutputPort(4, "Выход ресивера", "INFO");

    // Отказы блока

}

bool EX_RECEIVER::process(double t, double h, std::string &error)
{
    // Put your calculations here
    double k1_op = 0, k1_cl = 0, k2_op = 0, k2_cl = 0;

    // Получение входных значений
    if(PORT_IN_K1->hasConnection() && PORT_IN_K1->inputDataSize() >= 2) {
        k1_op = PORT_IN_K1->getInput()[0];
        k1_cl = PORT_IN_K1->getInput()[1];
    }

    if(PORT_IN_K2->hasConnection() && PORT_IN_K2->inputDataSize() >= 2) {
        k2_op = PORT_IN_K2->getInput()[0];
        k2_cl = PORT_IN_K2->getInput()[1];
    }

    double presIn = 0;
    if(PORT_IN_MAG->hasConnection() && PORT_IN_MAG->inputDataSize() >= 1) presIn = PORT_IN_MAG->getInput()[0];

    // Расчет состояний клапанов
    double Tvalve = 5;
    K1_STATE += h*Tvalve*(k1_op - k1_cl);
    if(K1_STATE > 100) K1_STATE = 100;
    if(K1_STATE < 0)   K1_STATE = 0;

    K2_STATE += h*Tvalve*(k2_op - k2_cl);
    if(K2_STATE > 100) K2_STATE = 100;
    if(K2_STATE < 0)   K2_STATE = 0;

    // Расчет ресивера
    double dP = 0;
    if(K1_STATE == 0 && K2_STATE != 0) {
        double T = 2000 - K2_STATE*19.8;
        dP = (0 - REC_PRESSURE) / T;
    }
    else if(K1_STATE != 0 && K2_STATE == 0) {
        double T = 2000 - K1_STATE*19.8;
        dP = (presIn - REC_PRESSURE) / T;
    }
    else if(K1_STATE != 0 && K2_STATE != 0) {
        double T = 2000 - K1_STATE*19.8;
        dP = (presIn - REC_PRESSURE) / T;
    }
    else {
        dP = 0;
    }

    REC_PRESSURE += h*dP;

    PORT_OUT_SENSORS->setNewOut(0, REC_PRESSURE);
    PORT_OUT_SENSORS->setNewOut(1, K1_STATE);
    PORT_OUT_SENSORS->setNewOut(2, K2_STATE);

//    PORT_OUT_SENSORS->NewOut[0] = REC_PRESSURE;
//    PORT_OUT_SENSORS->NewOut[1] = K1_STATE;
//    PORT_OUT_SENSORS->NewOut[2] = K2_STATE;

    PORT_OUT_PRESSURE->setNewOut(0, REC_PRESSURE);
//    PORT_OUT_PRESSURE->NewOut[0] = REC_PRESSURE;

    return true;
}

bool EX_RECEIVER::init(std::string &error, double h)
{
    // Put your initialization here
    std::vector<std::string> dn;
    dn.resize(3);
    dn[0] = "Давление в ресивере, Па";
    dn[1] = "Квх,  %";
    dn[2] = "Квых, %";
    PORT_OUT_SENSORS->setDataNames(dn);

    dn.resize(1);
    dn[0] = "Давление, Па";
    PORT_OUT_PRESSURE->setDataNames(dn);

    REC_PRESSURE = 0;
    K1_STATE = 0;
    K2_STATE = 0;

    return true;
}

ICalcElement *Create()
{
    return new EX_RECEIVER();
}

void Release(ICalcElement *block)
{
    if(block->type() == "EX_RECEIVER") delete block;
}

std::string Type()
{
    return "EX_RECEIVER";
}
