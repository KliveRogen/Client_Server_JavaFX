#include "CMN_PIDController.h"

CMN_PIDController::CMN_PIDController()
{
	// Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
	createParameter("P", "1");
	createParameter("I", "1");
	createParameter("D", "1");

	// Сигналы блока

	// Порты блока
    PortIn = createInputPort(0, "Вход", "INFO");
    PortOut = createOutputPort(1, "Выход", "INFO");
	// Отказы блока

}

bool CMN_PIDController::init(std::string &error, double h)
{
    std::vector<std::string> dn;

    dn.push_back("Выход регулятора");
    PortOut->setDataNames(dn);

    e_1 = 0;
    e_2 = 0;

    PortOut->setOut(0,0);

    return true;
}

bool CMN_PIDController::process(double t, double h, std::string &error)
{
    P = paramToDouble("P");
    I = paramToDouble("I");
    D = paramToDouble("D");

    double du, q0, q1, q2, e_0;

    std::vector<double> in = PortIn->getInput();

    if (in.size() == 0)
    {
        e_0 = 0;
    }
    else
    {
        e_0 = in[0];
    }

    q0 = P * (1 + (h / (2 * I)) + D / h);
    q1 = P * ((h / (2 * I)) - 1 - 2 * (D / h));
    q2 = P * D / h;

    du = q0 * e_0 + q1 * e_1 + q2 * e_2;

    e_2 = e_1;
    e_1 = e_0;

    PortOut->setNewOut(0, PortOut->getOut().at(0) + du);
    return true;
}

ICalcElement *Create()
{
    return new CMN_PIDController();
}

void Release(ICalcElement *block)
{
    if(block->type() == "CMN_PIDController") delete block;
}

std::string Type()
{
    return "CMN_PIDController";
}
