#include "HYDR_LIQUID.h"

HYDR_LIQUID::HYDR_LIQUID()
{
	// Расчетный тип блока
    BlockCalcType = E_OUTPUTONLY;

    // Параметры блока
	createParameter("rate", "0.5");
	createParameter("viscosity", "0.001004");
	createParameter("density", "1000");
	createParameter("pressure", "200");
	createParameter("concentration", "0");

	// Сигналы блока

	// Порты блока
    out = createOutputPort(0, "out", "INFO");

	// Отказы блока

}

void HYDR_LIQUID::setDataNames()
{
	std::vector<std::string> dn;
    dn.push_back("Расход, л/ч");
    dn.push_back("Кин. вязкость, Па*с");
    dn.push_back("Плотность, кг/м3");
    dn.push_back("Давление, кПа");
    dn.push_back("Концентрация, моль/л");
    out->setDataNames(dn);
}

bool HYDR_LIQUID::init(std::string &error, double h)
{

	setDataNames();
    /*
    int i=0;
    out->setOut(i,paramToDouble("rate")); i++;
    out->setOut(i,paramToDouble("viscosity")); i++;
    out->setOut(i,paramToDouble("density")); i++;
    out->setOut(i,paramToDouble("pressure")); i++;
    out->setOut(i,paramToDouble("concentration")); i++;
    */
    return true;
}

bool HYDR_LIQUID::process(double t, double h, std::string &error)
{
    int i=0;
    out->setOut(i,paramToDouble("rate")); i++;
    out->setOut(i,paramToDouble("viscosity")); i++;
    out->setOut(i,paramToDouble("density")); i++;
    out->setOut(i,paramToDouble("pressure")); i++;
    out->setOut(i,paramToDouble("concentration")); i++;
    return true;
}


ICalcElement *Create()
{
    return new HYDR_LIQUID();
}

void Release(ICalcElement *block)
{
    if(block->type() == "HYDR_LIQUID") delete block;
}

std::string Type()
{
    return "HYDR_LIQUID";
}
