#include "GCS_ControlValve.h"
#include "../constants_list.h"
#include <cstdlib>
#include <ctime>

GCS_ControlValve::GCS_ControlValve()
{
	// Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
	createParameter("valveTimeOpen", "1");
	createParameter("valveTransferCoef", "1");
	createParameter("valveInitPos", "0");

	// Сигналы блока
    valvePositionBar = createSignal("valvePositionBar", Signal::ST_DOUBLE);

	// Порты блока
    inGasValve=createInputPort(0, "UNKNOWN_NAME", "INFO");
    outGasValve=createOutputPort(1, "UNKNOWN_NAME", "INFO");
    inGivenPosition=createInputPort(2, "UNKNOWN_NAME", "INFO");
    outPosition=createOutputPort(3, "UNKNOWN_NAME", "INFO");
    inFeedbackGasValve=createInputPort(4, "UNKNOWN_NAME", "INFO");
    outFeedbackGasValve=createOutputPort(5, "UNKNOWN_NAME", "INFO");

    // Отказы блока
    createSituation("valveRandom");
    createSituation("valveIgnore");

}

double GCS_ControlValve::signumFunc(double argVal)
{
    if (argVal > 0.0) return 1.0;
    if (argVal < 0.0) return -1.0;
    return 0;
}

void GCS_ControlValve::setDataNames()
{
    //выходные параметры газа
    std::vector<std::string> outGasValveName;
    outGasValveName.push_back("Объемный расход, м^3/с");
    outGasValveName.push_back("Давление, Па");
    outGasValveName.push_back("Температура, °C");
    outGasValveName.push_back("Объемная активность газа, Бк/м^3");
    outGasValveName.push_back("Объемная доля частиц в газе, отн. ед.");
    outGasValve->setDataNames(outGasValveName);
    //выходные параметры клапана
    std::vector<std::string> outPositionName;
    outPositionName.push_back("Текущее положение клапана (от 0 до 1)");
    outPosition->setDataNames(outPositionName);
    //выходные параметры клапана
    std::vector<std::string> outFeedbackGasValveName;
    outFeedbackGasValveName.push_back("Общий коэффициент сопротивления");
    outFeedbackGasValve->setDataNames(outFeedbackGasValveName);
}

bool GCS_ControlValve::init(std::string &error, double h)
{
    // Put your initialization here
    setDataNames();
    //проверка корректности параметров
    for (int i = 0; i < (int)Parameters.size(); i++){
        if (paramToDouble(Parameters[i])<0){
            error = "Ошибка в заполнении исходных данных!";
            return false;
        }
    }
    //проверка правильности начального значения положения клапана (от 0 до 1)
    if (paramToDouble("valveInitPos") < 0 || paramToDouble("valveInitPos") > 1){
        error = "Ошибка в значении начального положения клапана!\nОно должно быть в диапазоне от 0 до 1.";
        return false;
    }
    valvePositionPrev = paramToDouble("valveInitPos");

    valvePositionCurrent = paramToDouble("valveInitPos");
    gasVolumeFlowRateCurrent=0;
    gasOutputPressureCurrent=0;
    gasTemperatureCurrent=0;
    gasActivityCurrent=0;
    gasParticleFractionCurrent=0;
    outValvesResistance=0;
    valvePositionCoef = 0;
    valvePositionGiven = paramToDouble("valveInitPos");
    situationValveIgnorePrev = 0;
    situationValveRandomPrev = 0;


    outPosition->setOut(0,valvePositionCurrent);
    outGasValve->setOut(0, gasVolumeFlowRateCurrent);
    outGasValve->setOut(1, gasOutputPressureCurrent);
    outGasValve->setOut(2, gasTemperatureCurrent);
    outGasValve->setOut(3, gasActivityCurrent);
    outGasValve->setOut(4, gasParticleFractionCurrent);

    outFeedbackGasValve->setOut(0, outValvesResistance);

    return true;
}

