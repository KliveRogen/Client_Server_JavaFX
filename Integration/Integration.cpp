#include "Integration.h"

Integration::Integration()
{
	// Расчетный тип блока
    //BlockCalcType = E_INITVALUES;
    BlockCalcType = E_OUTPUTONLY;

    // Параметры блока
	createParameter("K", "1");
	createParameter("T", "1");
    createParameter("V", "0");

	// Сигналы блока

	// Порты блока
    InPort = createInputPort(0, "Вход", "INFO");
    OutPort = createOutputPort(1, "Выход", "INFO");

	// Отказы блока

}

bool Integration::process(double t, double h, std::string &error)
{
    // Put your calculations here
    K = paramToDouble(Parameters[0]);
    T = paramToDouble(Parameters[1]);

    std::vector<double> inVect = InPort->getInput();
    if(inVect.size() == 0) {
        OutPort->setOut(0, V);
        //OutPort->setNewOut(0, V);
        return true;
    }
    else {
        if(T == 0) {
            error = "Деление на ноль!";
            return false;
        }

        V += inVect[0]*h*K/T;
        OutPort->setOut(0, V);
        //OutPort->setNewOut(0, V);
        return true;
    }
}

bool Integration::init(std::string &error, double h)
{
    // Put your initialization here
    std::vector<std::string> dn;
    dn.push_back("Интеграл");
    OutPort->setDataNames(dn);

    V = paramToDouble(Parameters[2]);
    OutPort->setOut(0, V);

    return true;
}

ICalcElement *Create()
{
    return new Integration();
}

void Release(ICalcElement *block)
{
    if(block->type() == "Integration") delete block;
}

std::string Type()
{
    return "Integration";
}
