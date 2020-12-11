#include "HYDR_COLLECTOR.h"

HYDR_COLLECTOR::HYDR_COLLECTOR()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
	createParameter("coeff1", "1");
	createParameter("square1", "1");
	createParameter("coeff2", "1");
	createParameter("square2", "1");
	createParameter("coeff3", "1");
	createParameter("square3", "1");

	// Сигналы блока

	// Порты блока
    in = createInputPort(0, "UNKNOWN_NAME", "INFO");
    out1 = createOutputPort(1, "UNKNOWN_NAME", "INFO");
    out2 = createOutputPort(2, "UNKNOWN_NAME", "INFO");
    out3 = createOutputPort(3, "UNKNOWN_NAME", "INFO");

	// Отказы блока

}

void HYDR_COLLECTOR::setDataNames()
{
    std::vector<std::string> dn1,dn2,dn3;
    dn1.push_back("Расход, л/ч");
    dn1.push_back("Кин. вязкость, Па*с");
    dn1.push_back("Плотность, кг/м3");
    dn1.push_back("Давление, кПа");
    dn1.push_back("Концентрация, моль/л");
    out1->setDataNames(dn1);
    dn2.push_back("Расход, л/ч");
    dn2.push_back("Кин. вязкость, Па*с");
    dn2.push_back("Плотность, кг/м3");
    dn2.push_back("Давление, кПа");
    dn2.push_back("Концентрация, моль/л");
    out2->setDataNames(dn2);
    dn3.push_back("Расход, л/ч");
    dn3.push_back("Кин. вязкость, Па*с");
    dn3.push_back("Плотность, кг/м3");
    dn3.push_back("Давление, кПа");
    dn3.push_back("Концентрация, моль/л");
    out3->setDataNames(dn3);
}

bool HYDR_COLLECTOR::init(std::string &error, double h)
{
	setDataNames();
    return true;
}

bool HYDR_COLLECTOR::process(double t, double h, std::string &error)
{
    int numOuts;
    numOuts = 3;
    double  coeff[numOuts],resis[numOuts],flowIn,visc,dens,pressIn,
            concIn,totalResis,flowOut[numOuts],pressOut[numOuts];
    //получение параметров
    coeff[0] = paramToDouble("coeff1");
    coeff[1] = paramToDouble("coeff2");
    coeff[2] = paramToDouble("coeff3");
    resis[0] = paramToDouble("square1");
    resis[1] = paramToDouble("square2");
    resis[2] = paramToDouble("square3");
    //получение входов
    flowIn = in->getInput()[0];
    visc = in->getInput()[1];
    dens = in->getInput()[2];
    pressIn = in->getInput()[3];
    concIn = in->getInput()[4];
    //расчет
    totalResis = 0;
    for (int i = 0; i < numOuts; i++) {
        totalResis = totalResis + resis[i]/coeff[i];
    }
    for (int i = 0; i < numOuts; i++) {
        flowOut[i] = flowIn * resis[i] /coeff[i] /totalResis;
        pressOut[i] = pressIn - flowOut[i] * coeff[i] /resis[i];
    }
    // выходы
    out1->setOut(0,flowOut[0]);
    out1->setOut(1,visc);
    out1->setOut(2,dens);
    out1->setOut(3,pressOut[0]);
    out1->setOut(4,concIn);
    out2->setOut(0,flowOut[1]);
    out2->setOut(1,visc);
    out2->setOut(2,dens);
    out2->setOut(3,pressOut[1]);
    out2->setOut(4,concIn);
    out3->setOut(0,flowOut[2]);
    out3->setOut(1,visc);
    out3->setOut(2,dens);
    out3->setOut(3,pressOut[2]);
    out3->setOut(4,concIn);

    return true;
}


ICalcElement *Create()
{
    return new HYDR_COLLECTOR();
}

void Release(ICalcElement *block)
{
    if(block->type() == "HYDR_COLLECTOR") delete block;
}

std::string Type()
{
    return "HYDR_COLLECTOR";
}
