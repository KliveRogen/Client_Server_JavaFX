#include "Amplifier.h"

Amplifier::Amplifier()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
	createParameter("GAIN", "1");
    Gain = 1.00;

	// Сигналы блока

	// Порты блока
    InPort = createInputPort(0, "Вход усилителя", "INFO");
    OutPort = createOutputPort(1, "Выход усилителя", "INFO");

	// Отказы блока

}

bool Amplifier::init(std::string &error, double h)
{
    // Put your initialization here
    std::vector<std::string> dn;
    IOutputPort *port = InPort->getConnectedPort();

    if (port)
    {
        dn = port->getDataNames();
        for (int i = 0; i < dn.size(); i++)
        {
            dn.at(i).insert(0, "Усиленное ");
        }
    }
    else
    {
        dn.push_back("Усиленное значение");
    }
    OutPort->setDataNames(dn);
    return true;
}

bool Amplifier::process(double t, double h, std::string &error)
{
    // Put your calculations here
    Gain = paramToDouble(Parameters[0]);

    std::vector<double> inVect = InPort->getInput();
    if(inVect.size() == 0) OutPort->setOut(0, 0);
    else
    {
        for (int i = 0; i < inVect.size(); i++)
        {
            OutPort->setOut(i, inVect[i] * Gain);
        }
    }
    return true;
}

ICalcElement *Create()
{
    return new Amplifier();
}

void Release(ICalcElement *block)
{
    if(block->type() == "Amplifier") delete block;
}

std::string Type()
{
    return "Amplifier";
}
