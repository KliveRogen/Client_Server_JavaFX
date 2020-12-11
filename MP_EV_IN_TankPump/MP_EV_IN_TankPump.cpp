#include "MP_EV_IN_TankPump.h"

MP_EV_IN_TankPump::MP_EV_IN_TankPump()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
	createParameter("Temperature", "40");
    createParameter("C_U", "25");
	createParameter("C_HNO3", "0.05");

	// Сигналы блока
	createSignal("FLOWRATE", Signal::ST_DOUBLE);

	// Порты блока
    PortIn = createInputPort(0, "Уставка расхода", "INFO");
    PortOut = createOutputPort(1, "Подача раствора", "INFO");

	// Отказы блока

}

bool MP_EV_IN_TankPump::init(std::string &error, double h)
{
    // Put your initialization here
    std::vector<std::string> dn;
    dn.push_back("Расход");
    dn.push_back("Температура");
    dn.push_back("Концентрация U");
    dn.push_back("Концентрация HNO3");
    PortOut->setDataNames(dn);
    return true;
}

bool MP_EV_IN_TankPump::process(double t, double h, std::string &error)
{
    // Put your calculations here
    std::vector<double> OutVect, InVect;
    OutVect.resize(4);
    InVect = PortIn->getInput();

    OutVect[0] = 0;
    if (InVect.size() != 0 )
    {
        OutVect[0] = InVect[0];
    }
    OutVect[1] = paramToDouble(getParameter("Temperature"));
    OutVect[2] = paramToDouble(getParameter("C_U"));
    OutVect[3] = paramToDouble(getParameter("C_HNO3"));

    PortOut->setOut(OutVect);
    return true;
}

ICalcElement *Create()
{
    return new MP_EV_IN_TankPump();
}

void Release(ICalcElement *block)
{
    if(block->type() == "MP_EV_IN_TankPump") delete block;
}

std::string Type()
{
    return "MP_EV_IN_TankPump";
}
