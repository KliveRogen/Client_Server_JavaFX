#include "GCS_ColdTrap.h"
#include <cmath>

GCS_ColdTrap::GCS_ColdTrap()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
	createParameter("coldTrapTemperature", "1");
	createParameter("coldTrapTempCoef", "0.9");
	createParameter("coldTrapTubeCrossSectionalArea", "0.008");
	createParameter("coldTrapNetPenetrateCoef", "1e-6");
	createParameter("coldTrapTubeEffLength", "10");
	createParameter("coldTrapVolume", "1");
	createParameter("coldTrapParticleCatchCoef", "0.001");
    createParameter("coldTrapInitMassParticle", "0");
    createParameter("maxParticleMass", "10");







	// Сигналы блока

	// Порты блока
    inGasColdTrap = createInputPort(0, "UNKNOWN_NAME", "INFO");
    outGasColdTrap = createOutputPort(1, "UNKNOWN_NAME", "INFO");
    outColdTrapParameters = createOutputPort(2, "UNKNOWN_NAME", "INFO");

	// Отказы блока

}

void GCS_ColdTrap::setDataNames()
{
    //выходные параметры газа
    std::vector<std::string> outGasColdTrapName;
    outGasColdTrapName.push_back("Объемный расход, м^3/с");
    outGasColdTrapName.push_back("Давление, Па");
    outGasColdTrapName.push_back("Температура, °C");
    outGasColdTrapName.push_back("Объемная активность газа, Бк/м^3");
    outGasColdTrapName.push_back("Объемная доля частиц в газе, отн. ед.");
    outGasColdTrap->setDataNames(outGasColdTrapName);
    //выходные параметры ловушки
    std::vector<std::string> outColdTrapParametersName;
    outColdTrapParametersName.push_back("Активность на холодной ловушке, Бк");
    outColdTrapParametersName.push_back("Масса частиц на холодной ловушке, кг");
    outColdTrapParameters->setDataNames(outColdTrapParametersName);
}

bool GCS_ColdTrap::init(std::string &error, double h)
{
    // Put your initialization here
    //проверка корректности параметров
    for (int i = 0; i < (int)Parameters.size(); i++){
        if (paramToDouble(Parameters[i])<0){
            error = "Ошибка в заполнении исходных данных!";
            return false;
        }
    }
    //постоянные параметры газа
    particleDensity = 100;//плотность частиц, кг/м^3
    particleMolarMass = 0.012;//молярная масса оседающей частицы, кг/моль
    gasSpecificConstant = 287; // удельная газовая постоянная, Дж/(кг*К)
    gasDecayRate = 2.3e-6; //постоянная распада,с-1;
    gasViscosity = 1.78e-5; //вязкость газа, Па*с;
    //подсчет начального количества частиц на фильтре
    coldTrapParticleNumberPrev = paramToDouble("coldTrapInitMassParticle")*6.22*pow(10,23)/particleMolarMass;
	setDataNames();
    return true;
}

