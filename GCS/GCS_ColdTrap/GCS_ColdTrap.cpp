#include "GCS_ColdTrap.h"
#include "../constants_list.h"
#include <cmath>
#define GAS_CONSTANT 8.314
#define AVOGADRO_CONSTANT 6.022e23

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
    createParameter("timeColdTrapLeak", "50");
    createParameter("timeColdTrapClog", "50");

	// Сигналы блока

	// Порты блока
    inGasColdTrap = createInputPort(0, "UNKNOWN_NAME", "INFO");
    outGasColdTrap = createOutputPort(1, "UNKNOWN_NAME", "INFO");
    outColdTrapParameters = createOutputPort(2, "UNKNOWN_NAME", "INFO");

    inFeedback = createInputPort(3, "UNKNOWN_NAME", "INFO"); // учет коэф. сопротвиления сопротивлений
    outFeedback = createOutputPort(4, "UNKNOWN_NAME", "INFO");


	// Отказы блока
    createSituation("coldTrapLeak");
    createSituation("coldTrapClog");
}

double GCS_ColdTrap::signumFunc(double argVal)
{
    if (argVal > 0.0) return 1.0;
    if (argVal < 0.0) return -1.0;
    return 0;
}

double GCS_ColdTrap::inertionFunc(double operationTime, double valueGiven, double valuePrev,  double step)
{
    double signArg = valueGiven - valuePrev;
    double valueDerivative = signArg / (operationTime / 3.8);
    double valueCurrent = valuePrev + valueDerivative * step;

    //условие для устранения статической ошибки из-за signumFunc
    if (valueCurrent > valuePrev && valueCurrent > valueGiven){
        valueCurrent = valueGiven;
    }else if (valueCurrent < valuePrev && valueCurrent < valueGiven){
        valueCurrent = valueGiven;
    }
    return valueCurrent;
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
    //выход обратной связи
    std::vector<std::string> outFeedbackName;
    outFeedbackName.push_back("Общий коэффициент сопротивления");
    outFeedback->setDataNames(outFeedbackName);
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
    //подсчет начального количества частиц на холодной ловушке
    coldTrapParticleNumberPrev = paramToDouble("coldTrapInitMassParticle") * AVOGADRO_CONSTANT / particleMolarMass;
    timeCounter = 0;
    situationColdTrapLeakPrev = 0;
    flagColdTrapLeak = 0;
    flagColdTrapClog = 0;
    timeCounterColdTrapLeak = 0;
    timeCounterColdTrapClog = 0;
    particleMassPrevWork = 0;
    coldTrapActivityPrevWork = 0;
    coldTrapResistantPrev = 0;
    outColdTrapResistance = 0;
    coldTrapResistantCurrent = 0;
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
            gasParticleFractionCurrent,  timeColdTrapLeak, timeColdTrapClog, inColdTrapResistance, signArg, coldTrapResistantDerivative;
    //считывание значений на входе
    gasInputVolumeFlowRate = inGasColdTrap->getInput()[0];//объемный расход газа, м^3/с
    gasInputPressure = inGasColdTrap->getInput()[1];//давление газа, Па
    gasInputTemperature = inGasColdTrap->getInput()[2];//температура газа, град. Цел.
    gasInputActivity = inGasColdTrap->getInput()[3];//активность газа, Бк
    gasInputParticleFraction = inGasColdTrap->getInput()[4];//объемная доля частиц в газе, отн. ед.
    inColdTrapResistance = inFeedback->getInput()[0]; //общий коэффициент сопротивления (от 0 до 1)
    //параметры холодной ловушки
    coldTrapTemperature = paramToDouble("coldTrapTemperature"); //температура холодной ловушки, град Цел.
    coldTrapTempCoef = paramToDouble("coldTrapTempCoef"); //температурный коэф. (от 0 до 1)
    coldTrapTubeCrossSectionalArea = paramToDouble("coldTrapTubeCrossSectionalArea"); //площадь поперечного сечения трубки ловушки, м^2
    coldTrapNetPenetrateCoef = paramToDouble("coldTrapNetPenetrateCoef");//проницаемость сетки
    coldTrapTubeEffLength = paramToDouble("coldTrapTubeEffLength");//длина, на протяжении которой измеряется перепад давления, м
    coldTrapVolume = paramToDouble("coldTrapVolume"); //объем холодной ловушки, м^3
    coldTrapParticleCatchCoef = paramToDouble("coldTrapParticleCatchCoef"); //коэффициент улавливания частиц
    maxParticleMass = paramToDouble("maxParticleMass"); //максимальная масса частиц на ловушке, кг
    //время аварии
    timeColdTrapLeak = paramToDouble("timeColdTrapLeak");
    timeColdTrapClog = paramToDouble("timeColdTrapClog");
    //рассчет дополнительных параметров
    particleInitNumberAir = particleDensity * coldTrapVolume * gasInputParticleFraction * AVOGADRO_CONSTANT / particleMolarMass; //начальное число частиц в кубическом метре воздуха 1/м^3;
    gasAirActivity=gasInputActivity  * coldTrapVolume;//активность газа в воздухе
    gasDensity = gasInputPressure / (gasSpecificConstant * (coldTrapTemperature + 273));//плотность газа, кг/м3
    gasMassFlowRate = gasInputVolumeFlowRate * gasDensity; //массовый расход, кг/с

    //проверка на нажатие/отжатие кнопки аварии (утечка)
    if (situationColdTrapLeakPrev != isSituationActive("coldTrapLeak")){
        if (isSituationActive("coldTrapLeak") > situationColdTrapLeakPrev){
            flagColdTrapLeak = 1;
        }else if (isSituationActive("coldTrapLeak") < situationColdTrapLeakPrev){
            flagColdTrapLeak = -1;
        }
    }

    //проверка на нажатие/отжатие кнопки аварии
    if (situationColdTrapClogPrev != isSituationActive("coldTrapClog")){
        if (isSituationActive("coldTrapClog") > situationColdTrapClogPrev){
            flagColdTrapClog = 1;
        }else if (isSituationActive("coldTrapClog") < situationColdTrapClogPrev){
            flagColdTrapClog = -1;
        }
    }

    if (flagColdTrapClog == 1){
        timeCounterColdTrapClog = timeCounterColdTrapClog + h;
        if (timeCounterColdTrapClog > timeColdTrapClog + 5){
            flagColdTrapClog = 0;
            timeCounterColdTrapClog = 0;
        }

        signArg = 1-coldTrapResistantPrev; //направление движения (открывается или закрывается клапан)
        //решение уравнения клапана методом Эйлера
        coldTrapResistantDerivative = signumFunc(signArg) / timeColdTrapClog;
        coldTrapResistantCurrent = coldTrapResistantPrev + coldTrapResistantDerivative * h;
        if (coldTrapResistantCurrent < 0){
            coldTrapResistantCurrent = 0;
        }else if (coldTrapResistantCurrent > 1){
            coldTrapResistantCurrent = 1;
        }
        //условие для устранения статической ошибки из-за signumFunc
        if (coldTrapResistantCurrent > coldTrapResistantPrev && coldTrapResistantCurrent > 1){
            coldTrapResistantCurrent = 1;
        }else if (coldTrapResistantCurrent < coldTrapResistantPrev && coldTrapResistantCurrent < 1){
            coldTrapResistantCurrent = 1;
        }
    }else if (flagColdTrapClog == -1){
        timeCounterColdTrapClog = timeCounterColdTrapClog + h;
        if (timeCounterColdTrapClog > timeColdTrapClog + 5){
            flagColdTrapClog = 0;
            timeCounterColdTrapClog = 0;
        }

        signArg = 0-coldTrapResistantPrev; //направление движения (открывается или закрывается клапан)
        //решение уравнения клапана методом Эйлера
        coldTrapResistantDerivative = signumFunc(signArg) / timeColdTrapClog;
        coldTrapResistantCurrent = coldTrapResistantPrev + coldTrapResistantDerivative * h;
        if (coldTrapResistantCurrent < 0){
            coldTrapResistantCurrent = 0;
        }else if (coldTrapResistantCurrent > 1){
            coldTrapResistantCurrent = 1;
        }
        //условие для устранения статической ошибки из-за signumFunc
        if (coldTrapResistantCurrent > coldTrapResistantPrev && coldTrapResistantCurrent > 0){
            coldTrapResistantCurrent = 0;
        }else if (coldTrapResistantCurrent < coldTrapResistantPrev && coldTrapResistantCurrent < 0){
            coldTrapResistantCurrent = 0;
        }
    }

    //расчет выходного сопротивления фильтра
    outColdTrapResistance = 1 - ((1 - inColdTrapResistance) * (1-coldTrapResistantCurrent));
    if (outColdTrapResistance > 1){
        outColdTrapResistance = 1;
    }

    if (flagColdTrapClog == 0 && !isSituationActive("coldTrapClog")){
    timeCounter = timeCounter + h; //таймер для расчета активности на фильтре
    }


    if (flagColdTrapLeak == 1){
        //произошла утечка теплоносителя
        timeCounterColdTrapLeak = timeCounterColdTrapLeak + h;
        if (timeCounterColdTrapLeak > timeColdTrapLeak){
            flagColdTrapLeak = 0;
            timeCounterColdTrapLeak = 0;

        }
        //учет инреции при аварии
        gasTemperatureCurrent = inertionFunc(timeColdTrapLeak, gasInputTemperature, gasTemperaturePrev, h);
    }else if (flagColdTrapLeak == -1){
        timeCounterColdTrapLeak = timeCounterColdTrapLeak + h;
        if (timeCounterColdTrapLeak > timeColdTrapLeak){
            flagColdTrapLeak = 0;
            timeCounterColdTrapLeak = 0;

        }
        gasTemperatureCurrent = (1 - coldTrapTempCoef)*(gasInputTemperature - coldTrapTemperature) + coldTrapTemperature;
        gasTemperatureCurrent = inertionFunc(timeColdTrapLeak, gasTemperatureCurrent, gasTemperaturePrev, h);
    }




    //решение усовершенствованным методом Эйлера (количество частиц на фильтре)
    coldTrapParticleNumberDerivative1 = ((-gasDecayRate * coldTrapParticleNumberPrev) + gasInputVolumeFlowRate * coldTrapParticleCatchCoef * particleInitNumberAir);
    coldTrapParticleNumberPrime = coldTrapParticleNumberPrev + h * coldTrapParticleNumberDerivative1;
    coldTrapParticleNumberDerivative2 = ((-gasDecayRate * coldTrapParticleNumberPrime) + gasInputVolumeFlowRate * coldTrapParticleCatchCoef * particleInitNumberAir);
    coldTrapParticleNumberCurrent = coldTrapParticleNumberPrev + (h / 2) * (coldTrapParticleNumberDerivative1 + coldTrapParticleNumberDerivative2);
    coldTrapParticleNumberPrev = coldTrapParticleNumberCurrent;
    //расчет массы частиц на холодной ловушке
    particleMassCurrent = particleMolarMass * coldTrapParticleNumberCurrent / AVOGADRO_CONSTANT;
    if (particleMassCurrent > maxParticleMass){
        particleMassCurrent = maxParticleMass;
    }
    fillCoef = 1-particleMassCurrent/maxParticleMass;//коэффициент заполненности
    //расчет накопленной активности на холодной ловушке с учетом заполненности ловушки
    if (particleMassCurrent < maxParticleMass){
        if (flagColdTrapClog == 0 && !isSituationActive("coldTrapClog")){
        coldTrapActivityCurrent = gasInputVolumeFlowRate * coldTrapParticleCatchCoef * gasAirActivity / gasDecayRate * (1 - exp(-gasDecayRate * timeCounter));
        }
    }
    coldTrapActivityPrevWork = coldTrapActivityCurrent;

    gasActivityCurrent = gasInputActivity * (1 - coldTrapParticleCatchCoef * fillCoef); //текущая активность газа, Бк
    if (gasActivityCurrent < 0){
        gasActivityCurrent = 0;
    }
    //расчет перепада давления в ловушке
    gasColdTrapPressureLoss = gasViscosity * coldTrapTubeEffLength * gasMassFlowRate / coldTrapNetPenetrateCoef / gasDensity / coldTrapTubeCrossSectionalArea;
    //выходное давление ловушки
    gasOutputPressureCurrent = gasInputPressure - gasColdTrapPressureLoss;
    //ограничение минимального давления
    if (gasOutputPressureCurrent < minPressure){
        gasOutputPressureCurrent = minPressure;
    }
    //текущая температура ловушки
    if (flagColdTrapLeak == 0 && !isSituationActive("coldTrapLeak")){
    gasTemperatureCurrent = (1 - coldTrapTempCoef)*(gasInputTemperature - coldTrapTemperature) + coldTrapTemperature;
    //текущая объемная доля частиц в газе
    }else if(flagColdTrapLeak == 0 && isSituationActive("coldTrapLeak")){
    gasTemperatureCurrent = gasInputTemperature;
    }
    gasParticleFractionCurrent = gasInputParticleFraction - gasInputParticleFraction * coldTrapParticleCatchCoef * fillCoef;
    if (gasParticleFractionCurrent < 0){
        gasParticleFractionCurrent = 0;
    }
    //текущий расход газа
    gasVolumeFlowRateCurrent = gasInputVolumeFlowRate * (1 - gasInputParticleFraction * coldTrapParticleCatchCoef * fillCoef);


    //запоминание параметров на предыдущей итерации
    coldTrapResistantPrev = coldTrapResistantCurrent;
    particleMassPrev = particleMassCurrent;
    gasPressurePrev = gasOutputPressureCurrent;
    gasFlowRatePrev = gasVolumeFlowRateCurrent;
    gasTemperaturePrev = gasTemperatureCurrent;
    gasParticleFractionPrev = gasParticleFractionCurrent;
    gasActivityPrev = gasActivityCurrent;
    coldTrapActivityPrev = coldTrapActivityCurrent;
    //запоминание параметров аварий
    situationColdTrapLeakPrev = isSituationActive("coldTrapLeak");
    situationColdTrapClogPrev = isSituationActive("coldTrapClog");
    outGasColdTrap->setOut(0, gasVolumeFlowRateCurrent);
    outGasColdTrap->setOut(1, gasOutputPressureCurrent);
    outGasColdTrap->setOut(2, gasTemperatureCurrent);
    outGasColdTrap->setOut(3, gasActivityCurrent);
    outGasColdTrap->setOut(4, gasParticleFractionCurrent);
    outColdTrapParameters->setOut(0, coldTrapActivityCurrent);
    outColdTrapParameters->setOut(1, particleMassCurrent);
    //выходное сопротивление
    outFeedback->setOut(0, outColdTrapResistance);


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
