#include "HYDR_LIQUIDVALVE.h"

HYDR_LIQUIDVALVE::HYDR_LIQUIDVALVE()
{
	// Расчетный тип блока
    BlockCalcType = E_OUTPUTONLY;

    // Параметры блока
    createParameter("press1", "0");
    createParameter("press2", "0");
    createParameter("ro", "1000");

	// Сигналы блока

	// Порты блока
    out = createOutputPort(0, "out", "INFO");

	// Отказы блока

}

void HYDR_LIQUIDVALVE::setDataNames()
{
	std::vector<std::string> dn;
    dn.push_back("Давление 1, Па");
    dn.push_back("Давление 2, Па");
    dn.push_back("Плотность, кг/м3");
    out->setDataNames(dn);
}

bool HYDR_LIQUIDVALVE::init(std::string &error, double h)
{
	setDataNames();
    return true;
}

bool HYDR_LIQUIDVALVE::process(double t, double h, std::string &error)
{
    int i=0;
    out->setOut(i,paramToDouble("press1")); i++;
    out->setOut(i,paramToDouble("press2")); i++;
    out->setOut(i,paramToDouble("ro")); i++;
    return true;
}


ICalcElement *Create()
{
    return new HYDR_LIQUIDVALVE();
}

void Release(ICalcElement *block)
{
    if(block->type() == "HYDR_LIQUIDVALVE") delete block;
}

std::string Type()
{
    return "HYDR_LIQUIDVALVE";
}
