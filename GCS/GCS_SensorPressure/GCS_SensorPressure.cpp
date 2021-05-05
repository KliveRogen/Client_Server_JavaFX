#include "GCS_SensorPressure.h"

GCS_SensorPressure::GCS_SensorPressure()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
    createParameter("physicalUnits", "0");
    createParameter("minimumValue", "3000");
    createParameter("maximumValue", "1000000");
    createParameter("divisionValue", "0.1");

	// Сигналы блока
    pressureValue = createSignal("pressureValue", Signal::ST_DOUBLE);
    sensorPressureUnit = createSignal("sensorPressureUnit", Signal::ST_DOUBLE);


	// Порты блока
    inPort = createInputPort(0, "UNKNOWN_NAME", "INFO");
    outPort = createOutputPort(1, "UNKNOWN_NAME", "INFO");

	// Отказы блока

}

void GCS_SensorPressure::setDataNames()
{
    std::vector<std::string> dn;
    dn.push_back("Показание датчика давления");
    outPort->setDataNames(dn);
}

bool GCS_SensorPressure::init(std::string &error, double h)
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
    for (int i = 0; i < (int)Parameters.size(); i++){
        if (paramToDouble(Parameters[i])<0){
            error = "Ошибка в заполнении исходных данных!";
            return false;
        }
    }
    //передача единиц измерения на датчик
    sensorPressureUnit->Value.intVal = 0;
	setDataNames();
    return true;
}

bool GCS_SensorPressure::process(double t, double h, std::string &error)
{
    // Put your calculations here
    double gasInputPressure, minimumValue, maximumValue, divisionValue;
    int physicalUnits, wholePartNumber;
    gasInputPressure = inPort->getInput()[1]; //давление газа, Па

    physicalUnits = paramToInt("physicalUnits");
    minimumValue = paramToDouble("minimumValue");
    maximumValue = paramToDouble("maximumValue");
    divisionValue = paramToDouble("divisionValue");
    //проверки на пределы измеряемого значения и его вывод
    if (gasInputPressure > maximumValue){
        gasInputPressure = maximumValue;
    }else if (gasInputPressure < minimumValue){
        gasInputPressure = minimumValue;
    }else{
        //вывод необходимых единиц измерения
        switch(physicalUnits){
        case 0:
            //выбраны Па
            sensorPressureUnit->Value.intVal = 0;
            //приведение значение в соответствии с ценой деления
            wholePartNumber = gasInputPressure / divisionValue;
            gasInputPressure = wholePartNumber * divisionValue;
            break;
        case 1:
            //выбраны бар
            sensorPressureUnit->Value.intVal = 1;
            gasInputPressure = gasInputPressure / 100000;
            //приведение значение в соответствии с ценой деления
            wholePartNumber = gasInputPressure / divisionValue;
            gasInputPressure = wholePartNumber * divisionValue;
            break;
        case 2:
            //выбраны атм
            sensorPressureUnit->Value.intVal = 2;
            gasInputPressure = gasInputPressure / 101325;
            //приведение значение в соответствии с ценой деления
            wholePartNumber = gasInputPressure / divisionValue;
            gasInputPressure = wholePartNumber * divisionValue;
            break;
        default:
            //выбраны Па по умолчанию
            sensorPressureUnit->Value.intVal = 0;
            //приведение значение в соответствии с ценой деления
            wholePartNumber = gasInputPressure / divisionValue;
            gasInputPressure = wholePartNumber * divisionValue;
            break;
        }
}
    //передача значения давления на шкалу датчика
    outPort->setOut(0, gasInputPressure);
    pressureValue->Value.doubleVal = gasInputPressure;
    return true;
}


ICalcElement *Create()
{
    return new GCS_SensorPressure();
}

void Release(ICalcElement *block)
{
    if(block->type() == "GCS_SensorPressure") delete block;
}

std::string Type()
{
    return "GCS_SensorPressure";
}
