#include "MP_CR_InitTank.h"

MP_CR_InitTank::MP_CR_InitTank()
{
	// Расчетный тип блока
    BlockCalcType = E_OUTPUTONLY;

    // Параметры блока
	createParameter("Temperature", "50");
	createParameter("CU", "416");
	createParameter("CPu", "221");
	createParameter("CNp", "1");
	createParameter("CHNO3", "209");

	// Сигналы блока

	// Порты блока
    PortOut = createOutputPort(0, "Продукт", "INFO");

	// Отказы блока

}

void MP_CR_InitTank::setDataNames()
{
	std::vector<std::string> dn;
    dn.push_back("Температура раствора, °C");
    dn.push_back("Концентрация урана, г/л");
    dn.push_back("Концентрация плутония, г/л");
    dn.push_back("Концентрация нептуния, г/л");
    dn.push_back("Концентрация азотной кислоты, г/л");
    PortOut->setDataNames(dn);
}

bool MP_CR_InitTank::init(std::string &error, double h)
{
    // Put your initialization here

    setDataNames();

    int i = 0;
    std::vector<double> VectOut;
    VectOut.resize(5);
    VectOut[i++] = paramToDouble("Temperature");
    VectOut[i++] = paramToDouble("CU");
    VectOut[i++] = paramToDouble("CPu");
    VectOut[i++] = paramToDouble("CNp");
    VectOut[i++] = paramToDouble("CHNO3");
    PortOut->setOut(VectOut);

    return true;
}

bool MP_CR_InitTank::process(double t, double h, std::string &error)
{
    // Put your calculations here
    int i = 0;
    std::vector<double> VectOut;
    VectOut.resize(5);
    VectOut[i++] = paramToDouble("Temperature");
    VectOut[i++] = paramToDouble("CU");
    VectOut[i++] = paramToDouble("CPu");
    VectOut[i++] = paramToDouble("CNp");
    VectOut[i++] = paramToDouble("CHNO3");
    PortOut->setOut(VectOut);
    return true;
}


ICalcElement *Create()
{
    return new MP_CR_InitTank();
}

void Release(ICalcElement *block)
{
    if(block->type() == "MP_CR_InitTank") delete block;
}

std::string Type()
{
    return "MP_CR_InitTank";
}
