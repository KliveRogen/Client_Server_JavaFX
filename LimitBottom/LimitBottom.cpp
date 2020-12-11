#include "LimitBottom.h"

LimitBottom::LimitBottom()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
	createParameter("BOTTOM", "0");

	// Сигналы блока

	// Порты блока
    InPort = createInputPort(0, "Вход", "INFO");
    OutPort = createOutputPort(1, "Выход", "INFO");

	// Отказы блока

}

bool LimitBottom::process(double t, double h, std::string &error)
{
    // Put your calculations here
    std::vector<double> inVect = InPort->getInput();
    if(inVect.size() == 0) {
        OutPort->setOut(0, 0);
        return true;
    }

    double in = inVect[0];
    double out;
    double limit = paramToDouble(Parameters[0]);

    if(in <= limit) out = limit;
    else out = in;

    OutPort->setOut(0, out);
    return true;
}

bool LimitBottom::init(std::string &error, double h)
{
    // Put your initialization here
    std::vector<std::string> dn;
    dn.push_back("Значение");
    OutPort->setDataNames(dn);

    return true;
}

ICalcElement *Create()
{
    return new LimitBottom();
}

void Release(ICalcElement *block)
{
    if(block->type() == "LimitBottom") delete block;
}

std::string Type()
{
    return "LimitBottom";
}
