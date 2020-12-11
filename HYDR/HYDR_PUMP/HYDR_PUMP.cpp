#include "HYDR_PUMP.h"

HYDR_PUMP::HYDR_PUMP()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
	createParameter("rate", "1");
	createParameter("inertia", "10");

	// Сигналы блока

	// Порты блока
    in = createInputPort(0, "in", "INFO");
    ctrl = createInputPort(1, "out", "INFO");
    out = createOutputPort(2, "ctrl", "INFO");

	// Отказы блока

}

void HYDR_PUMP::setDataNames()
{
	std::vector<std::string> dn;
    dn.push_back("Расход, л/ч");
    dn.push_back("Кин. вязкость, Па*с");
    dn.push_back("Плотность, кг/м3");
    dn.push_back("Давление, кПа");
    dn.push_back("Концентрация, моль/л");
    out->setDataNames(dn);
}

bool HYDR_PUMP::init(std::string &error, double h)
{
    flowRateCalc = 0;
    flowRateCurrent = 0;
	setDataNames();
    return true;
}

bool HYDR_PUMP::process(double t, double h, std::string &error)
{
    double flowRate,inert;
    //получение параметров
    flowRate = paramToDouble("rate");
    inert = paramToDouble("inertia");
    //проверка вводимых данных
    if (inert < 0 || flowRate < 0) {
        error = "Ошибка настройки параметров блока (Насосная станция)";
        return false;
    }
    //проверка наличия управляющего сигнала
    if ( ctrl->getInput()[0] <= 0 ) {
        flowRate = 0;
    }
    //расчет расхода //не знаю смысл двух инерционностей, так было в оригинальном коде
    flowRateCalc += h * (flowRate - flowRateCalc) / inert;
    flowRateCurrent += h * (flowRateCalc - flowRateCurrent) / inert;
    //вывод
    out->setOut(0,flowRateCurrent);
    out->setOut(1,in->getInput()[1]);
    out->setOut(2,in->getInput()[2]);
    out->setOut(3,in->getInput()[3]);
    out->setOut(4,in->getInput()[4]);
    return true;
}


ICalcElement *Create()
{
    return new HYDR_PUMP();
}

void Release(ICalcElement *block)
{
    if(block->type() == "HYDR_PUMP") delete block;
}

std::string Type()
{
    return "HYDR_PUMP";
}
//функция использовалась в оригинальном коде, видимо для определения направления движения жидкости через насос
/*
  void calculateFlowrate(BlockData &blockData)
{
    Port *inPort = blockData.OutputPorts[0];
    HydraulicBranch *branch = inPort->Branch;
    HydraulicNode *node = branch->StartNode;

    if(node == inPort->Node)
    {
        branch->FlowRate = CURRENT_FLOWRATE;
    }
    else
    {
        branch->FlowRate = -CURRENT_FLOWRATE;
    }
}
*/
