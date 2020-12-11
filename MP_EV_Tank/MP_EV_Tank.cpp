#include "MP_EV_Tank.h"

MP_EV_Tank::MP_EV_Tank()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
    createParameter("MaxVolume", "100");
	createParameter("HighLevelPercentage", "95");
	createParameter("LowLevelPercentage", "5");

	// Сигналы блока
	createSignal("HighLevelAlarm", Signal::ST_BOOL);
	createSignal("LowLevelAlarm", Signal::ST_BOOL);
	createSignal("Level", Signal::ST_DOUBLE);

	// Порты блока
    PortIn = createInputPort(0, "Раствор", "INFO");
    PortOut = createOutputPort(1, "Датчики", "INFO");

	// Отказы блока

}

bool MP_EV_Tank::init(std::string &error, double h)
{
    // Put your initialization here

    Volume = 0;
    CU = 0;
    CHNO3 = 0;
    Temperature = 0;

    std::vector<std::string> dn;
    dn.push_back("Объем жидкости в емкости");
    dn.push_back("Температура");
    dn.push_back("Концентрация U");
    dn.push_back("Концентрация HNO3");
    PortOut->setDataNames(dn);
    return true;
}

bool MP_EV_Tank::process(double t, double h, std::string &error)
{
    // Put your calculations here
    double maxVolume = paramToDouble(getParameter("MaxVolume"));
    std::vector<double> VectIn, VectOut;
    VectOut.resize(4);
    VectIn = PortIn->getInput();

    if (VectIn.size() == 4)
    {
        double inFlow = VectIn[0];
        double inTemp = VectIn[1];
        double inCU = VectIn[2];
        double inCHNO3 = VectIn[3];

        if (inFlow > 0)
        {
            double inVol = inFlow*h;
            Temperature = (Temperature*Volume + inTemp*inVol)/(Volume+inVol);
            CHNO3 = (CHNO3*Volume + inCHNO3*inVol)/(Volume+inVol);
            CU = (CU*Volume + inCU*inVol)/(Volume+inVol);
            Volume += inVol;
        }
        if (Volume > maxVolume)
        {
            error = "Емкость переполнена!";
            return false;
        }
    }

    // TODO: Обработать сигналы

    VectOut[0] = Volume;
    VectOut[1] = Temperature;
    VectOut[2] = CU;
    VectOut[3] = CHNO3;

    PortOut->setOut(VectOut);
    return true;
}

ICalcElement *Create()
{
    return new MP_EV_Tank();
}

void Release(ICalcElement *block)
{
    if(block->type() == "MP_EV_Tank") delete block;
}

std::string Type()
{
    return "MP_EV_Tank";
}
