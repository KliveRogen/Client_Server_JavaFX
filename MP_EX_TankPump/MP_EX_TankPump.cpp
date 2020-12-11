#include "MP_EX_TankPump.h"

double lToM3() {
    return 1 / 1000.00;
}

double M3ToL() {
    return 1000.00;
}

double secToHour() {
    return 1 / 3600.00;
}

double HourToSec() {
    return 3600.00;
}

MP_EX_TankPump::MP_EX_TankPump()
{
	// Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
    createParameter("TankHeight", "0.3");
    createParameter("TankSection", "0.031");
    createParameter("TankInitVolume", "90");
    createParameter("TankInitTemp", "20");
    createParameter("C_U", "0");
    createParameter("C_Pu", "0");
    createParameter("C_HNO3", "0");
    createParameter("C_Np", "0");
    createParameter("FLOW_ES", "20");

	// Сигналы блока
	createSignal("FLOWRATE", Signal::ST_DOUBLE);
    createSignal("LEVEL", Signal::ST_DOUBLE);

	// Порты блока
    PortFlowRate = createInputPort(0, "Уставка расхода", "INFO");
    PortFlowIn = createInputPort(1, "Прием раствора", "INFO");
    PortFlowOut = createOutputPort(2, "Подача раствора", "INFO");
    PortSensors = createOutputPort(3, "Датчики", "INFO");

	// Отказы блока
    createSituation("ES_DOSATION");
}

bool MP_EX_TankPump::init(std::string &error, double h)
{
    // Предварительный расчет
    double th, ts;
    th = paramToDouble("TankHeight");
    ts = paramToDouble("TankSection");

    if(th <= 0) return false;
    if(ts <= 0) return false;

    TankVolume = th*ts;
    SolVolume  = TankVolume*paramToDouble("TankInitVolume");
    if(SolVolume < 0) return false;

    C_U = paramToDouble("C_U");
    C_Pu = paramToDouble("C_Pu");
    C_HNO3 = paramToDouble("C_HNO3");
    C_Np = paramToDouble("C_Np");
    Temp = paramToDouble("TankInitTemp");

    if(C_U < 0)    return false;
    if(C_Pu < 0)   return false;
    if(C_HNO3 < 0) return false;
    if(C_Np < 0)   return false;

    M_U    = C_U*SolVolume*M3ToL();
    M_Pu   = C_Pu*SolVolume*M3ToL();
    M_HNO3 = C_HNO3*SolVolume*M3ToL();
    M_Np   = C_Np*SolVolume*M3ToL();

    // Put your initialization here
    std::vector<std::string> dn;
    dn.push_back("Расход, мл/ч");
    dn.push_back("Температура, °С");
    dn.push_back("Концентрация U, г/л");
    dn.push_back("Концентрация Pu, г/л");
    dn.push_back("Концентрация HNO3, г/л");
    dn.push_back("Концентрация Np, г/л");
    PortFlowOut->setDataNames(dn);

    dn.clear();
    dn.push_back("Объем, м3");
    dn.push_back("Уровень, м");
    PortSensors->setDataNames(dn);

    return true;
}

bool MP_EX_TankPump::process(double t, double h, std::string &error)
{
    // Put your calculations here

    double dM_U = 0, dM_Pu = 0, dM_HNO3 = 0, dM_Np = 0;
    double Qin = 0, Qout = 0;

    // Расход на выходе
    if(PortFlowRate->getConnectedPort()) {
        Qout = PortFlowRate->getInput()[0];
        if(Qout < 0) Qout = 0;
    }

    // Поток на входе
    std::vector<double> solIn = PortFlowIn->getInput();
    if(solIn.empty()) {
        solIn.resize(6);
        for(size_t i = 0; i < solIn.size(); i++) solIn[i] = 0;
    }
    else {
        if(solIn.size() != 6) {
            error = "Несоответствие входного потока в емкость!";
            return false;
        }
    }

    if(isSituationActive("ES_DOSATION")) {
        double add = paramToDouble("FLOW_ES");

        Qout = Qout*(100.00 + add) / 100.00;
        if(Qout < 0) Qout = 0;
    }

    Qin = solIn[0];
    double Qin_s = Qin / HourToSec();
    double Qout_s = Qout / HourToSec();

    dM_U    = (solIn[2]*Qin_s - C_U*Qout_s)*h;
    dM_Pu   = (solIn[3]*Qin_s - C_Pu*Qout_s)*h;
    dM_HNO3 = (solIn[4]*Qin_s - C_HNO3*Qout_s)*h;
    dM_Np   = (solIn[5]*Qin_s - C_Np*Qout_s)*h;

    std::vector<double> flowOut;
    // sensors.resize(2);
    flowOut.resize(6);



    flowOut[0] = Qout;
    flowOut[1] = Temp;
    flowOut[2] = C_U;
    flowOut[3] = C_Pu;
    flowOut[4] = C_HNO3;
    flowOut[5] = C_Np;

    PortFlowOut->setNewOut(flowOut);

    SolVolume += lToM3()*(Qin - Qout)*h*secToHour();
    if(SolVolume < 0) {
        error = "Опустошение емкости!";
        return false;
    }

    M_U += dM_U;
    M_Pu += dM_Pu;
    M_HNO3 += dM_HNO3;
    M_Np += dM_Np;

    if(SolVolume == 0) {
        C_U = 0;
        C_Pu = 0;
        C_HNO3 = 0;
        C_Np = 0;
    }
    else {
        C_U = M_U/(SolVolume*M3ToL());
        C_Pu = M_Pu/(SolVolume*M3ToL());
        C_HNO3 = M_HNO3/(SolVolume*M3ToL());
        C_Np = M_Np/(SolVolume*M3ToL());
    }


    PortSensors->setNewOut(0, SolVolume);
    PortSensors->setNewOut(1, paramToDouble("TankHeight")*SolVolume/TankVolume);

    return true;
}

ICalcElement *Create()
{
    return new MP_EX_TankPump();
}

void Release(ICalcElement *block)
{
    if(block->type() == "MP_EX_TankPump") delete block;
}

std::string Type()
{
    return "MP_EX_TankPump";
}
