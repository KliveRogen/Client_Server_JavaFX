#include "Step.h"

Step::Step()
{
	// Расчетный тип блока
    BlockCalcType = E_OUTPUTONLY;

    // Параметры блока
    createParameter("START_VALUE", "0");
    createParameter("FINISH_VALUE", "1");
    createParameter("TIME", "1");

	// Сигналы блока

    // Порты блока
    OutPort = createOutputPort(0, "Выход", "INFO");

	// Отказы блока

}

bool Step::process(double t, double h, std::string &error)
{
    // Put your calculations here
    StartValue = paramToDouble(Parameters[0]);
    FinishValue = paramToDouble(Parameters[1]);
    StepTime = paramToDouble(Parameters[2]);

    double out;
    if(StepTime <= t) out = FinishValue;
    else out = StartValue;

    OutPort->setOut(0, out);
    return true;
}

bool Step::init(std::string &error, double h)
{
    // Put your initialization here
    std::vector<std::string> dn;
    dn.push_back("Уставка");
    OutPort->setDataNames(dn);

    return true;
}

ICalcElement *Create()
{
    return new Step();
}

void Release(ICalcElement *block)
{
    if(block->type() == "Step") delete block;
}

std::string Type()
{
    return "Step";
}
