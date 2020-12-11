#include "LimitTopBottom.h"

LimitTopBottom::LimitTopBottom()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
	createParameter("TOP", "1");
	createParameter("BOTTOM", "-1");

	// Сигналы блока

	// Порты блока
    InPort = createInputPort(0, "Вход", "INFO");
    OutPort = createOutputPort(1, "Выход", "INFO");

	// Отказы блока

}

bool LimitTopBottom::process(double t, double h, std::string &error)
{
    // Put your calculations here
    std::vector<double> inVect = InPort->getInput();
    if(inVect.size() == 0) {
        OutPort->setOut(0, 0);
        return true;
    }

    double limitTop = paramToDouble(Parameters[0]);
    double limitBot = paramToDouble(Parameters[1]);

    if(limitTop <= limitBot) {
        error = "Некорректные значения верхнего и нижнего пределов!";
        return false;
    }

    double in = inVect[0];
    double out;
    if(in >= limitTop) out = limitTop;
    else if(in <= limitBot) out = limitBot;
    else out = in;

    OutPort->setOut(0, out);
    return true;
}

bool LimitTopBottom::init(std::string &error, double h)
{
    // Put your initialization here
    std::vector<std::string> dn;
    dn.push_back("Значение");
    OutPort->setDataNames(dn);

    return true;
}

ICalcElement *Create()
{
    return new LimitTopBottom();
}

void Release(ICalcElement *block)
{
    if(block->type() == "LimitTopBottom") delete block;
}

std::string Type()
{
    return "LimitTopBottom";
}
