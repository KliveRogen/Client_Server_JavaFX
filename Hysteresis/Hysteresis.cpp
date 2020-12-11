#include "Hysteresis.h"

Hysteresis::Hysteresis()
{
    // Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
    createParameter("down", "0");
    createParameter("up", "1");
    createParameter("left", "-1");
    createParameter("right", "1");

    // Сигналы блока

    // Порты блока
    PortIn = createInputPort(0, "Вход", "INFO");
    PortOut = createOutputPort(1, "Выход", "INFO");

    // Отказы блока

}

bool Hysteresis::init(std::string &error, double h)
{
    std::vector<std::string> dn;
    IOutputPort *port = PortIn->getConnectedPort();
    if (port)
    {
        dn.push_back("Гистерезис");
    }
    else
    {
        dn.push_back("Гистерезис(не подключен)");
    }
    Down = paramToDouble(Parameters[0]);
    Out = Down;
    PortOut->setDataNames(dn);
    return true;
}

bool Hysteresis::process(double t, double h, std::string &error)
{
    // Put your calculations here
    Down = paramToDouble(Parameters[0]);
    Up = paramToDouble(Parameters[1]);
    Left = paramToDouble(Parameters[2]);
    Right = paramToDouble(Parameters[3]);

    double in;
    std::vector<double> inVect = PortIn->getInput();

    if (inVect.size()==0)
    {
        Out = 0;
    }
    else
    {
        in = inVect[0]; // Независимо от размера подключенного вектора, работа идет по нулевому
        if (in < Left)
        {
            Out = Down;
        }
        else if (in > Right)
        {
            Out = Up;
        }
        else if ( in>Left && in<Right && Out == Down)
        {
            Out = Down;
        }
        else if ( in>Left && in<Right && Out == Up)
        {
            Out = Up;
        }
    }

    PortOut->setOut(0, Out);

    return true;
}


ICalcElement *Create()
{
    return new Hysteresis();
}

void Release(ICalcElement *block)
{
    if(block->type() == "Hysteresis") delete block;
}

std::string Type()
{
    return "Hysteresis";
}
