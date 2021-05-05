#include "GCS_SensorTemperature.h"

GCS_SensorTemperature::GCS_SensorTemperature()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
    createParameter("physicalUnits", "0");
    createParameter("minimumValue", "-273");
    createParameter("maximumValue", "1000");
    createParameter("divisionValue", "0.1");

	// Сигналы блока
    temperatureValue = createSignal("temperatureValue", Signal::ST_DOUBLE);
    sensorTemperatureUnit = createSignal("sensorTemperatureUnit", Signal::ST_DOUBLE);


	// Порты блока
    inPort = createInputPort(0, "UNKNOWN_NAME", "INFO");
    outPort = createOutputPort(1, "UNKNOWN_NAME", "INFO");

	// Отказы блока

}

void GCS_SensorTemperature::setDataNames()
{
    std::vector<std::string> dn;
    dn.push_back("Показание датчика температуры");
    outPort->setDataNames(dn);
}

bool GCS_SensorTemperature::init(std::string &error, double h)
{
    // Put your initialization here
    //проверка корректности параметров
    if (paramToDouble("physicalUnits") < 0 || paramToDouble("physicalUnits") > 1){
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
    sensorTemperatureUnit->Value.intVal = 0;
	setDataNames();
    return true;
}

bool GCS_SensorTemperature::process(double t, double h, std::string &error)
{
    // Put your calculations here
    double gasInputTemperature, minimumValue, maximumValue, divisionValue;
    int physicalUnits, wholePartNumber;
    gasInputTemperature = inPort->getInput()[2]; //температура газа, град. Цельсия

    physicalUnits = paramToInt("physicalUnits");
    minimumValue = paramToDouble("minimumValue");
    maximumValue = paramToDouble("maximumValue");
    divisionValue = paramToDouble("divisionValue");
    //проверки на пределы измеряемого значения и его вывод
    if (gasInputTemperature > maximumValue){
        gasInputTemperature = maximumValue;
    }else if (gasInputTemperature < minimumValue){
        gasInputTemperature = minimumValue;
    }else{
        //вывод необходимых единиц измерения
        switch(physicalUnits){
        case 0:
            //выбраны м3/с
            sensorTemperatureUnit->Value.intVal = 0;
            //приведение значение в соответствии с ценой деления
            wholePartNumber = gasInputTemperature / divisionValue;
            gasInputTemperature = wholePartNumber * divisionValue;
            break;
        case 1:
            //выбраны л/с
            sensorTemperatureUnit->Value.intVal = 1;
            gasInputTemperature = gasInputTemperature + 273.15;
            //приведение значение в соответствии с ценой деления
            wholePartNumber = gasInputTemperature / divisionValue;
            gasInputTemperature = wholePartNumber * divisionValue;
            break;
        default:
            //выбраны Па по умолчанию
            sensorTemperatureUnit->Value.intVal = 0;
            //приведение значение в соответствии с ценой деления
            wholePartNumber = gasInputTemperature / divisionValue;
            gasInputTemperature = wholePartNumber * divisionValue;
            break;
        }
}
    //передача значения давления на шкалу датчика
    outPort->setOut(0, gasInputTemperature);
    temperatureValue->Value.doubleVal = gasInputTemperature;
    return true;
}


ICalcElement *Create()
{
    return new GCS_SensorTemperature();
}

void Release(ICalcElement *block)
{
    if(block->type() == "GCS_SensorTemperature") delete block;
}

std::string Type()
{
    return "GCS_SensorTemperature";
}
