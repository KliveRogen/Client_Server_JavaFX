#include "MP_CR_PeristalticPump.h"

MP_CR_PeristalticPump::MP_CR_PeristalticPump()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
	createParameter("PumpFlow", "0");
    createParameter("DosationErrCoef", "10");

    createSituation("ES_DOSATION_ERR");
    createSituation("ES_DOSATION_ERR");

	// Сигналы блока

	// Порты блока
    PortInFlow = createInputPort(0, "Управление", "INFO");
    PortInLiq = createInputPort(1, "Материал", "INFO");
    PortOut = createOutputPort(2, "Раствор", "INFO");

	// Отказы блока

}

void MP_CR_PeristalticPump::setDataNames()
{
	std::vector<std::string> dn;
    dn.push_back("Расход, м3/с");
    dn.push_back("Температура, °C");
    dn.push_back("Концентрация урана, г/л");
    dn.push_back("Концентрация плутония, г/л");
    dn.push_back("Концентрация нептуния, г/л");
    dn.push_back("Концентрация азотной кислоты, г/л");
    PortOut->setDataNames(dn);
}

bool MP_CR_PeristalticPump::init(std::string &error, double h)
{
    // Put your initialization here
	setDataNames();

    PortOut->setOut(0,0);

    return true;
}

bool MP_CR_PeristalticPump::process(double t, double h, std::string &error)
{
    // Put your calculations here
    std::vector<double> VectInFlow, VectInLiq;

    VectInFlow = PortInFlow->getInput();
    VectInLiq = PortInLiq->getInput();

    double outFlow = 0;

    if (VectInFlow.size() != 0)
    {
        outFlow = VectInFlow[0];
    }
    else
    {
        outFlow = paramToDouble("PumpFlow");
    }

    int i = 0;

    double coeff = paramToDouble("DosationErrCoef"); // Читать с параметра

    if (isSituationActive("ES_DOSATION_ERR"))
    {
        outFlow = outFlow*(100.0 + coeff)/100.0;

        if (outFlow < 0) outFlow = 0;
    }

    PortOut->setOut(i, outFlow); i++;
    PortOut->setOut(i, VectInLiq[i-1]); i++;
    PortOut->setOut(i, VectInLiq[i-1]); i++;
    PortOut->setOut(i, VectInLiq[i-1]); i++;
    PortOut->setOut(i, VectInLiq[i-1]); i++;
    PortOut->setOut(i, VectInLiq[i-1]); i++;
    return true;
}


ICalcElement *Create()
{
    return new MP_CR_PeristalticPump();
}

void Release(ICalcElement *block)
{
    if(block->type() == "MP_CR_PeristalticPump") delete block;
}

std::string Type()
{
    return "MP_CR_PeristalticPump";
}
