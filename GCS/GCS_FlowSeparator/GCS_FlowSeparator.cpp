#include "GCS_FlowSeparator.h"

GCS_FlowSeparator::GCS_FlowSeparator()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока

	// Сигналы блока

	// Порты блока
    inGas = createInputPort(0, "UNKNOWN_NAME", "INFO");
    outGas1 = createOutputPort(1, "UNKNOWN_NAME", "INFO");
    outGas2 = createOutputPort(2, "UNKNOWN_NAME", "INFO");

	// Отказы блока

}

void GCS_FlowSeparator::setDataNames()
{
    //выходные параметры первого потока газа
    std::vector<std::string> outGas1Name;
    outGas1Name.push_back("Объемный расход, м^3/с");
    outGas1Name.push_back("Давление, Па");
    outGas1Name.push_back("Температура, °C");
    outGas1Name.push_back("Объемная активность газа, Бк/м^3");
    outGas1Name.push_back("Объемная доля частиц в газе, отн. ед.");
    outGas1->setDataNames(outGas1Name);
    //выходные параметры второго потока газа
    std::vector<std::string> outGas2Name;
    outGas2Name.push_back("Объемный расход, м^3/с");
    outGas2Name.push_back("Давление, Па");
    outGas2Name.push_back("Температура, °C");
    outGas2Name.push_back("Объемная активность газа, Бк/м^3");
    outGas2Name.push_back("Объемная доля частиц в газе, отн. ед.");
    outGas2->setDataNames(outGas2Name);
}

bool GCS_FlowSeparator::init(std::string &error, double h)
{
    // Put your initialization here
	
	setDataNames();
    return true;
}

bool GCS_FlowSeparator::process(double t, double h, std::string &error)
{
    // Put your calculations here
    double gasVolumeFlowRate, gasInputPressure, gasInputTemperature, gasInputActivity, gasInputParticleFraction, gas1VolumeFlowRateCurrent, gas1OutputPressureCurrent, gas1TemperatureCurrent, gas1ActivityCurrent, gas1ParticleFractionCurrent,
            gas2VolumeFlowRateCurrent, gas2OutputPressureCurrent, gas2TemperatureCurrent, gas2ActivityCurrent, gas2ParticleFractionCurrent;
    //считывание входных значений
    //первый поток газа
    gasVolumeFlowRate = inGas->getInput()[0];//объемный расход газа, м^3/с
    gasInputPressure = inGas->getInput()[1];//давление газа, Па
    gasInputTemperature = inGas->getInput()[2];//температура газа, град. Цел.
    gasInputActivity = inGas->getInput()[3];//активность газа, Бк
    gasInputParticleFraction = inGas->getInput()[4];//объемная доля частиц в газе, отн. ед.
    //расчет параметров газа
    gas1VolumeFlowRateCurrent= gasVolumeFlowRate/2; //текущий объемный расход на первом выходе, м^3/с
    gas2VolumeFlowRateCurrent = gasVolumeFlowRate/2; //текущий объемный расход на втором выходе, м^3/с
    gas1OutputPressureCurrent= gasInputPressure;//текущее давление на первом выходе, Па
    gas2OutputPressureCurrent = gasInputPressure;//текущее давление на втором выходе, Па
    gas1TemperatureCurrent=gasInputTemperature;//текущая температура газа на первом выходе
    gas2TemperatureCurrent=gasInputTemperature;//текущая температура газа на втором выходе
    gas1ActivityCurrent=gasInputActivity;//текущая активность газа на первом выходе
    gas2ActivityCurrent=gasInputActivity;//текущая активность газа на втором выходе
    gas1ParticleFractionCurrent=gasInputParticleFraction;//текущая объемная доля частиц в газе на первом выходе
    gas2ParticleFractionCurrent=gasInputParticleFraction;//текущая объемная доля частиц в газе на втором выходе
    //передача значений на выходной порт
    outGas1->setOut(0, gas1VolumeFlowRateCurrent);
    outGas1->setOut(1, gas1OutputPressureCurrent);
    outGas1->setOut(2, gas1TemperatureCurrent);
    outGas1->setOut(3, gas1ActivityCurrent);
    outGas1->setOut(4, gas1ParticleFractionCurrent);
    outGas2->setOut(0, gas2VolumeFlowRateCurrent);
    outGas2->setOut(1, gas2OutputPressureCurrent);
    outGas2->setOut(2, gas2TemperatureCurrent);
    outGas2->setOut(3, gas2ActivityCurrent);
    outGas2->setOut(4, gas2ParticleFractionCurrent);
    return true;
}


ICalcElement *Create()
{
    return new GCS_FlowSeparator();
}

void Release(ICalcElement *block)
{
    if(block->type() == "GCS_FlowSeparator") delete block;
}

std::string Type()
{
    return "GCS_FlowSeparator";
}