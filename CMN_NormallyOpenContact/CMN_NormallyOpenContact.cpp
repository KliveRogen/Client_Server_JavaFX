#include "CMN_NormallyOpenContact.h"

// NOTE:
// Был разработан для реализации выпарного аппарата
// Работает только с нулевой переменной в каждом векторе

CMN_NormallyOpenContact::CMN_NormallyOpenContact()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
	createParameter("Threshold", "0");

	// Сигналы блока

    // Порты блока
    PortControl = createInputPort(0, "Вход управления", "INFO");
    PortOut = createOutputPort(1, "Выход", "INFO");
    PortIn1 = createInputPort(2, "Вход 1", "INFO");
    PortIn2 = createInputPort(3, "Вход 2", "INFO");

	// Отказы блока

}

bool CMN_NormallyOpenContact::init(std::string &error, double h)
{
    // Put your initialization here

    std::vector<std::string> dn;
    dn.push_back("Нормально разомкнутый ключ");
    PortOut->setDataNames(dn);
    return true;
}

bool CMN_NormallyOpenContact::process(double t, double h, std::string &error)
{
    Treshold = paramToDouble("Threshold");

    std::vector<double> vectControl, vectIn1, vectIn2, vectOut;

    vectControl = PortControl->getInput();
    vectIn1 = PortIn1->getInput();
    vectIn2 = PortIn2->getInput();

    vectOut.resize(1);
    vectOut[0] = 0; // Значение по умолчанию, используется при неподключенном порте.
                    // Может выводить сразу NaN, чтобы было видно?
    vectControl = PortControl->getInput();
    if (vectControl.size() != 0)
    {
        if (vectControl[0] > Treshold)
        {
            if (vectIn1.size() > 0)
            {
                vectOut[0] = vectIn1[0];
            }
        }
        else
        {
            if (vectIn2.size() > 0)
            {
                vectOut[0] = vectIn2[0];
            }
        }
    }
    PortOut->setOut(vectOut);
    return true;
}

ICalcElement *Create()
{
    return new CMN_NormallyOpenContact();
}

void Release(ICalcElement *block)
{
    if(block->type() == "CMN_NormallyOpenContact") delete block;
}

std::string Type()
{
    return "CMN_NormallyOpenContact";
}
