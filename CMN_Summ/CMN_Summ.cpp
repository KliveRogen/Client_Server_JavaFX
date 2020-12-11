#include "CMN_Summ.h"

CMN_Summ::CMN_Summ()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
	createParameter("FirstSign", "+");
	createParameter("SecondSign", "-");

	// Сигналы блока

	// Порты блока
    PortIn1 = createInputPort(0, "Первое", "INFO");
    PortIn2 = createInputPort(1, "Второе", "INFO");
    PortOut = createOutputPort(2, "Выход", "INFO");

	// Отказы блока
}

bool CMN_Summ::init(std::string &error, double h)
{
    // Put your initialization here
    std::vector<std::string> dn;
    dn.push_back("sum");
    PortOut->setDataNames(dn);
    return true;
}

bool CMN_Summ::process(double t, double h, std::string &error)
{
    // Put your calculations here
    double Out = 0;
    std::vector<double> VectIn1, VectIn2;
    VectIn1 = PortIn1->getInput();
    VectIn2 = PortIn2->getInput();

    if (getParameter("FirstSign").Value == "+")
    {
        if (VectIn1.size()!=0)
            Out += VectIn1[0];
    }
    else if (getParameter("FirstSign").Value == "-")
    {
        if (VectIn1.size()!=0)
            Out -= VectIn1[0];
    }

    if (getParameter("SecondSign").Value == "+")
    {
        if (VectIn2.size()!=0)
            Out += VectIn2[0];
    }
    else if (getParameter("SecondSign").Value == "-")
    {
        if (VectIn2.size()!=0)
            Out -= VectIn2[0];
    }

    PortOut->setOut(0, Out);

    return true;
}


ICalcElement *Create()
{
    return new CMN_Summ();
}

void Release(ICalcElement *block)
{
    if(block->type() == "CMN_Summ") delete block;
}

std::string Type()
{
    return "CMN_Summ";
}
