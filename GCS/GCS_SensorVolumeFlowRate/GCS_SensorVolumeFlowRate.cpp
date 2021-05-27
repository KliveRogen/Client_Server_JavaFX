#include "GCS_SensorVolumeFlowRate.h"
#include <cstdlib>
#include <ctime>


GCS_SensorVolumeFlowRate::GCS_SensorVolumeFlowRate()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
    createParameter("physicalUnits", "0");
    createParameter("minimumValue", "3000");
    createParameter("maximumValue", "1000000");
    createParameter("divisionValue", "0.1");

	// Сигналы блока
    flowRateValue = createSignal("flowRateValue", Signal::ST_DOUBLE);
    sensorFlowRateUnit = createSignal("sensorFlowRateUnit", Signal::ST_DOUBLE);


	// Порты блока
    inPort = createInputPort(0, "UNKNOWN_NAME", "INFO");
    outPort = createOutputPort(1, "UNKNOWN_NAME", "INFO");

    createSituation("valueRandom");
    createSituation("valueIgnore");

	// Отказы блока

}

void GCS_SensorVolumeFlowRate::setDataNames()
{
    std::vector<std::string> dn;
    dn.push_back("Показание датчика расхода");
    outPort->setDataNames(dn);
}

bool GCS_SensorVolumeFlowRate::init(std::string &error, double h)
{
    // Put your initialization here
    //проверка корректности параметров
    if (paramToDouble("physicalUnits") < 0 || paramToDouble("physicalUnits") > 3){
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
    for (int i = 0; i < (int)Parameters.size(); i++){
        if (paramToDouble(Parameters[i])<0){
            error = "Ошибка в заполнении исходных данных!";
            return false;
        }
    }
    //передача единиц измерения на датчик
    sensorFlowRateUnit->Value.intVal = 0;
    gasInputFlowRate = 0;
    valueCoef = 0;
    situationValueRandomPrev = 0;
	setDataNames();
    return true;
}

bool GCS_SensorVolumeFlowRate::process(double t, double h, std::string &error)
{
    // Put your calculations here
    double minimumValue, maximumValue, divisionValue;
    int physicalUnits, wholePartNumber;

    if (!isSituationActive("valueIgnore")){
        gasInputFlowRate = inPort->getInput()[0]; //расход газа, м3/с
    }
    physicalUnits = paramToInt("physicalUnits");
    minimumValue = paramToDouble("minimumValue");
    maximumValue = paramToDouble("maximumValue");
    divisionValue = paramToDouble("divisionValue");

    //проверка на нажатие/отжатие кнопки аварии (утечка)
    if (situationValueRandomPrev != isSituationActive("valueRandom")){
        if (isSituationActive("valueRandom") > situationValueRandomPrev){
            //генерация рандомный значений положения клапана+-0.4 от заданного положения
            srand(time(0));
            valueCoef = (double)(1 + rand() % 17 - 9) / 20 * gasInputFlowRate;
        }else if (isSituationActive("valueRandom") < situationValueRandomPrev){
            valueCoef = 0;
        }
    }
    gasInputFlowRate = gasInputFlowRate + valueCoef;
    //проверки на пределы измеряемого значения и его вывод

    //вывод необходимых единиц измерения
    switch(physicalUnits){
    case 0:
        //выбраны м3/с
        sensorFlowRateUnit->Value.intVal = 0;
        //приведение значение в соответствии с ценой деления
        wholePartNumber = gasInputFlowRate / divisionValue;
        gasInputFlowRate = wholePartNumber * divisionValue;
        break;
    case 1:
        //выбраны л/с
        sensorFlowRateUnit->Value.intVal = 1;
        gasInputFlowRate = gasInputFlowRate * 1000;
        //приведение значение в соответствии с ценой деления
        wholePartNumber = gasInputFlowRate / divisionValue;
        gasInputFlowRate = wholePartNumber * divisionValue;
        break;
    case 2:
        //выбраны м3/ч
        sensorFlowRateUnit->Value.intVal = 2;
        gasInputFlowRate = gasInputFlowRate * 3600;
        //приведение значение в соответствии с ценой деления
        wholePartNumber = gasInputFlowRate / divisionValue;
        gasInputFlowRate = wholePartNumber * divisionValue;
        break;
    case 3:
        //выбраны л/ч
        sensorFlowRateUnit->Value.intVal = 3;
        gasInputFlowRate = gasInputFlowRate * 3600 * 1000;
        //приведение значение в соответствии с ценой деления
        wholePartNumber = gasInputFlowRate / divisionValue;
        gasInputFlowRate = wholePartNumber * divisionValue;
        break;
    default:
        //выбраны Па по умолчанию
        sensorFlowRateUnit->Value.intVal = 0;
        //приведение значение в соответствии с ценой деления
        wholePartNumber = gasInputFlowRate / divisionValue;
        gasInputFlowRate = wholePartNumber * divisionValue;
        break;
    }

    if (gasInputFlowRate > maximumValue){
        gasInputFlowRate = maximumValue;
    }else if (gasInputFlowRate < minimumValue){
        gasInputFlowRate = minimumValue;
    }

    situationValueRandomPrev = isSituationActive("valueRandom");
    //передача значения давления на шкалу датчика
    outPort->setOut(0, gasInputFlowRate);
    flowRateValue->Value.doubleVal = gasInputFlowRate;
    return true;
}


ICalcElement *Create()
{
    return new GCS_SensorVolumeFlowRate();
}

void Release(ICalcElement *block)
{
    if(block->type() == "GCS_SensorVolumeFlowRate") delete block;
}

std::string Type()
{
    return "GCS_SensorVolumeFlowRate";
}