bool GCS_ControlValve::process(double t, double h, std::string &error)
{
    // Put your calculations here
    double valveTimeOpen, valveTransferCoef, valvePositionDerivative,
            gasInputVolumeFlowRate, gasInputPressure, gasInputTemperature, gasInputActivity, gasInputParticleFraction,signArg,
               inValvesResistance;


    valveTimeOpen = paramToDouble("valveTimeOpen"); //время открытия клапана, с
    valveTransferCoef = paramToDouble("valveTransferCoef"); //передаточный коэффициент клапана
    //считывание значений на входе
    gasInputVolumeFlowRate = inGasValve->getInput()[0]; //объемный расход газа, м^3/с
    gasInputPressure = inGasValve->getInput()[1];//давление газа, Па
    gasInputTemperature = inGasValve->getInput()[2];//температура газа, град. Цел.
    gasInputActivity = inGasValve->getInput()[3];//активность газа, Бк
    gasInputParticleFraction = inGasValve->getInput()[4];//объемная доля частиц в газе, отн. ед.
    if (!isSituationActive("valveIgnore")){
        valvePositionGiven = inGivenPosition->getInput()[0]; //заданная позиция клапана (от 0 до 1)
    }
    inValvesResistance = inFeedbackGasValve->getInput()[0]; //общий коэффициент сопротивления от вентилей на линии (от 0 до 1)
    //проверка заданного положения клапана
    if (valvePositionGiven < 0 || valvePositionGiven > 1){
        error = "Ошибка в значении заданного положения клапана!\nОно должно быть в диапазоне от 0 до 1.";
        return false;
    }
 //если авария сбоя задающего устройства, то при каждом переключении будет изменяться +-0.4 задающ. воздействие
    if (isSituationActive("valveRandom") && (valvePositionGivenPrev != valvePositionGiven)){
        srand(time(0));
        valvePositionCoef = (double)(1 + rand() % 17 - 9) / 20;
    }

    //проверка на нажатие/отжатие кнопки аварии (утечка)
    if (situationValveRandomPrev != isSituationActive("valveRandom")){
        if (isSituationActive("valveRandom") > situationValveRandomPrev){
            //генерация рандомный значений положения клапана+-0.4 от заданного положения
            srand(time(0));
            valvePositionCoef = (double)(1 + rand() % 17 - 9) / 20;
        }else if (isSituationActive("valveRandom") < situationValveRandomPrev){
            valvePositionCoef = 0;
        }
    }

    valvePositionGivenPrev = valvePositionGiven;
    valvePositionGiven = valvePositionGiven + valvePositionCoef;
    if (valvePositionGiven < 0){
        valvePositionGiven = 0;
    }else if (valvePositionGiven > 1){
        valvePositionGiven = 1;
    }

    signArg = valvePositionGiven-valvePositionPrev; //направление движения (открывается или закрывается клапан)
    //решение уравнения клапана методом Эйлера
    valvePositionDerivative = valveTransferCoef * signumFunc(signArg) / valveTimeOpen;
    valvePositionCurrent = valvePositionPrev + valvePositionDerivative * h;
    if (valvePositionCurrent < 0){
        valvePositionCurrent = 0;
    }else if (valvePositionCurrent > 1){
        valvePositionCurrent = 1;
    }
    //условие для устранения статической ошибки из-за signumFunc
    if (valvePositionCurrent > valvePositionPrev && valvePositionCurrent > valvePositionGiven){
        valvePositionCurrent = valvePositionGiven;
    }else if (valvePositionCurrent < valvePositionPrev && valvePositionCurrent < valvePositionGiven){
        valvePositionCurrent = valvePositionGiven;
    }
    gasVolumeFlowRateCurrent = gasInputVolumeFlowRate; //расход изменится на след. итерации с помощью блока насоса
    gasOutputPressureCurrent = gasInputPressure * valvePositionCurrent;//текущее давление на выходе фильтра, Па
    //ограничение минимального давления
    if (gasOutputPressureCurrent < minPressure){
        gasOutputPressureCurrent = minPressure;
    }
    gasTemperatureCurrent = gasInputTemperature; //текущая температура газа на выходе
    gasActivityCurrent = gasInputActivity; //текущая активность газа на выходе
    gasParticleFractionCurrent = gasInputParticleFraction; //текущая объемная доля частиц на выходе

    //рассчет общего сопротивленя клапанов
    outValvesResistance = 1 - ((1 - inValvesResistance) * valvePositionCurrent);
    if (outValvesResistance > 1){
        outValvesResistance = 1;
    }
    situationValveRandomPrev = isSituationActive("valveRandom");
    situationValveIgnorePrev = isSituationActive("valveIgnore");
    //передача значений на выходные порты
    outPosition->setNewOut(0, valvePositionCurrent);

    valvePositionPrev = valvePositionCurrent;
    outGasValve->setNewOut(0, gasVolumeFlowRateCurrent);
    outGasValve->setNewOut(1, gasOutputPressureCurrent);
    outGasValve->setNewOut(2, gasTemperatureCurrent);
    outGasValve->setNewOut(3, gasActivityCurrent);
    outGasValve->setNewOut(4, gasParticleFractionCurrent);
    outFeedbackGasValve->setNewOut(0, outValvesResistance);
    //передача значения положения клапана на шкалу
    valvePositionBar->Value.doubleVal = valvePositionCurrent * 100;
    return true;
}


ICalcElement *Create()
{
    return new GCS_ControlValve();
}

void Release(ICalcElement *block)
{
    if(block->type() == "GCS_ControlValve") delete block;
}

std::string Type()
{
    return "GCS_ControlValve";
}
