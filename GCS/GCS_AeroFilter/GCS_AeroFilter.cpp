#include "GCS_AeroFilter.h"
#include <cmath>

GCS_AeroFilter::GCS_AeroFilter()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
	createParameter("filterCrossSectionalArea", "0.01");
	createParameter("filterCatchCoef", "0.4");
	createParameter("filterPressureLossCoef", "1000");
	createParameter("filterFiberLength", "10");
	createParameter("filterResistanceForce", "20");
    createParameter("filterInitMassParticle", "0");
    createParameter("maxParticleMass", "10");

	// Сигналы блока

	// Порты блока
    inGasAeroFilter = createInputPort(0, "UNKNOWN_NAME", "INFO"); //входные параметры газа
    outGasAeroFilter = createOutputPort(1, "UNKNOWN_NAME", "INFO"); //выходные параметры газа
    outAeroFilterParameters = createOutputPort(2, "UNKNOWN_NAME", "INFO"); //выходные параметры фильтра (накопленная масса, активность)

	// Отказы блока

}

void GCS_AeroFilter::setDataNames()
{
    //выходные параметры газа
    std::vector<std::string> outGasAeroFilterName;
    outGasAeroFilterName.push_back("Объемный расход, м^3/с");
    outGasAeroFilterName.push_back("Давление, Па");
    outGasAeroFilterName.push_back("Температура, °C");
    outGasAeroFilterName.push_back("Объемная активность газа, Бк/м^3");
    outGasAeroFilterName.push_back("Объемная доля частиц в газе, отн. ед.");
    outGasAeroFilter->setDataNames(outGasAeroFilterName);
    //выходные параметры фильтра
    std::vector<std::string> outAeroFilterParametersName;
    outAeroFilterParametersName.push_back("Активность на фильтре, Бк");
    outAeroFilterParametersName.push_back("Масса частиц на фильтре, кг");
    outAeroFilterParameters->setDataNames(outAeroFilterParametersName);
}

bool GCS_AeroFilter::init(std::string &error, double h)
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
    gasDecayRate = 2.3e-6;//постоянная распада нуклида,с-1
    particleDensity = 100;//плотность частиц, кг/м^3
    particleMolarMass = 0.012;//молярная масса оседающей частицы, кг/моль
    gasViscosity = 1.78e-5;//µ – вязкость газа, Па*с;
    //рассчет начального количества частиц на фильтре
    filterParticleNumberPrev = paramToDouble("filterInitMassParticle")*6.22*pow(10,23)/particleMolarMass;
	setDataNames();
    return true;
}

