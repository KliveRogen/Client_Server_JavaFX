#include "GCS_AeroFilter.h"
#include "../constants_list.h"
#include <cmath>
#define GAS_CONSTANT 8.314
#define AVOGADRO_CONSTANT 6.022e23

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
    createParameter("filterVolume", "1");
    createParameter("timeFilterBreak", "50");
    createParameter("timeFilterClog", "50");

	// Сигналы блока

	// Порты блока
    inGasAeroFilter = createInputPort(0, "UNKNOWN_NAME", "INFO"); //входные параметры газа
    outGasAeroFilter = createOutputPort(1, "UNKNOWN_NAME", "INFO"); //выходные параметры газа
    outAeroFilterParameters = createOutputPort(2, "UNKNOWN_NAME", "INFO"); //выходные параметры фильтра (накопленная масса, активность)

    inFeedbackFilter = createInputPort(3, "UNKNOWN_NAME", "INFO"); // учет коэф. сопротвиления сопротивлений
    outFeedbackFilter = createOutputPort(4, "UNKNOWN_NAME", "INFO");

	// Отказы блока
    createSituation("filterBreak");
    createSituation("filterClog");
}


double GCS_AeroFilter::signumFunc(double argVal)
{
    if (argVal > 0.0) return 1.0;
    if (argVal < 0.0) return -1.0;
    return 0;
}

