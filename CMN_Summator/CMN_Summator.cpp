#include "CMN_Summator.h"

#include <QString>
#include <QDomElement>

CMN_Summator::CMN_Summator()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
    createParameter("SUM_SIGNS", "++");

	// Сигналы блока

    // Порты блока
    PortOut = createOutputPort(0, "Выход", "INFO");

	// Отказы блока
}

bool CMN_Summator::init(std::string &error, double h)
{
    // Put your initialization here
    std::vector<std::string> dn;
    dn.push_back("Сумма");
    PortOut->setDataNames(dn);
    return true;
}

void CMN_Summator::loadParameters(QDomElement &domParams)
{
    CalcElement::loadParameters(domParams);

    Signs = QString::fromStdString(getParameter("SUM_SIGNS").Value);
    for(int i = 0; i < Signs.size(); i++) {
        if(Signs.at(i) == '+' || Signs.at(i) == '-') {
            InPorts.append(createInputPort(i+1, "Вход", "INFO"));
        }
        else return;
    }
}

bool CMN_Summator::process(double t, double h, std::string &error)
{
    // Put your calculations here
    double Out = 0;
    for(int i = 0; i < InPorts.size(); i++) {
        std::vector<double> inVect = InPorts[i]->getInput();
        if(!inVect.empty()) {
            if(Signs.at(i) == '+') Out += inVect[0];
            else if(Signs.at(i) == '-') Out -= inVect[0];
            else {
                error = "Ошибка в знаке сумматора!";
                return false;
            }
        }
    }

    PortOut->setOut(0, Out);

    return true;
}


ICalcElement *Create()
{
    return new CMN_Summator();
}

void Release(ICalcElement *block)
{
    if(block->type() == "Summator") delete block;
}

std::string Type()
{
    return "Summator";
}
