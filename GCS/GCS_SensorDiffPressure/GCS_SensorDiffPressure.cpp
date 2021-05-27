#include "GCS_SensorDiffPressure.h"
#include <cstdlib>
#include <ctime>

GCS_SensorDiffPressure::GCS_SensorDiffPressure()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
    createParameter("physicalUnits", "0");
    createParameter("minimumValue", "-1000000");
    createParameter("maximumValue", "1000000");
    createParameter("divisionValue", "0.1");

	// Сигналы блока
    pressureValue = createSignal("pressureValue", Signal::ST_DOUBLE);
    sensorPressureUnit = createSignal("sensorPressureUnit", Signal::ST_DOUBLE);


	// Порты блока
    inPort1 = createInputPort(0, "UNKNOWN_NAME", "INFO");
    inPort2 = createInputPort(1, "UNKNOWN_NAME", "INFO");
    outPort = createOutputPort(2, "UNKNOWN_NAME", "INFO");

    // Отказы блока
    createSituation("valueRandom");
    createSituation("valueIgnore");
}

void GCS_SensorDiffPressure::setDataNames()
{
    std::vector<std::string> dn;
    dn.push_back("Показание датчика диф. давления");
    outPort->setDataNames(dn);
}

bool GCS_SensorDiffPressure::init(std::string &error, double h)
{
    // Put your initialization here
    //проверка корректности параметров
    if (paramToDouble("physicalUnits") < 0 || paramToDouble("physicalUnits") > 2){
        error = "Ошибка в значении параметра Единицы измерения";
        return false;
    }
    if ((paramToDouble("maximumValue") - paramToDouble("minimumValue")) < paramToDouble("divisionValue")){
        error = "Ошибка! Цена должна быть меньше диапазона измерений";
        return false;
    }
    if ((paramToDouble("maximumValue") - paramToDouble("minimumValue")) < 0){
        error = "Ошибка в соотношении максимального и миминмального значений шкалы";
        return false;
    }
    if (paramToDouble("divisionValue") < 0){
        error = "Ошибка в значении цены деления";
        return false;
    }
    //передача единиц измерения на датчик
    sensorPressureUnit->Value.intVal = 0;
    gasDiffPressure = 0;
    valueCoef = 0;
    situationValueRandomPrev = 0;
	setDataNames();
    return true;
}

bool GCS_SensorDiffPressure::process(double t, double h, std::string &error)
{
    // Put your calculations here
    double gasInputPressure1, gasInputPressure2, minimumValue, maximumValue, divisionValue;
    int physicalUnits, wholePartNumber;
    gasInputPressure1 = inPort1->getInput()[1]; //давление газа, Па
    gasInputPressure2 = inPort2->getInput()[1]; //давление газа, Па

    physicalUnits = paramToInt("physicalUnits");
    minimumValue = paramToDouble("minimumValue");
    maximumValue = paramToDouble("maximumValue");
    divisionValue = paramToDouble("divisionValue");
    //вычисление разности давалений
    if (!isSituationActive("valueIgnore")){
        gasDiffPressure = gasInputPressure1 - gasInputPressure2;
    }

    //проверка на нажатие/отжатие кнопки аварии (утечка)
    if (situationValueRandomPrev != isSituationActive("valueRandom")){
        if (isSituationActive("valueRandom") > situationValueRandomPrev){
            //генерация рандомный значений положения клапана+-0.4 от заданного положения
            srand(time(0));
            valueCoef = (double)(1 + rand() % 17 - 9) / 20 * gasDiffPressure;
        }else if (isSituationActive("valueRandom") < situationValueRandomPrev){
            valueCoef = 0;
        }
    }
    gasDiffPressure = gasDiffPressure + valueCoef;

    //вывод необходимых единиц измерения
    switch(physicalUnits){
    case 0:
        //выбраны Па
        sensorPressureUnit->Value.intVal = 0;
        //приведение значение в соответствии с ценой деления
        wholePartNumber = gasDiffPressure / divisionValue;
        gasDiffPressure = wholePartNumber * divisionValue;
        break;
    case 1:
        //выбраны бар
        sensorPressureUnit->Value.intVal = 1;
        gasDiffPressure = gasDiffPressure / 100000;
        //приведение значение в соответствии с ценой деления
        wholePartNumber = gasDiffPressure / divisionValue;
        gasDiffPressure = wholePartNumber * divisionValue;
        break;
    case 2:
        //выбраны атм
        sensorPressureUnit->Value.intVal = 2;
        gasDiffPressure = gasDiffPressure / 101325;
        //приведение значение в соответствии с ценой деления
        wholePartNumber = gasDiffPressure / divisionValue;
        gasDiffPressure = wholePartNumber * divisionValue;
        break;
    default:
        //выбраны Па по умолчанию
        sensorPressureUnit->Value.intVal = 0;
        //приведение значение в соответствии с ценой деления
        wholePartNumber = gasDiffPressure / divisionValue;
        gasDiffPressure = wholePartNumber * divisionValue;
        break;

    //проверки на пределы измеряемого значения и его вывод
    if (gasDiffPressure > maximumValue){
        gasDiffPressure = maximumValue;
    }else if (gasDiffPressure < minimumValue){
        gasDiffPressure = minimumValue;
    }

}
    situationValueRandomPrev = isSituationActive("valueRandom");
    //передача значения давления на шкалу датчика
    pressureValue->Value.doubleVal = gasDiffPressure;
    outPort->setOut(0, gasDiffPressure);

    return true;
}


ICalcElement *Create()
{
    return new GCS_SensorDiffPressure();
}

void Release(ICalcElement *block)
{
    if(block->type() == "GCS_SensorDiffPressure") delete block;
}

std::string Type()
{
    return "GCS_SensorDiffPressure";
}