bool GCS_AeroFilter::process(double t, double h, std::string &error)
{
    // Put your calculations here
    double gasVelocity, gasFilterConstantPressureLoss, filterParticleNumberPrime, filterParticleNumberCurrent, particleMassCurrent,
            gasFilterParticlePressureLoss, gasInputVolumeFlowRate, gasInputPressure, gasInputTemperature,
            gasInputActivity, gasInputParticleFraction, particleInitNumberAir, filterCrossSectionalArea,
            filterCatchCoef, filterPressureLossCoef, filterFiberLength, filterResistanceForce,
            maxParticleMass, filterParticleNumberDerivative1, filterParticleNumberDerivative2, fillCoef,
            gasActivityCurrent, gasOutputPressureCurrent, gasParticleFractionCurrent, gasVolumeFlowRateCurrent, gasTemperatureCurrent;
    //считывание значений на входе
    gasInputVolumeFlowRate = inGasAeroFilter->getInput()[0];//объемный расход газа, м^3/с
    gasInputPressure = inGasAeroFilter->getInput()[1]; //давление газа, Па
    gasInputTemperature = inGasAeroFilter->getInput()[2];//температура газа, град. Цел.
    gasInputActivity = inGasAeroFilter->getInput()[3];//активность газа, Бк
    gasInputParticleFraction = inGasAeroFilter->getInput()[4];//объемная доля частиц в газе, отн. ед.
    //параметры фильтра
    filterCrossSectionalArea = paramToDouble("filterCrossSectionalArea");//площадь поперечного сечения, м^2 
    filterCatchCoef = paramToDouble("filterCatchCoef");//коэффициент улавливания фильтра
    filterPressureLossCoef = paramToDouble("filterPressureLossCoef");//коэффициент перепада давления из-за осаждения частиц, устанавливается экспериментально Па/кг;
    filterFiberLength = paramToDouble("filterFiberLength");//длина волокон, приходящихся на единицу площади фильтрующего материала, м-1
    filterResistanceForce = paramToDouble("filterResistanceForce");//безразмерная сила сопротивления, действующая на единицу длины волокна    
    maxParticleMass = paramToDouble("maxParticleMass"); //максимальная масса частиц на фильтре, кг

    //расчет дополнительных параметров
    particleInitNumberAir=particleDensity*1*gasInputParticleFraction*6.22*pow(10,23)/particleMolarMass; //начальное число частиц в кубическом метре воздуха 1/м^3;
    gasVelocity=gasInputVolumeFlowRate/filterCrossSectionalArea; //скорость потока, м/с;
    gasFilterConstantPressureLoss=gasVelocity*gasViscosity*filterFiberLength*filterResistanceForce; //постоянный перепад давления на фильтре, Па
    //решение уравнения количества частиц на фильтре (Усовершенствованный метод Эйлера)
    filterParticleNumberDerivative1=((-gasDecayRate*filterParticleNumberPrev)+gasInputVolumeFlowRate*filterCatchCoef*particleInitNumberAir);
    filterParticleNumberPrime=filterParticleNumberPrev+h*filterParticleNumberDerivative1;
    filterParticleNumberDerivative2=((-gasDecayRate*filterParticleNumberPrime)+gasInputVolumeFlowRate*filterCatchCoef*particleInitNumberAir);
    filterParticleNumberCurrent=filterParticleNumberPrev+(h/2)*( filterParticleNumberDerivative1+ filterParticleNumberDerivative2);
    filterParticleNumberPrev = filterParticleNumberCurrent;
    //расчет массы частиц на фильтре
    particleMassCurrent = particleMolarMass*filterParticleNumberCurrent/6.22/pow(10,23);
    //условие достижения максимальной массы
    if (particleMassCurrent>maxParticleMass){
        particleMassCurrent=maxParticleMass;
    }
    fillCoef = 1-particleMassCurrent/maxParticleMass;//коэффициент заполненности
    //расчет активности на фильтре. Если фильтр забился, то активность больше не накапливается
    if (particleMassCurrent<maxParticleMass){
        filterActivityCurrent = gasInputVolumeFlowRate*filterCatchCoef*gasInputActivity/gasDecayRate* (1-exp(-gasDecayRate*t));
    }
    //текущая активность газа на выходе
    gasActivityCurrent = gasInputActivity*(1-filterCatchCoef*fillCoef);
    if (gasActivityCurrent<0){
        gasActivityCurrent=0;
    }
    gasFilterParticlePressureLoss= filterPressureLossCoef*particleMassCurrent;//падение давления за счет накопления массы частиц, Па
    gasOutputPressureCurrent= gasInputPressure-gasFilterConstantPressureLoss-gasFilterParticlePressureLoss; //текущее давление на выходе фильтра, Па
    if(gasOutputPressureCurrent<gasInputPressure/2){//учет того, что давление не может так просто упасть до нуля
        gasOutputPressureCurrent = gasInputPressure/2;
    }
    //текущая объемная доля частиц на выходе
    gasParticleFractionCurrent = gasInputParticleFraction - gasInputParticleFraction*filterCatchCoef*fillCoef;
    if (gasParticleFractionCurrent<0){
        gasParticleFractionCurrent=0;
    }
    gasVolumeFlowRateCurrent = gasInputVolumeFlowRate*(1-gasInputParticleFraction*filterCatchCoef*fillCoef);//текущий объемный расход газа на выходе
    gasTemperatureCurrent = gasInputTemperature;//текущая температура газа на выходе
    //передача значений на выходные порты
    outGasAeroFilter->setOut(0, gasVolumeFlowRateCurrent);
    outGasAeroFilter->setOut(1, gasOutputPressureCurrent);
    outGasAeroFilter->setOut(2, gasTemperatureCurrent);
    outGasAeroFilter->setOut(3, gasActivityCurrent);
    outGasAeroFilter->setOut(4, gasParticleFractionCurrent);
    outAeroFilterParameters->setOut(0, filterActivityCurrent);
    outAeroFilterParameters->setOut(1, particleMassCurrent);
    return true;
}


ICalcElement *Create()
{
    return new GCS_AeroFilter();
}

void Release(ICalcElement *block)
{
    if(block->type() == "GCS_AeroFilter") delete block;
}

std::string Type()
{
    return "GCS_AeroFilter";
}
