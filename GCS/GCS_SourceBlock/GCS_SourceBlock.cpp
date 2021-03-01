#include "GCS_SourceBlock.h"

GCS_SourceBlock::GCS_SourceBlock()
{
	// Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
	createParameter("gasFlowRate", "0.026");
	createParameter("gasPressure", "100000");
	createParameter("gasTemperature", "500");
	createParameter("gasActivity", "10000");
    createParameter("particleFraction", "0.005");
    createParameter("pumpMaxPressure", "300000");

	// Сигналы блока

	// Порты блока
    out = createOutputPort(0, "UNKNOWN_NAME", "INFO");
    in = createInputPort(1, "UNKNOWN_NAME", "INFO");

	// Отказы блока

}

void GCS_SourceBlock::setDataNames()
{
    std::vector<std::string> dn;
    dn.push_back("Объемный расход, м^3/с");
    dn.push_back("Давление, Па");
    dn.push_back("Температура, °C");
    dn.push_back("Объемная активность газа, Бк/м^3");
    dn.push_back("Объемная доля частиц в газе, отн. ед.");
    out->setDataNames(dn);
}

bool GCS_SourceBlock::init(std::string &error, double h)
{
    // Put your initialization here
    // Put your calculations here
    setDataNames();
    //Проверка на соответствие давлений
    if ((paramToDouble("pumpMaxPressure") < paramToDouble("gasPressure"))
            || (paramToDouble("pumpMaxPressure") == paramToDouble("gasPressure") && paramToDouble("gasFlowRate") != 0)){
        error = "Давления насоса имеют неправильное соотношение";
        return false;
    }
    //вывод параметров на выходной порт и проверка
    for (int i = 0; i < ((int)Parameters.size()-1); i++){
        if (paramToDouble(Parameters[i])<0){
            error = "Ошибка в заполнении исходных данных!";
            return false;
        }
        out->setOut(i,paramToDouble(Parameters[i]));
    }

    return true;
}

bool GCS_SourceBlock::process(double t, double h, std::string &error)
{
    // Put your calculations here
    //Проверка на соответствие давлений
    if ((paramToDouble("pumpMaxPressure") < paramToDouble("gasPressure"))
            || (paramToDouble("pumpMaxPressure") == paramToDouble("gasPressure") && paramToDouble("gasFlowRate") != 0)){
        error = "Давления насоса имеют неправильное соотношение";
        return false;
    }
    //вывод параметров на выходной порт и проверка
    for (int i = 0; i < ((int)Parameters.size()); i++){
        if (paramToDouble(Parameters[i])<0){
            error = "Ошибка в заполнении исходных данных!";
            return false;
        }  
    }

    double inValvesResistance, characteristicSlope;
    inValvesResistance = in->getInput()[0]; //общее сопротивление от клапанов
    if (inValvesResistance>1){
        inValvesResistance=1;
    }

    characteristicSlope = (paramToDouble("gasPressure")-paramToDouble("pumpMaxPressure"))/paramToDouble("gasFlowRate"); //вычисление коэффциента наклона кривой напроно-расходной характеристики
    outGasFlowRate=(1-inValvesResistance)*paramToDouble("gasFlowRate");//объемный расход через насос,м^3/c
    outGasPressure=outGasFlowRate*characteristicSlope+paramToDouble("pumpMaxPressure");//давление на выходе насоса, Па

    out->setNewOut(0,outGasFlowRate);
    out->setNewOut(1,outGasPressure);
    out->setNewOut(2,paramToDouble("gasTemperature"));
    out->setNewOut(3,paramToDouble("gasActivity"));
    out->setNewOut(4,paramToDouble("particleFraction"));

    return true;
}


ICalcElement *Create()
{
    return new GCS_SourceBlock();
}

void Release(ICalcElement *block)
{
    if(block->type() == "GCS_SourceBlock") delete block;
}

std::string Type()
{
    return "GCS_SourceBlock";
}
