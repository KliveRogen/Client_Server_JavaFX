#include "GCS_FlowConnector.h"

GCS_FlowConnector::GCS_FlowConnector()
{
	// Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока


	// Сигналы блока

	// Порты блока
    inGasFirst=createInputPort(0, "UNKNOWN_NAME", "INFO");
    inGasSecond=createInputPort(1, "UNKNOWN_NAME", "INFO");
    outGas=createOutputPort(2, "UNKNOWN_NAME", "INFO");

	// Отказы блока
	createSituation("calcDist");

}
void GCS_FlowConnector::setDataNames()
{
    //выходные параметры газа
    std::vector<std::string> dn1;
    dn1.push_back("Объемный расход, м^3/с");
    dn1.push_back("Давление, Па");
    dn1.push_back("Температура, °C");
    dn1.push_back("Объемная активность газа, Бк/м^3");
    dn1.push_back("Объемная доля частиц в газе, отн. ед.");
    outGas->setDataNames(dn1);
}
bool GCS_FlowConnector::init(std::string &error, double h)
{
    //проверка корректности параметров
    for (int i = 0; i < (int)Parameters.size(); i++){
        if (paramToDouble(Parameters[i])<0){
            error = "Ошибка в заполнении исходных данных!";
            return false;
        }
    }
    // Put your initialization here
    setDataNames();

    gasVolumeFlowRateCurrent=0;
    gasOutputPressureCurrent=0;
    gasTemperatureCurrent=0;
    gasActivityCurrent=0;
    gasParticleFractionCurrent=0;

    outGas->setOut(0, gasVolumeFlowRateCurrent);
    outGas->setOut(1, gasOutputPressureCurrent);
    outGas->setOut(2, gasTemperatureCurrent);
    outGas->setOut(3, gasActivityCurrent);
    outGas->setOut(4, gasParticleFractionCurrent);


    return true;
}
bool GCS_FlowConnector::process(double t, double h, std::string &error)
{
    // Put your calculations here

    double gasFirstVolumeFlowRate, gasFirstInputPressure, gasFirstInputTemperature, gasFirstInputActivity,
            gasFirstInputParticleFraction, gasSecondVolumeFlowRate, gasSecondInputPressure, gasSecondInputTemperature, gasSecondInputActivity,
            gasSecondInputParticleFraction;

    //считывание входных значений
    gasFirstVolumeFlowRate = inGasFirst->getInput()[0];//объемный расход первого потока газа, м^3/с
    gasFirstInputPressure = inGasFirst->getInput()[1];//давление первого потока газа, Па
    gasFirstInputTemperature = inGasFirst->getInput()[2];//температура первого потока газа, град. Цел.
    gasFirstInputActivity = inGasFirst->getInput()[3];//активность первого потока газа, Бк
    gasFirstInputParticleFraction = inGasFirst->getInput()[4];//объемная доля частиц в первом потоке газа, отн. ед.
    gasSecondVolumeFlowRate = inGasSecond->getInput()[0];//объемный расход второго потока газа, м^3/с
    gasSecondInputPressure = inGasSecond->getInput()[1];//давление второго потока газа, Па
    gasSecondInputTemperature = inGasSecond->getInput()[2];//температура второго потока газа, град. Цел.
    gasSecondInputActivity = inGasSecond->getInput()[3];//активность второго потока газа, Бк
    gasSecondInputParticleFraction = inGasSecond->getInput()[4];//объемная доля частиц во втором потоке газе, отн. ед.
    //рассчет эжектора
    gasVolumeFlowRateCurrent= gasSecondVolumeFlowRate+gasFirstVolumeFlowRate; //текущий объемный расход на выходе, м^3/с

    gasOutputPressureCurrent= gasFirstVolumeFlowRate/gasVolumeFlowRateCurrent*gasFirstInputPressure+
            gasSecondVolumeFlowRate/gasVolumeFlowRateCurrent*gasSecondInputPressure;//текущее давление на выходе, Па
    gasTemperatureCurrent=gasFirstVolumeFlowRate/gasVolumeFlowRateCurrent*gasFirstInputTemperature+
            gasSecondVolumeFlowRate/gasVolumeFlowRateCurrent*gasSecondInputTemperature;//текущая температура газа на выходе
    gasActivityCurrent=gasFirstVolumeFlowRate/gasVolumeFlowRateCurrent*gasFirstInputActivity+
            gasSecondVolumeFlowRate/gasVolumeFlowRateCurrent*gasSecondInputActivity;//текущая активность газа на выходе
    gasParticleFractionCurrent=gasFirstVolumeFlowRate/gasVolumeFlowRateCurrent*gasFirstInputParticleFraction+
            gasSecondVolumeFlowRate/gasVolumeFlowRateCurrent*gasSecondInputParticleFraction;//текущая объемная доля частиц в газе на выходе
    //передача значений на выходные порты
    outGas->setNewOut(0, gasVolumeFlowRateCurrent);
    outGas->setNewOut(1, gasOutputPressureCurrent);
    outGas->setNewOut(2, gasTemperatureCurrent);
    outGas->setNewOut(3, gasActivityCurrent);
    outGas->setNewOut(4, gasParticleFractionCurrent);



    return true;
}
ICalcElement *Create()
{
    return new GCS_FlowConnector();
}
void Release(ICalcElement *block)
{
    if(block->type() == "GCS_FlowConnector") delete block;
}
std::string Type()
{
    return "GCS_FlowConnector";
}
