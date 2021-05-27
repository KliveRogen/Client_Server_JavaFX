#include "GCS_Pipe.h"
#include <cmath>
#include "../constants_list.h"

GCS_Pipe::GCS_Pipe()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
    createParameter("pipeDiameter", "0.05");
    createParameter("frictionCoef", "1");
    createParameter("gasDensity", "1");
    createParameter("pipeLength", "10");
    createParameter("timeClog", "50");

	// Сигналы блока

	// Порты блока
    inGasPipe = createInputPort(0, "UNKNOWN_NAME", "INFO");
    outGasPipe = createOutputPort(1, "UNKNOWN_NAME", "INFO");

    inFeedback = createInputPort(2, "UNKNOWN_NAME", "INFO"); // учет коэф. сопротвиления сопротивлений
    outFeedback = createOutputPort(3, "UNKNOWN_NAME", "INFO");
	// Отказы блока
    createSituation("pipeClog");
}

double GCS_Pipe::signumFunc(double argVal)
{
    if (argVal > 0.0) return 1.0;
    if (argVal < 0.0) return -1.0;
    return 0;
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
    //выход обратной связи
    std::vector<std::string> outFeedbackName;
    outFeedbackName.push_back("Общий коэффициент сопротивления");
    outFeedback->setDataNames(outFeedbackName);
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
    situationClogPrev = 0;
    flagClog = 0;
    resistantPrev = 0;
    outResistance = 0;
    resistantCurrent = 0;
    timeCounterClog = 0;
	setDataNames();
    return true;
}

bool GCS_Pipe::process(double t, double h, std::string &error)
{
    // Put your calculations here
    double gasInputVolumeFlowRate, gasInputPressure, gasInputTemperature, gasInputActivity, gasInputParticleFraction, gasOutputPressureCurrent, gasPressureLoss,
            gasVolumeFlowRateCurrent, gasTemperatureCurrent, gasActivityCurrent, gasParticleFractionCurrent, pipeDiameter, frictionCoef, pipeLength;
    double  timeClog, inResistance, signArg, resistantDerivative;
    //считывание значений на входе
    gasInputVolumeFlowRate = inGasPipe->getInput()[0];//объемный расход газа, м^3/с
    gasInputPressure = inGasPipe->getInput()[1];//давление газа, Па
    gasInputTemperature = inGasPipe->getInput()[2];//температура газа, град. Цел.
    gasInputActivity = inGasPipe->getInput()[3];//активность газа, Бк
    gasInputParticleFraction = inGasPipe->getInput()[4];//объемная доля частиц в газе, отн. ед.
    inResistance = inFeedback->getInput()[0]; //общий коэффициент сопротивления (от 0 до 1)
    //параметры трубы
    pipeDiameter = paramToDouble("pipeDiameter"); //диаметр трубы, м
    frictionCoef = paramToDouble("frictionCoef"); //коэффициент трения
    pipeLength = paramToDouble("pipeLength"); //длина трубы, м
    //время аварии
    timeClog = paramToDouble("timeClog");

    //проверка на нажатие/отжатие кнопки аварии
    if (situationClogPrev != isSituationActive("pipeClog")){
        if (isSituationActive("pipeClog") > situationClogPrev){
            flagClog = 1;
        }else if (isSituationActive("pipeClog") < situationClogPrev){
            flagClog = -1;
        }
    }
    //проверка на состояние флага аварии для реализации инерции
    if (flagClog == 1){
        timeCounterClog = timeCounterClog + h;
        if (timeCounterClog > timeClog + 5){
            flagClog = 0;
            timeCounterClog = 0;
        }

        signArg = 1-resistantPrev; //направление движения (открывается или закрывается клапан)
        //решение уравнения клапана методом Эйлера
        resistantDerivative = signumFunc(signArg) / timeClog;
        resistantCurrent = resistantPrev + resistantDerivative * h;
        if (resistantCurrent < 0){
            resistantCurrent = 0;
        }else if (resistantCurrent > 1){
            resistantCurrent = 1;
        }
        //условие для устранения статической ошибки из-за signumFunc
        if (resistantCurrent > resistantPrev && resistantCurrent > 1){
            resistantCurrent = 1;
        }else if (resistantCurrent < resistantPrev && resistantCurrent < 1){
            resistantCurrent = 1;
        }
    }else if (flagClog == -1){
        timeCounterClog = timeCounterClog + h;
        if (timeCounterClog > timeClog + 5){
            flagClog = 0;
            timeCounterClog = 0;
        }

        signArg = 0-resistantPrev; //направление движения (открывается или закрывается клапан)
        //решение уравнения клапана методом Эйлера
        resistantDerivative = signumFunc(signArg) / timeClog;
        resistantCurrent = resistantPrev + resistantDerivative * h;
        if (resistantCurrent < 0){
            resistantCurrent = 0;
        }else if (resistantCurrent > 1){
            resistantCurrent = 1;
        }
        //условие для устранения статической ошибки из-за signumFunc
        if (resistantCurrent > resistantPrev && resistantCurrent > 0){
            resistantCurrent = 0;
        }else if (resistantCurrent < resistantPrev && resistantCurrent < 0){
            resistantCurrent = 0;
        }
    }

    //расчет выходного сопротивления
    outResistance = 1 - ((1 - inResistance) * (1-resistantCurrent));
    if (outResistance > 1){
        outResistance = 1;
    }

    //рассчет выходных параметров
    gasPressureLoss = frictionCoef * pow(gasInputVolumeFlowRate, 2) * gasDensity * pipeLength / pow(pipeDiameter, 5);//перепад давления в трубе, Па
    gasOutputPressureCurrent = (gasInputPressure - gasPressureLoss;//текущее давление на выходе трубы, Па
    gasVolumeFlowRateCurrent = gasInputVolumeFlowRate;//текущий об. расх. газа на выходе трубы, м^3/с
    gasTemperatureCurrent = gasInputTemperature;//текущая температура газа на выходе трубы, град. Цел.
    gasActivityCurrent = gasInputActivity;//текущая температура на выходе трубы, град. Цел.
    gasParticleFractionCurrent = gasInputParticleFraction;//текущая доля частиц на выходе трубы
    //ограничение минимального давления
    if (gasOutputPressureCurrent < minPressure){
        gasOutputPressureCurrent = minPressure;
    }
    resistantPrev = resistantCurrent;
    situationClogPrev = isSituationActive("pipeClog");
    outGasPipe->setOut(0, gasVolumeFlowRateCurrent);
    outGasPipe->setOut(1, gasOutputPressureCurrent);
    outGasPipe->setOut(2, gasTemperatureCurrent);
    outGasPipe->setOut(3, gasActivityCurrent);
    outGasPipe->setOut(4, gasParticleFractionCurrent);
    //выходное сопротивление
    outFeedback->setOut(0, outResistance);
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
