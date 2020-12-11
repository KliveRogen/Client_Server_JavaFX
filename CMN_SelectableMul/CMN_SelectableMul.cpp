#include "CMN_SelectableMul.h"

#include <QString>
#include <QStringList>

CMN_SelectableMul::CMN_SelectableMul()
{
    // Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
    createParameter("SelectedString1", "");
    createParameter("SelectedString2", "");

    // Сигналы блока

    // Порты блока
    PortIn1 = createInputPort(0, "Вход 1", "INFO");
    PortIn2 = createInputPort(1, "Вход 2", "INFO");
    PortOut = createOutputPort(2, "Выход", "INFO");

    // Отказы блока

}

bool CMN_SelectableMul::init(std::string &error, double h)
{
    std::vector<std::string> dn;
    IOutputPort *port1 = PortIn1->getConnectedPort();
    IOutputPort *port2 = PortIn2->getConnectedPort();

    QStringList paramsList1 = QString::fromStdString(getParameter("SelectedString1").Value).split(" ");
    paramsList1.removeAll("");
    Size1 = paramsList1.size();

    QStringList paramsList2 = QString::fromStdString(getParameter("SelectedString2").Value).split(" ");
    paramsList2.removeAll("");
    Size2 = paramsList2.size();

    std::vector<std::string> names1 = port1->getDataNames();
    std::vector<std::string> names2 = port2->getDataNames();

    // TODO: Тут должна быть проверка
    /*
    for (int i = 0; i < paramsList1.size(); i++)
    {
        if(paramsList1[i].toInt() > names1.size())
        {
            error = "Значение выше чем возможно";
            return false;
        }
    }

    for (int i = 0; i < paramsList2.size(); i++)
    {
        if(paramsList2[i].toInt() > names2.size())
        {
            error = "Значение выше чем возможно";
            return false;
        }
    }
    */

    if (names1.size() == 0 && names2.size() == 0)
    {
        dn.push_back("Селектор(не подключен)");
        PortOut->setDataNames(dn);
        return true;
    }

    if(Size1 == 0) // для пустой строки переносим все имена
    {
        for (int i = 0; i < names1.size(); i++)
        {
            QString name = QString::fromStdString(names1[i]);
            name.append(" (C)");
            dn.push_back(name.toStdString());
        }
    }
    else
    {
        for (int i = 0; i < Size1; i++)
        {
            QString name = QString::fromStdString(names1[paramsList1[i].toInt()]);
            name.append(" (C)");
            dn.push_back(name.toStdString());
        }
    }

    if(Size2 == 0) // для пустой строки переносим все имена
    {
        for (int i = 0; i < names2.size(); i++)
        {
            QString name = QString::fromStdString(names2[i]);
            name.append(" (C)");
            dn.push_back(name.toStdString());
        }
    }
    else
    {
        for (int i = 0; i < Size2; i++)
        {
            QString name = QString::fromStdString(names2[paramsList2[i].toInt()]);
            name.append(" (C)");
            dn.push_back(name.toStdString());
        }
    }
    PortOut->setDataNames(dn);
    return true;
}

bool CMN_SelectableMul::process(double t, double h, std::string &error)
{

    QStringList paramsList1 = QString::fromStdString(getParameter("SelectedString1").Value).split(" ");
    QStringList paramsList2 = QString::fromStdString(getParameter("SelectedString2").Value).split(" ");
    paramsList1.removeAll("");
    paramsList2.removeAll("");
    IOutputPort *port1 = PortIn1->getConnectedPort();
    IOutputPort *port2 = PortIn2->getConnectedPort();

    std::vector<std::string> names1 = port1->getDataNames();
    std::vector<std::string> names2 = port2->getDataNames();

    for (int i = 0; i < paramsList1.size(); i++)
    {
        if(paramsList1[i].toInt() > names1.size())
        {
            error = "Значение выше чем возможно";
            return false;
        }
    }

    for (int i = 0; i < paramsList2.size(); i++)
    {
        if(paramsList2[i].toInt() > names2.size())
        {
            error = "Значение выше чем возможно";
            return false;
        }
    }

    std::vector<double> OutVect;

    if (paramsList1.size() != Size1 || paramsList2.size() != Size2)
    {
        error = "Запрещено изменение размера вектора в процессе расчета!";
        return false;
    }

    std::vector<double> inVect1 = PortIn1->getInput();
    std::vector<double> inVect2 = PortIn2->getInput();

    if (inVect1.size() == 0 && inVect2.size() == 0)
    {
        OutVect.push_back(0);
    }
    else
    {
        if (inVect1.size() != 0)
        {
            if (Size1 == 0)
            {
                for (int i = 0; i < inVect1.size(); i++)
                {
                    OutVect.push_back(inVect1.at(i));
                }
            }
            else
            {
                for (int i = 0; i < Size1; i++)
                {
                    OutVect.push_back(inVect1[paramsList1[i].toInt()]);
                }
            }
        }
        if (inVect2.size() != 0)
        {
            if (Size2 == 0)
            {
                for (int i = 0; i < inVect2.size(); i++)
                {
                    OutVect.push_back(inVect2.at(i));
                }
            }
            else
            {
                for (int i = 0; i < Size2; i++)
                {
                    OutVect.push_back(inVect2[paramsList2[i].toInt()]);
                }
            }
        }
    }
    PortOut->setOut(OutVect);
    return true;
}


ICalcElement *Create()
{
    return new CMN_SelectableMul();
}

void Release(ICalcElement *block)
{
    if(block->type() == "CMN_SelectableMul") delete block;
}

std::string Type()
{
    return "CMN_SelectableMul";
}
