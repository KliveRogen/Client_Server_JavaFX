#include "InertionOne.h"

InertionOne::InertionOne()
{
	// Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
	createParameter("GAIN", "1");
	createParameter("INERTION", "1");

	// Сигналы блока

	// Порты блока
    PortIn = createInputPort(0, "Вход ИЗ", "INFO");
    PortOut = createOutputPort(1, "Выход ИЗ", "INFO");

	// Отказы блока

}

bool InertionOne::process(double t, double h, std::string &error)
{
    K = paramToDouble(Parameters[0]);
    T = paramToDouble(Parameters[1]);

    if(T == 0) {
        error = "Inertion is equal to zero";
        return false;
    }

    std::vector<double> in = PortIn->getInput();
    if(in.size() != 0) {
        double dout = (K*in[0] - Out) / T;
        Out += dout*h;
    }

    PortOut->setNewOut(0, Out);

    return true;
}

bool InertionOne::init(std::string &error, double h)
{
    // Put your initialization here
    Out = 0;

    std::vector<std::string> dn;
    dn.push_back("Выход ИЗ");
    PortOut->setDataNames(dn);
    PortOut->setOut(0, Out);

    return true;
}

ICalcElement *Create()
{
    return new InertionOne();
}

void Release(ICalcElement *block)
{
    if(block->type() == "InertionOne") delete block;
}

std::string Type()
{
    return "InertionOne";
}
