#include "CMN_VectorSelector.h"

#include <QString>
#include <QStringList>

CMN_VectorSelector::CMN_VectorSelector()
{
    // Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
    createParameter("ParamsString", "");

    // Сигналы блока

    // Порты блока
    PortIn = createInputPort(0, "Вход", "INFO");
    PortOut = createOutputPort(1, "Выход", "INFO");

    // Отказы блока

}

bool CMN_VectorSelector::init(std::string &error, double h)
{
    // Put your initialization here
    std::vector<std::string> dn;
    IOutputPort *port = PortIn->getConnectedPort();
    std::vector<std::string> names = port->getDataNames();

    QStringList paramsList = QString::fromStdString(getParameter("ParamsString").Value).split(" ");
    paramsList.removeAll("");
    Size = paramsList.size();

    // TODO: перенести проверку из process обратно сюда
    /*
    for (int i = 0; i < paramsList.size(); i++)
    {
        if(paramsList[i].toInt() > names.size())
        {
            error = "Значение выше чем возможно";
            return false;
        }
    }
    */

    if (PortIn->getInput().size() == 0)
        dn.push_back("Селектор(не подключен)");
    else
    {
        if(Size == 0) // для пустой строки переносим все имена
        {
            for (int i = 0; i < names.size(); i++)
            {
                QString name = QString::fromStdString(names[i]);
                name.append(" (C)");
                dn.push_back(name.toStdString());
            }
        }
        else
        {
            for (int i = 0; i < Size; i++)
            {
                QString name = QString::fromStdString(names[paramsList[i].toInt()]);
                name.append(" (C)");
                dn.push_back(name.toStdString());
            }
        }
    }
    PortOut->setDataNames(dn);
    return true;
}

bool CMN_VectorSelector::process(double t, double h, std::string &error)
{
    QStringList paramsList = QString::fromStdString(getParameter("ParamsString").Value).split(" ");
    paramsList.removeAll("");
    std::vector<double> OutVect;
    if (paramsList.size() != Size)
    {
        error = "Запрещено изменение размера вектора в процессе расчета!";
        return false;
    }

    IOutputPort *port = PortIn->getConnectedPort();
    std::vector<std::string> names = port->getDataNames();
    for (int i = 0; i < paramsList.size(); i++)
    {
        if(paramsList[i].toInt() > names.size())
        {
            error = "Значение выше чем возможно";
            return false;
        }
    }

    std::vector<double> inVect = PortIn->getInput();
    if (inVect.size() == 0)
    {
        OutVect.push_back(0);
    }
    else
    {
        if (Size == 0)
        {
            OutVect = inVect;
        }
        else
        {
            for (int i = 0; i < Size; i++)
            {
                OutVect.push_back(inVect[paramsList[i].toInt()]);
            }
        }
    }
    PortOut->setOut(OutVect);
    return true;
}


ICalcElement *Create()
{
    return new CMN_VectorSelector();
}

void Release(ICalcElement *block)
{
    if(block->type() == "CMN_VectorSelector") delete block;
}

std::string Type()
{
    return "CMN_VectorSelector";
}
