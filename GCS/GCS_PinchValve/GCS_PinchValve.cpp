#include "GCS_PinchValve.h"
#include "../constants_list.h"

GCS_PinchValve::GCS_PinchValve()
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
	createSituation("calcDist");

}

double GCS_PinchValve::signumFunc(double argVal)
{
    if (argVal > 0.0) return 1.0;
    if (argVal < 0.0) return -1.0;
    return 0;
}


void GCS_PinchValve::setDataNames()
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

bool GCS_PinchValve::init(std::string &error, double h)
{
    // Put your initialization here
    setDataNames();
    //проверка корректности параметров
    for (int i = 0; i < (int)Parameters.size(); i++){
        if (paramToDouble(Parameters[i]) < 0){
            error = "Ошибка в заполнении исходных данных!";
            return false;
        }
    }
    //проверка правильности начального значения положения клапана (от 0 до 1)
    if (paramToDouble("valveInitPos") < 0 || paramToDouble("valveInitPos") > 1){
        error = "Ошибка в значении начального положения клапана!\nОно должно быть в диапазоне от 0 до 1.";
        return false;
    }
    //преобразование некорректно введеных условий
    if (paramToDouble("valveInitPos") < 0.5){
        valvePositionPrev = 0;
        valvePositionCurrent=0;
    }else{
        valvePositionPrev = 1;
        valvePositionCurrent=1;
    }
    gasVolumeFlowRateCurrent=0;
    gasOutputPressureCurrent=0;
    gasTemperatureCurrent=0;
    gasActivityCurrent=0;
    gasParticleFractionCurrent=0;
    outValvesResistance=0;

   outPosition->setOut(0,valvePositionCurrent);
   outGasValve->setOut(0, gasVolumeFlowRateCurrent);
   outGasValve->setOut(1, gasOutputPressureCurrent);
   outGasValve->setOut(2, gasTemperatureCurrent);
   outGasValve->setOut(3, gasActivityCurrent);
   outGasValve->setOut(4, gasParticleFractionCurrent);

   outFeedbackGasValve->setOut(0, outValvesResistance);

    return true;
}

bool GCS_PinchValve::process(double t, double h, std::string &error)
{
    // Put your calculations here
    double valvePositionGiven, valveTimeOpen, valveTransferCoef, valvePositionDerivative,
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
    valvePositionGiven = inGivenPosition->getInput()[0]; //заданная позиция клапана (0 или 1)
    inValvesResistance = inFeedbackGasValve->getInput()[0]; //общий коэффициент сопротивления от вентилей на линии (от 0 до 1)
    //проверка заданного положения клапана
    if (valvePositionGiven < 0 || valvePositionGiven > 1){
        error = "Ошибка в значении заданного положения клапана!\nОно должно быть в диапазоне от 0 до 1.";
        return false;
    }
    //преобразование некорректно введеных условий
    if (valvePositionGiven < 0.5){
        valvePositionGiven = 0;
    }else{
        valvePositionGiven = 1;
    }
    signArg = valvePositionGiven-valvePositionPrev; //направление движения (открывается или закрывается клапан)
    //решение уравнения клапана методом Эйлера
    valvePositionDerivative = valveTransferCoef * signumFunc(signArg) / valveTimeOpen;
    valvePositionCurrent = valvePositionPrev + valvePositionDerivative * h;
    //условие для устранения статической ошибки из-за signumFunc
    if (valvePositionCurrent > valvePositionPrev && valvePositionCurrent > valvePositionGiven){
        valvePositionCurrent = valvePositionGiven;
    }else if (valvePositionCurrent < valvePositionPrev && valvePositionCurrent < valvePositionGiven){
        valvePositionCurrent = valvePositionGiven;
    }
    gasVolumeFlowRateCurrent = gasInputVolumeFlowRate * valvePositionCurrent;
    gasOutputPressureCurrent = gasInputPressure * valvePositionCurrent;//текущее давление на выходе фильтра, Па
    //ограничение минимального давления
    if (gasOutputPressureCurrent < minPressure){
        gasOutputPressureCurrent = minPressure;
    }
    gasTemperatureCurrent = gasInputTemperature;//текущая температура газа на выходе
    gasActivityCurrent = gasInputActivity;//текущая активность газа на выходе
    gasParticleFractionCurrent = gasInputParticleFraction;//текущая объемная доля частиц на выходе

    //рассчет общего сопротивленя клапанов
    outValvesResistance = 1 - ((1 - inValvesResistance) * valvePositionCurrent);
    if (outValvesResistance > 1){
        outValvesResistance = 1;
    }
    //передача значений на выходные порты
    outPosition->setNewOut(0,valvePositionCurrent);
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
    return new GCS_PinchValve();
}

void Release(ICalcElement *block)
{
    if(block->type() == "GCS_PinchValve") delete block;
}

std::string Type()
{
    return "GCS_PinchValve";
}
