#include "GCS_SourceBlock.h"

GCS_SourceBlock::GCS_SourceBlock()
{
	// Расчетный тип блока
    BlockCalcType = E_OUTPUTONLY;

    // Параметры блока
	createParameter("gasFlowRate", "0.026");
	createParameter("gasPressure", "100000");
	createParameter("gasTemperature", "500");
	createParameter("gasActivity", "10000");
	createParameter("particleFraction", "0.05");

	// Сигналы блока

	// Порты блока
    out = createOutputPort(0, "UNKNOWN_NAME", "INFO");

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
	
	setDataNames();
    return true;
}

bool GCS_SourceBlock::process(double t, double h, std::string &error)
{
    // Put your calculations here

    //вывод параметров на выходной порт и проверка
    for (int i = 0; i < (int)Parameters.size(); i++){
        if (paramToDouble(Parameters[i])<0){
            error = "Ошибка в заполнении исходных данных!";
            return false;
        }
        out->setOut(i,paramToDouble(Parameters[i]));
    }
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