double GCS_AeroFilter::inertionFunc(double operationTime, double valueGiven, double valuePrev,  double step)
{
    double signArg = valueGiven - valuePrev;
    double valueDerivative = signArg / (operationTime / 3.5);
    double valueCurrent = valuePrev + valueDerivative * step;

    //условие для устранения статической ошибки из-за signumFunc
    if (valueCurrent > valuePrev && valueCurrent > valueGiven){
        valueCurrent = valueGiven;
    }else if (valueCurrent < valuePrev && valueCurrent < valueGiven){
        valueCurrent = valueGiven;
    }
    return valueCurrent;
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
    //выход обратной связи
    std::vector<std::string> outFeedbackFilterName;
    outFeedbackFilterName.push_back("Общий коэффициент сопротивления");
    outFeedbackFilter->setDataNames(outFeedbackFilterName);
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
    //рассчет начального количества частиц на фильтре
    filterParticleNumberPrev = paramToDouble("filterInitMassParticle") * AVOGADRO_CONSTANT / particleMolarMass;
    timeCounter = 0;
    situationFilterBrakePrev = 0;
    flagFilterBreak = 0;
    flagFilterClog = 0;
    timeCounterFilterBreak = 0;
    timeCounterFilterClog = 0;
    particleMassPrevWork = 0;
    filterActivityPrevWork = 0;
    filterResistantPrev = 0;
    outFilterResistance = 0;
    filterResistantCurrent = 0;
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
            maxParticleMass, filterParticleNumberDerivative1, filterParticleNumberDerivative2, fillCoef, filterVolume,
            gasActivityCurrent, gasOutputPressureCurrent, gasParticleFractionCurrent, gasVolumeFlowRateCurrent, gasTemperatureCurrent, timeFilterBreak, timeFilterClog, inFilterResistance,
            signArg, filterResistantDerivative;
    //считывание значений на входе
    gasInputVolumeFlowRate = inGasAeroFilter->getInput()[0];//объемный расход газа, м^3/с
    gasInputPressure = inGasAeroFilter->getInput()[1]; //давление газа, Па
    gasInputTemperature = inGasAeroFilter->getInput()[2];//температура газа, град. Цел.
    gasInputActivity = inGasAeroFilter->getInput()[3];//активность газа, Бк
    gasInputParticleFraction = inGasAeroFilter->getInput()[4];//объемная доля частиц в газе, отн. ед.
    inFilterResistance = inFeedbackFilter->getInput()[0]; //общий коэффициент сопротивления (от 0 до 1)

    //параметры фильтра
    filterCrossSectionalArea = paramToDouble("filterCrossSectionalArea");//площадь поперечного сечения, м^2
    filterCatchCoef = paramToDouble("filterCatchCoef");//коэффициент улавливания фильтра
    filterPressureLossCoef = paramToDouble("filterPressureLossCoef");//коэффициент перепада давления из-за осаждения частиц, устанавливается экспериментально Па/кг;
    filterFiberLength = paramToDouble("filterFiberLength");//длина волокон, приходящихся на единицу площади фильтрующего материала, м-1
    filterResistanceForce = paramToDouble("filterResistanceForce");//безразмерная сила сопротивления, действующая на единицу длины волокна
    maxParticleMass = paramToDouble("maxParticleMass"); //максимальная масса частиц на фильтре, кг
    filterVolume = paramToDouble("filterVolume"); //внутренний объем фильтра, м^3
    //время аварии
    timeFilterBreak = paramToDouble("timeFilterBreak");
    timeFilterClog = paramToDouble("timeFilterClog");
    //расчет дополнительных параметров
    particleInitNumberAir = particleDensity * filterVolume * gasInputParticleFraction * AVOGADRO_CONSTANT / particleMolarMass; //начальное число частиц в кубическом метре воздуха 1/м^3;
    gasVelocity = gasInputVolumeFlowRate / filterCrossSectionalArea; //скорость потока, м/с;
    gasFilterConstantPressureLoss = gasVelocity * gasViscosity * filterFiberLength * filterResistanceForce; //постоянный перепад давления на фильтре, Па

    //проверка на нажатие/отжатие кнопки аварии
    if (situationFilterBrakePrev != isSituationActive("filterBreak")){
        filterParticleNumberPrev = paramToDouble("filterInitMassParticle") * AVOGADRO_CONSTANT / particleMolarMass;
        if (isSituationActive("filterBreak") > situationFilterBrakePrev){
            flagFilterBreak = 1;
        }else if (isSituationActive("filterBreak") < situationFilterBrakePrev){
            flagFilterBreak = -1;
        }
    }
    //проверка на нажатие/отжатие кнопки аварии
    if (situationFilterClogPrev != isSituationActive("filterClog")){
        if (isSituationActive("filterClog") > situationFilterClogPrev){
            flagFilterClog = 1;
        }else if (isSituationActive("filterClog") < situationFilterClogPrev){
            flagFilterClog = -1;
        }
    }

    if (flagFilterClog == 1){
        timeCounterFilterClog = timeCounterFilterClog + h;
        if (timeCounterFilterClog > timeFilterClog + 5){
            flagFilterClog = 0;
            timeCounterFilterClog = 0;
        }

        signArg = 1-filterResistantPrev; //направление движения (открывается или закрывается клапан)
        //решение уравнения клапана методом Эйлера
        filterResistantDerivative = signumFunc(signArg) / timeFilterClog;
        filterResistantCurrent = filterResistantPrev + filterResistantDerivative * h;
        if (filterResistantCurrent < 0){
            filterResistantCurrent = 0;
        }else if (filterResistantCurrent > 1){
            filterResistantCurrent = 1;
        }
        //условие для устранения статической ошибки из-за signumFunc
        if (filterResistantCurrent > filterResistantPrev && filterResistantCurrent > 1){
            filterResistantCurrent = 1;
        }else if (filterResistantCurrent < filterResistantPrev && filterResistantCurrent < 1){
            filterResistantCurrent = 1;
        }
    }else if (flagFilterClog == -1){
        timeCounterFilterClog = timeCounterFilterClog + h;
        if (timeCounterFilterClog > timeFilterClog + 5){
            flagFilterClog = 0;
            timeCounterFilterClog = 0;
        }

        signArg = 0-filterResistantPrev; //направление движения (открывается или закрывается клапан)
        //решение уравнения клапана методом Эйлера
        filterResistantDerivative = signumFunc(signArg) / timeFilterClog;
        filterResistantCurrent = filterResistantPrev + filterResistantDerivative * h;
        if (filterResistantCurrent < 0){
            filterResistantCurrent = 0;
        }else if (filterResistantCurrent > 1){
            filterResistantCurrent = 1;
        }
        //условие для устранения статической ошибки из-за signumFunc
        if (filterResistantCurrent > filterResistantPrev && filterResistantCurrent > 0){
            filterResistantCurrent = 0;
        }else if (filterResistantCurrent < filterResistantPrev && filterResistantCurrent < 0){
            filterResistantCurrent = 0;
        }
    }
    //расчет выходного сопротивления фильтра
    outFilterResistance = 1 - ((1 - inFilterResistance) * (1-filterResistantCurrent));
    if (outFilterResistance > 1){
        outFilterResistance = 1;
    }

    if (flagFilterClog == 0 && !isSituationActive("filterClog")){
    timeCounter = timeCounter + h; //таймер для расчета активности на фильтре
    }
    //решение уравнения клапана методом Эйлера

    if (flagFilterBreak == 1){
        //произошел разрыв фильтра
        timeCounterFilterBreak = timeCounterFilterBreak + h;
        if (timeCounterFilterBreak > timeFilterBreak){
            flagFilterBreak = 0;
            timeCounterFilterBreak = 0;
            timeCounter = 0;
        }
        //учет инреции при аварии
        gasOutputPressureCurrent = inertionFunc(timeFilterBreak, gasInputPressure, gasPressurePrev, h);
        gasVolumeFlowRateCurrent = inertionFunc(timeFilterBreak, gasInputVolumeFlowRate, gasFlowRatePrev, h);
        gasParticleFractionCurrent = inertionFunc(timeFilterBreak, gasInputParticleFraction, gasParticleFractionPrev, h);
        gasActivityCurrent = inertionFunc(timeFilterBreak, gasInputActivity, gasActivityPrev, h);
        gasTemperatureCurrent = gasInputTemperature;
        particleMassCurrent = particleMassPrevWork;
        filterActivityCurrent = filterActivityPrevWork;
    }else if (flagFilterBreak == -1){
        //замена фильтра после аварии
        timeCounterFilterBreak = timeCounterFilterBreak + h;
        if (timeCounterFilterBreak > timeFilterBreak*3){
            flagFilterBreak = 0;
            timeCounterFilterBreak = 0;
            timeCounter = 0;
        }
        //РАСЧЕТ ДЛЯ АВАРИИ
        //расчет массы частиц на фильтре
        particleMassCurrent = particleMolarMass * filterParticleNumberPrev / AVOGADRO_CONSTANT;
        //условие достижения максимальной массы
        if (particleMassCurrent > maxParticleMass){
            particleMassCurrent = maxParticleMass;
        }
        fillCoef = 1 - particleMassCurrent / maxParticleMass;//коэффициент заполненности

        //расчет активности на фильтре. Если фильтр забился, то активность больше не накапливается
        if (particleMassCurrent < maxParticleMass){
            filterActivityCurrent = gasInputVolumeFlowRate * filterCatchCoef * gasInputActivity / gasDecayRate * (1 - exp(-gasDecayRate * 0));
        }
        //текущая активность газа на выходе
        gasActivityCurrent = gasInputActivity * (1 - filterCatchCoef * fillCoef);
        if (gasActivityCurrent < 0){
            gasActivityCurrent = 0;
        }
        gasFilterParticlePressureLoss = filterPressureLossCoef * particleMassCurrent;//падение давления за счет накопления массы частиц, Па
        gasOutputPressureCurrent = gasInputPressure - gasFilterConstantPressureLoss - gasFilterParticlePressureLoss; //текущее давление на выходе фильтра, Па
        //ограничение минимального давления
        if (gasOutputPressureCurrent < minPressure){
            gasOutputPressureCurrent = minPressure;
        }
        //текущая объемная доля частиц на выходе
        gasParticleFractionCurrent = gasInputParticleFraction - gasInputParticleFraction * filterCatchCoef * fillCoef;
        if (gasParticleFractionCurrent < 0){
            gasParticleFractionCurrent = 0;
        }
        gasVolumeFlowRateCurrent = gasInputVolumeFlowRate * (1 - gasInputParticleFraction * filterCatchCoef * fillCoef);//текущий объемный расход газа на выходе
        //РАСЧЕТ ДЛЯ АВАРИИ
        //учет инреции при замене фильтра
        gasOutputPressureCurrent = inertionFunc(timeFilterBreak, gasOutputPressureCurrent, gasPressurePrev, h);
        gasVolumeFlowRateCurrent = inertionFunc(timeFilterBreak, gasVolumeFlowRateCurrent, gasFlowRatePrev, h);
        gasParticleFractionCurrent = inertionFunc(timeFilterBreak, gasParticleFractionCurrent, gasParticleFractionPrev, h);
        gasActivityCurrent = inertionFunc(timeFilterBreak, gasActivityCurrent, gasActivityPrev, h);
        particleMassCurrent = inertionFunc(timeFilterBreak, particleMassCurrent, particleMassPrev, h);
        filterActivityCurrent = inertionFunc(timeFilterBreak, filterActivityCurrent, filterActivityPrev, h);;
        gasTemperatureCurrent = gasInputTemperature;
    }else{
        if (isSituationActive("filterBreak")){
            particleMassCurrent = particleMassPrevWork;
            filterActivityCurrent = filterActivityPrevWork;
            gasParticleFractionCurrent = gasInputParticleFraction;
            gasVolumeFlowRateCurrent = gasInputVolumeFlowRate;
            gasTemperatureCurrent = gasInputTemperature;
            gasActivityCurrent = gasInputActivity;
            gasOutputPressureCurrent = gasInputPressure;
        }else{
            //решение уравнения количества частиц на фильтре (Усовершенствованный метод Эйлера)
            filterParticleNumberDerivative1 = ((-gasDecayRate * filterParticleNumberPrev) + gasInputVolumeFlowRate * filterCatchCoef * particleInitNumberAir);
            filterParticleNumberPrime = filterParticleNumberPrev + h * filterParticleNumberDerivative1;
            filterParticleNumberDerivative2 =((-gasDecayRate * filterParticleNumberPrime) + gasInputVolumeFlowRate * filterCatchCoef * particleInitNumberAir);
            filterParticleNumberCurrent = filterParticleNumberPrev + (h / 2) * (filterParticleNumberDerivative1 + filterParticleNumberDerivative2);
            filterParticleNumberPrev = filterParticleNumberCurrent;
            //расчет массы частиц на фильтре
            particleMassCurrent = particleMolarMass * filterParticleNumberCurrent / AVOGADRO_CONSTANT;
            //условие достижения максимальной массы
            if (particleMassCurrent > maxParticleMass){
                particleMassCurrent = maxParticleMass;
            }
            particleMassPrevWork = particleMassCurrent;
            fillCoef = 1 - particleMassCurrent / maxParticleMass;//коэффициент заполненности

            //расчет активности на фильтре. Если фильтр забился, то активность больше не накапливается
            if (particleMassCurrent < maxParticleMass){
                if (flagFilterClog == 0 && !isSituationActive("filterClog")){
                filterActivityCurrent = gasInputVolumeFlowRate * filterCatchCoef * gasInputActivity / gasDecayRate * (1 - exp(-gasDecayRate * timeCounter));
                }
            }
            filterActivityPrevWork = filterActivityCurrent;
            //текущая активность газа на выходе
            gasActivityCurrent = gasInputActivity * (1 - filterCatchCoef * fillCoef);
            if (gasActivityCurrent < 0){
                gasActivityCurrent = 0;
            }
            gasFilterParticlePressureLoss = filterPressureLossCoef * particleMassCurrent;//падение давления за счет накопления массы частиц, Па
            gasOutputPressureCurrent = gasInputPressure - gasFilterConstantPressureLoss - gasFilterParticlePressureLoss; //текущее давление на выходе фильтра, Па
            //ограничение минимального давления
            if (gasOutputPressureCurrent < minPressure){
                gasOutputPressureCurrent = minPressure;
            }
            //текущая объемная доля частиц на выходе
            gasParticleFractionCurrent = gasInputParticleFraction - gasInputParticleFraction * filterCatchCoef * fillCoef;
            if (gasParticleFractionCurrent < 0){
                gasParticleFractionCurrent = 0;
            }
            gasVolumeFlowRateCurrent = gasInputVolumeFlowRate * (1 - gasInputParticleFraction * filterCatchCoef * fillCoef);//текущий объемный расход газа на выходе
            gasTemperatureCurrent = gasInputTemperature;//текущая температура газа на выходе
        }
    }

    //запоминание параметров на предыдущей итерации
    filterResistantPrev = filterResistantCurrent;
    particleMassPrev = particleMassCurrent;
    gasPressurePrev = gasOutputPressureCurrent;
    gasFlowRatePrev = gasVolumeFlowRateCurrent;
    gasParticleFractionPrev = gasParticleFractionCurrent;
    gasActivityPrev = gasActivityCurrent;
    filterActivityPrev = filterActivityCurrent;
    //запоминание параметров аварий
    situationFilterBrakePrev = isSituationActive("filterBreak");
    situationFilterClogPrev = isSituationActive("filterClog");
    //передача значений на выходные порты
    outGasAeroFilter->setOut(0, gasVolumeFlowRateCurrent);
    outGasAeroFilter->setOut(1, gasOutputPressureCurrent);
    outGasAeroFilter->setOut(2, gasTemperatureCurrent);
    outGasAeroFilter->setOut(3, gasActivityCurrent);
    outGasAeroFilter->setOut(4, gasParticleFractionCurrent);
    outAeroFilterParameters->setOut(0, filterActivityCurrent);
    outAeroFilterParameters->setOut(1, particleMassCurrent);
    //выходное сопротивление
    outFeedbackFilter->setOut(0, outFilterResistance);
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
