#include "GCS_Pipe.h"
#include <cmath>

GCS_Pipe::GCS_Pipe()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
    createParameter("pipeDiameter", "0.05");
    createParameter("frictionCoef", "1");
    createParameter("gasDensity", "1");
    createParameter("pipeLength", "10");

	// Сигналы блока

	// Порты блока
    inGasPipe = createInputPort(0, "UNKNOWN_NAME", "INFO");
    outGasPipe = createOutputPort(1, "UNKNOWN_NAME", "INFO");

	// Отказы блока

}

void GCS_Pipe::setDataNames()
{
	std::vector<std::string> dn;
    dn.push_back("Объемный расход, м^3/с");
    dn.push_back("Давление, Па");
    dn.push_back("Температура, °C");
    dn.push_back("Объемная активность газа, Бк/м^3");
    dn.push_back("Объемная доля частиц в газе, отн. ед.");
    outGasPipe->setDataNames(dn);
}

bool GCS_Pipe::init(std::string &error, double h)
{
    //проверка корректности параметров
    for (int i = 0; i < (int)Parameters.size(); i++){
        if (paramToDouble(Parameters[i])<0){
            error = "Ошибка в заполнении исходных данных!";
            return false;
        }
    }
	setDataNames();
    return true;
}

bool GCS_Pipe::process(double t, double h, std::string &error)
{
    // Put your calculations here
    double gasInputVolumeFlowRate, gasInputPressure, gasInputTemperature, gasInputActivity, gasInputParticleFraction, gasOutputPressureCurrent, gasPressureLoss,
            gasVolumeFlowRateCurrent, gasTemperatureCurrent, gasActivityCurrent, gasParticleFractionCurrent, gasDensity, pipeDiameter, frictionCoef, pipeLength;
    //считывание значений на входе
    gasInputVolumeFlowRate = inGasPipe->getInput()[0];//объемный расход газа, м^3/с
    gasInputPressure = inGasPipe->getInput()[1];//давление газа, Па
    gasInputTemperature = inGasPipe->getInput()[2];//температура газа, град. Цел.
    gasInputActivity = inGasPipe->getInput()[3];//активность газа, Бк
    gasInputParticleFraction = inGasPipe->getInput()[4];//объемная доля частиц в газе, отн. ед.
    //параметры трубы
    pipeDiameter = paramToDouble("pipeDiameter"); //диаметр трубы, м
    frictionCoef = paramToDouble("frictionCoef"); //коэффициент трения
    pipeLength = paramToDouble("pipeLength"); //длина трубы, м
    //постоянные параметры газа
    gasDensity = 1; //плотность газа, кг/м^3
    //рассчет выходных параметров
    gasPressureLoss = frictionCoef*pow(gasInputVolumeFlowRate, 2)*gasDensity*pipeLength/pow(pipeDiameter,5);//перепад давления в трубе, Па
    gasOutputPressureCurrent = gasInputPressure-gasPressureLoss;//текущее давление на выходе трубы, Па
    gasVolumeFlowRateCurrent=gasInputVolumeFlowRate;//текущий об. расх. газа на выходе трубы, м^3/с
    gasTemperatureCurrent=gasInputTemperature;//текущая температура газа на выходе трубы, град. Цел.
    gasActivityCurrent=gasInputActivity;//текущая температура на выходе трубы, град. Цел.
    gasParticleFractionCurrent=gasInputParticleFraction;//текущая доля частиц на выходе трубы
    outGasPipe->setOut(0, gasVolumeFlowRateCurrent);
    outGasPipe->setOut(1, gasOutputPressureCurrent);
    outGasPipe->setOut(2, gasTemperatureCurrent);
    outGasPipe->setOut(3, gasActivityCurrent);
    outGasPipe->setOut(4, gasParticleFractionCurrent);
    return true;
}


ICalcElement *Create()
{
    return new GCS_Pipe();
}

void Release(ICalcElement *block)
{
    if(block->type() == "GCS_Pipe") delete block;
}

std::string Type()
{
    return "GCS_Pipe";
}
