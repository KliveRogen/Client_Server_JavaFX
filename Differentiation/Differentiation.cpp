#include "Differentiation.h"

Differentiation::Differentiation()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока

	// Сигналы блока

	// Порты блока
    InPort = createInputPort(0, "Исходный сигнал", "INFO");
    OutPort = createOutputPort(1, "Производная", "INFO");

    Prev = 0;

	// Отказы блока

}

bool Differentiation::process(double t, double h, std::string &error)
{
    // Put your calculations here
    std::vector<double> inVect = InPort->getInput();
    if(inVect.size() == 0) OutPort->setOut(0, 0);
    else {
        OutPort->setOut(0, (inVect[0] - Prev) / h);
        Prev = inVect[0];
    }

    return true;
}

bool Differentiation::init(std::string &error, double h)
{
    // Put your initialization here
    std::vector<std::string> dn;
    dn.push_back("Производная");
    OutPort->setDataNames(dn);

    return true;
}

ICalcElement *Create()
{
    return new Differentiation();
}

void Release(ICalcElement *block)
{
    if(block->type() == "Differentiation") delete block;
}

std::string Type()
{
    return "Differentiation";
}