bool GCS_ColdTrap::process(double t, double h, std::string &error)
{
    // Put your calculations here
    double  gasOutputPressureCurrent, gasTemperatureCurrent, gasVolumeFlowRateCurrent, coldTrapParticleNumberCurrent, particleMassCurrent,\
            coldTrapParticleNumberPrime, particleInitNumberAir,
            coldTrapTemperature, coldTrapTempCoef, coldTrapTubeCrossSectionalArea, coldTrapNetPenetrateCoef, coldTrapTubeEffLength,
            coldTrapVolume, coldTrapParticleCatchCoef, gasDensity, gasMassFlowRate, gasAirActivity,
            gasInputVolumeFlowRate, gasInputPressure, gasInputActivity, gasInputParticleFraction, gasInputTemperature, maxParticleMass,
            coldTrapParticleNumberDerivative1, coldTrapParticleNumberDerivative2, fillCoef, gasActivityCurrent, gasColdTrapPressureLoss,
            gasParticleFractionCurrent;
    //считывание значений на входе
    gasInputVolumeFlowRate = inGasColdTrap->getInput()[0];//объемный расход газа, м^3/с
    gasInputPressure = inGasColdTrap->getInput()[1];//давление газа, Па
    gasInputTemperature = inGasColdTrap->getInput()[2];//температура газа, град. Цел.
    gasInputActivity = inGasColdTrap->getInput()[3];//активность газа, Бк
    gasInputParticleFraction = inGasColdTrap->getInput()[4];//объемная доля частиц в газе, отн. ед.
    //параметры холодной ловушки
    coldTrapTemperature = paramToDouble("coldTrapTemperature"); //температура холодной ловушки, град Цел.
    coldTrapTempCoef = paramToDouble("coldTrapTempCoef"); //температурный коэф. (от 0 до 1)
    coldTrapTubeCrossSectionalArea = paramToDouble("coldTrapTubeCrossSectionalArea"); //площадь поперечного сечения трубки ловушки, м^2
    coldTrapNetPenetrateCoef = paramToDouble("coldTrapNetPenetrateCoef");//проницаемость сетки
    coldTrapTubeEffLength=paramToDouble("coldTrapTubeEffLength");//длина, на протяжении которой измеряется перепад давления, м;
    coldTrapVolume = paramToDouble("coldTrapVolume"); //объем холодной ловушки;
    coldTrapParticleCatchCoef = paramToDouble("coldTrapParticleCatchCoef"); //коэффициент улавливания частиц
    maxParticleMass = paramToDouble("maxParticleMass"); //максимальная масса частиц на ловушке, кг
    //рассчет дополнительных параметров
    particleInitNumberAir=particleDensity*1*gasInputParticleFraction*6.22*pow(10,23)/particleMolarMass; //начальное число частиц в кубическом метре воздуха 1/м^3;
    gasAirActivity=gasInputActivity  * coldTrapVolume;//активность газа в воздухе
    gasDensity = gasInputPressure/(gasSpecificConstant*(coldTrapTemperature+273));//плотность газа, кг/м3
    gasMassFlowRate = gasInputVolumeFlowRate * gasDensity; //массовый расход, кг/с
    //решение усовершенствованным методом Эйлера (количество частиц на фильтре)
    coldTrapParticleNumberDerivative1 = ((-gasDecayRate*coldTrapParticleNumberPrev)+gasInputVolumeFlowRate*coldTrapParticleCatchCoef*particleInitNumberAir);
    coldTrapParticleNumberPrime = coldTrapParticleNumberPrev+h*coldTrapParticleNumberDerivative1;
    coldTrapParticleNumberDerivative2 = ((-gasDecayRate*coldTrapParticleNumberPrime)+gasInputVolumeFlowRate*coldTrapParticleCatchCoef*particleInitNumberAir);
    coldTrapParticleNumberCurrent = coldTrapParticleNumberPrev+(h/2)*( coldTrapParticleNumberDerivative1+ coldTrapParticleNumberDerivative2);
    coldTrapParticleNumberPrev = coldTrapParticleNumberCurrent;
    //расчет массы частиц на холодной ловушке
    particleMassCurrent = particleMolarMass*coldTrapParticleNumberCurrent/6.22/pow(10,23);
    if (particleMassCurrent > maxParticleMass){
        particleMassCurrent = maxParticleMass;
    }
    //расчет накопленной активности на холодной ловушке с учетом заполненности ловушки
    if (particleMassCurrent < maxParticleMass){
        coldTrapActivityCurrent = gasInputVolumeFlowRate*coldTrapParticleCatchCoef*gasAirActivity/gasDecayRate* (1-exp(-gasDecayRate*t));
    }
    fillCoef = 1-particleMassCurrent/maxParticleMass;//коэффициент заполненности
    gasActivityCurrent = gasInputActivity*(1-coldTrapParticleCatchCoef*fillCoef); //текущая активность газа, Бк
    if (gasActivityCurrent < 0){
        gasActivityCurrent = 0;
    }
    //расчет перепада давления в ловушке
    gasColdTrapPressureLoss = gasViscosity * coldTrapTubeEffLength * gasMassFlowRate/coldTrapNetPenetrateCoef/gasDensity/coldTrapTubeCrossSectionalArea;
    //выходное давление ловушки
    gasOutputPressureCurrent = gasInputPressure - gasColdTrapPressureLoss;
    if(gasOutputPressureCurrent<gasInputPressure/2){//учет того, что давление не может так просто упасть до нуля
        gasOutputPressureCurrent = gasInputPressure/2;
    }
    //текущая температура ловушки
    gasTemperatureCurrent = (1-coldTrapTempCoef)*(gasInputTemperature - coldTrapTemperature)+coldTrapTemperature;
    //текущая объемная доля частиц в газе
    gasParticleFractionCurrent = gasInputParticleFraction - gasInputParticleFraction*coldTrapParticleCatchCoef*fillCoef;
    if (gasParticleFractionCurrent < 0){
        gasParticleFractionCurrent = 0;
    }
    //текущий расход газа
    gasVolumeFlowRateCurrent = gasInputVolumeFlowRate*(1-gasInputParticleFraction*coldTrapParticleCatchCoef*fillCoef);
    outGasColdTrap->setOut(0, gasVolumeFlowRateCurrent);
    outGasColdTrap->setOut(1, gasOutputPressureCurrent);
    outGasColdTrap->setOut(2, gasTemperatureCurrent);
    outGasColdTrap->setOut(3, gasActivityCurrent);
    outGasColdTrap->setOut(4, gasParticleFractionCurrent);
    outColdTrapParameters->setOut(0, coldTrapActivityCurrent);
    outColdTrapParameters->setOut(1, particleMassCurrent);
    return true;
}


ICalcElement *Create()
{
    return new GCS_ColdTrap();
}

void Release(ICalcElement *block)
{
    if(block->type() == "GCS_ColdTrap") delete block;
}

std::string Type()
{
    return "GCS_ColdTrap";
}
