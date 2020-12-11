#include "Multiconst.h"

#include <QString>
#include <QStringList>

Multiconst::Multiconst()
{
	// Расчетный тип блока
    BlockCalcType = E_OUTPUTONLY;

    // Параметры блока
	createParameter("VALUE", "0");

	// Сигналы блока

	// Порты блока
    OutPort = createOutputPort(0, "Выход", "INFO");

	// Отказы блока

}

bool Multiconst::process(double t, double h, std::string &error)
{
    // Put your calculations here
    QStringList values = QString::fromStdString(Parameters[0].Value).split(" ");
    if(values.size() != Size) {
        error = "Запрещено изменение размера вектора в процессе расчета!";
        return false;
    }

    std::vector<double> vect;
    vect.resize(Size);
    for(int i = 0; i < values.size(); i++) vect[i] = values[i].toDouble();

    OutPort->setOut(vect);
    return true;
}

bool Multiconst::init(std::string &error, double h)
{
    // Put your initialization here
    QStringList values = QString::fromStdString(Parameters[0].Value).split(" ");
    Size = values.size();

    std::vector<std::string> dn;
    dn.resize(Size);
    for (int i = 0; i < Size; i++)
    {
        QString str = QString("%1: %2").arg(QString::fromStdString(getName())).arg(i);
        dn.at(i) = str.toStdString();
    }
    OutPort->setDataNames(dn);
    return true;
}

ICalcElement *Create()
{
    return new Multiconst();
}

void Release(ICalcElement *block)
{
    if(block->type() == "Multiconst") delete block;
}

std::string Type()
{
    return "Multiconst";
}
