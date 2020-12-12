#include "GCS_Ejector.h"

GCS_Ejector::GCS_Ejector()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
	createParameter("ejectorLossCoef", "0.5");

	// Сигналы блока

	// Порты блока
    inPassiveGasEjector=createInputPort(0, "UNKNOWN_NAME", "INFO");
    inActiveGasEjector=createInputPort(1, "UNKNOWN_NAME", "INFO");
    outGasEjector=createOutputPort(2, "UNKNOWN_NAME", "INFO");

	// Отказы блока
	createSituation("calcDist");

}
void GCS_Ejector::setDataNames()
{
    //выходные параметры газа
    std::vector<std::string> outGasEjectorName;
    outGasEjectorName.push_back("Объемный расход, м^3/с");
    outGasEjectorName.push_back("Давление, Па");
    outGasEjectorName.push_back("Температура, °C");
    outGasEjectorName.push_back("Объемная активность газа, Бк/м^3");
    outGasEjectorName.push_back("Объемная доля частиц в газе, отн. ед.");
    outGasEjector->setDataNames(outGasEjectorName);
}
bool GCS_Ejector::init(std::string &error, double h)
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
    return true;
}
bool GCS_Ejector::process(double t, double h, std::string &error)
{
    // Put your calculations here

    double ejectorLossCoef, passiveGasVolumeFlowRate, passiveGasInputPressure, passiveGasInputTemperature, passiveGasInputActivity,
            passiveGasInputParticleFraction, activeGasVolumeFlowRate, activeGasInputPressure, activeGasInputTemperature, activeGasInputActivity,
            activeGasInputParticleFraction, gasVolumeFlowRateCurrent, gasOutputPressureCurrent, gasTemperatureCurrent, gasActivityCurrent,
            gasParticleFractionCurrent;
    //параметры эжектора
    ejectorLossCoef = paramToDouble("ejectorLossCoef"); //коэффициент эффективности эжекции (от 0 до 1)
    //считывание входных значений
    passiveGasVolumeFlowRate = inPassiveGasEjector->getInput()[0];//объемный расход пассивного газа, м^3/с
    passiveGasInputPressure = inPassiveGasEjector->getInput()[1];//давление пассивного газа, Па
    passiveGasInputTemperature = inPassiveGasEjector->getInput()[2];//температура пассивного газа, град. Цел.
    passiveGasInputActivity = inPassiveGasEjector->getInput()[3];//активность пассивного газа, Бк
    passiveGasInputParticleFraction = inPassiveGasEjector->getInput()[4];//объемная доля частиц в пассивном газе, отн. ед.
    activeGasVolumeFlowRate = inActiveGasEjector->getInput()[0];//объемный расход активного газа, м^3/с
    activeGasInputPressure = inActiveGasEjector->getInput()[1];//давление активного газа, Па
    activeGasInputTemperature = inActiveGasEjector->getInput()[2];//температура активного газа, град. Цел.
    activeGasInputActivity = inActiveGasEjector->getInput()[3];//активность активного газа, Бк
    activeGasInputParticleFraction = inActiveGasEjector->getInput()[4];//объемная доля частиц в активном газе, отн. ед.
    //рассчет эжектора
    gasVolumeFlowRateCurrent= ejectorLossCoef*activeGasVolumeFlowRate+passiveGasVolumeFlowRate; //текущий объемный расход на выходе, м^3/с
    if (gasVolumeFlowRateCurrent>activeGasVolumeFlowRate){
        gasVolumeFlowRateCurrent=activeGasVolumeFlowRate;
    }
    gasOutputPressureCurrent= passiveGasVolumeFlowRate/gasVolumeFlowRateCurrent*passiveGasInputPressure+
            activeGasVolumeFlowRate/gasVolumeFlowRateCurrent*activeGasInputPressure;//текущее давление на выходе, Па
    gasTemperatureCurrent=passiveGasVolumeFlowRate/gasVolumeFlowRateCurrent*passiveGasInputTemperature+
            activeGasVolumeFlowRate/gasVolumeFlowRateCurrent*activeGasInputTemperature;//текущая температура газа на выходе
    gasActivityCurrent=passiveGasVolumeFlowRate/gasVolumeFlowRateCurrent*passiveGasInputActivity+
            activeGasVolumeFlowRate/gasVolumeFlowRateCurrent*activeGasInputActivity;//текущая активность газа на выходе
    gasParticleFractionCurrent=passiveGasVolumeFlowRate/gasVolumeFlowRateCurrent*passiveGasInputParticleFraction+
            activeGasVolumeFlowRate/gasVolumeFlowRateCurrent*activeGasInputParticleFraction;//текущая объемная доля частиц в газе на выходе
    //передача значений на выходные порты
    outGasEjector->setOut(0, gasVolumeFlowRateCurrent);
    outGasEjector->setOut(1, gasOutputPressureCurrent);
    outGasEjector->setOut(2, gasTemperatureCurrent);
    outGasEjector->setOut(3, gasActivityCurrent);
    outGasEjector->setOut(4, gasParticleFractionCurrent);
    return true;
}
ICalcElement *Create()
{
    return new GCS_Ejector();
}
void Release(ICalcElement *block)
{
    if(block->type() == "GCS_Ejector") delete block;
}
std::string Type()
{
    return "GCS_Ejector";
}
