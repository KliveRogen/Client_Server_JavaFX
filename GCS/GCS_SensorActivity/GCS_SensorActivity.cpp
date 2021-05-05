#include "GCS_SensorActivity.h"

GCS_SensorActivity::GCS_SensorActivity()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
    createParameter("physicalUnits", "0");
    createParameter("minimumValue", "0");
    createParameter("maximumValue", "1000000");
    createParameter("divisionValue", "0.1");

	// Сигналы блока
    activityValue = createSignal("activityValue", Signal::ST_DOUBLE);
    sensorActivityUnit = createSignal("sensorActivityUnit", Signal::ST_DOUBLE);


	// Порты блока
    inPort = createInputPort(0, "UNKNOWN_NAME", "INFO");
    outPort = createOutputPort(1, "UNKNOWN_NAME", "INFO");

	// Отказы блока

}

void GCS_SensorActivity::setDataNames()
{
    std::vector<std::string> dn;
    dn.push_back("Показание датчика об. активности");
    outPort->setDataNames(dn);
}

bool GCS_SensorActivity::init(std::string &error, double h)
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
    for (int i = 0; i < (int)Parameters.size(); i++){
        if (paramToDouble(Parameters[i])<0){
            error = "Ошибка в заполнении исходных данных!";
            return false;
        }
    }
    //передача единиц измерения на датчик
    sensorActivityUnit->Value.intVal = 0;
	setDataNames();
    return true;
}

bool GCS_SensorActivity::process(double t, double h, std::string &error)
{
    // Put your calculations here
    double gasInputActivity, minimumValue, maximumValue, divisionValue;
    int physicalUnits, wholePartNumber;
    gasInputActivity = inPort->getInput()[3]; //активность газа, Бк

    physicalUnits = paramToInt("physicalUnits");
    minimumValue = paramToDouble("minimumValue");
    maximumValue = paramToDouble("maximumValue");
    divisionValue = paramToDouble("divisionValue");
    //проверки на пределы измеряемого значения и его вывод
    if (gasInputActivity > maximumValue){
        gasInputActivity = maximumValue;
    }else if (gasInputActivity < minimumValue){
        gasInputActivity = minimumValue;
    }else{
        //вывод необходимых единиц измерения
        switch(physicalUnits){
        case 0:
            //выбраны Бк/м^3
            sensorActivityUnit->Value.intVal = 0;
            //приведение значение в соответствии с ценой деления
            wholePartNumber = gasInputActivity / divisionValue;
            gasInputActivity = wholePartNumber * divisionValue;
            break;
        case 1:
            //выбраны нКи/м^3
            sensorActivityUnit->Value.intVal = 1;
            gasInputActivity = gasInputActivity / 37;
            //приведение значение в соответствии с ценой деления
            wholePartNumber = gasInputActivity / divisionValue;
            gasInputActivity = wholePartNumber * divisionValue;
            break;
        default:
            //выбраны Бк/м^3 по умолчанию
            sensorActivityUnit->Value.intVal = 0;
            //приведение значение в соответствии с ценой деления
            wholePartNumber = gasInputActivity / divisionValue;
            gasInputActivity = wholePartNumber * divisionValue;
            break;
        }
}
    //передача значения давления на шкалу датчика
    outPort->setOut(0, gasInputActivity);
    activityValue->Value.doubleVal = gasInputActivity;
    return true;
}


ICalcElement *Create()
{
    return new GCS_SensorActivity();
}

void Release(ICalcElement *block)
{
    if(block->type() == "GCS_SensorActivity") delete block;
}

std::string Type()
{
    return "GCS_SensorActivity";
}
