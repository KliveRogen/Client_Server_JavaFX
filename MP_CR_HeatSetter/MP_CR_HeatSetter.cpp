#include "MP_CR_HeatSetter.h"

MP_CR_HeatSetter::MP_CR_HeatSetter()
{
	// Расчетный тип блока
    BlockCalcType = E_OUTPUTONLY;

    // Параметры блока
	createParameter("HeatUpCryst", "5");
	createParameter("HeatDownCryst", "5");
	createParameter("HeatUpWash", "5");
	createParameter("HeatDownWash", "5");

    createParameter("HeatUpCrystInertion", "50");
    createParameter("HeatDownCrystInertion", "50");
    createParameter("HeatUpWashInertion", "50");
    createParameter("HeatDownWashInertion", "50");

    createParameter("HeatUpCrystTempErr", "50");
    createParameter("HeatDownCrystTempErr", "50");
    createParameter("HeatUpWashTempErr", "50");
    createParameter("HeatDownWashTempErr", "50");
	// Сигналы блока

	// Порты блока
    PortOut = createOutputPort(0, "", "INFO");

    // Отказы блока
    createSituation("ES_TEMPERATURE_ERR");
}

void MP_CR_HeatSetter::setDataNames()
{
	std::vector<std::string> dn;
    dn.push_back("Температура верхней рубашки зоны кристаллизации, °C");
    dn.push_back("Температура нижней рубашки зоны кристаллизации, °C");
    dn.push_back("Температура верхней рубашки зоны промывки, °C");
    dn.push_back("Температура нижней рубашки зоны промывки, °C");
    PortOut->setDataNames(dn);
}

bool MP_CR_HeatSetter::init(std::string &error, double h)
{
    // Put your initialization here
	
	setDataNames();
    int i = 0;
    double TargetTempHeatUpCryst = paramToDouble("HeatUpCryst");
    double TargetTempHeatDownCryst = paramToDouble("HeatDownCryst");
    double TargetTempHeatUpWash = paramToDouble("HeatUpWash");
    double TargetTempHeatDownWash = paramToDouble("HeatDownWash");

    std::vector<double> TargetTemp;
    TargetTemp.push_back(TargetTempHeatUpCryst);
    TargetTemp.push_back(TargetTempHeatDownCryst);
    TargetTemp.push_back(TargetTempHeatUpWash);
    TargetTemp.push_back(TargetTempHeatDownWash);

    Out = TargetTemp;

    PortOut->setOut(Out);
    return true;
}

bool MP_CR_HeatSetter::process(double t, double h, std::string &error)
{
    // Put your calculations here
    int i = 0;
    double TargetTempHeatUpCryst = paramToDouble("HeatUpCryst");
    double TargetTempHeatDownCryst = paramToDouble("HeatDownCryst");
    double TargetTempHeatUpWash = paramToDouble("HeatUpWash");
    double TargetTempHeatDownWash = paramToDouble("HeatDownWash");

    double HeatUpCrystInertion = paramToDouble("HeatUpCrystInertion");
    double HeatDownCrystInertion = paramToDouble("HeatDownCrystInertion");
    double HeatUpWashInertion = paramToDouble("HeatUpWashInertion");
    double HeatDownWashInertion = paramToDouble("HeatDownWashInertion");

    int size = 4;
    std::vector<double> TargetTemp;
    TargetTemp.push_back(TargetTempHeatUpCryst);
    TargetTemp.push_back(TargetTempHeatDownCryst);
    TargetTemp.push_back(TargetTempHeatUpWash);
    TargetTemp.push_back(TargetTempHeatDownWash);

    std::vector<double> Inertion;
    Inertion.push_back(HeatUpCrystInertion);
    Inertion.push_back(HeatDownCrystInertion);
    Inertion.push_back(HeatUpWashInertion);
    Inertion.push_back(HeatDownWashInertion);

    std::vector<double> Error(size, 0);

    if (isSituationActive("ES_TEMPERATURE_ERR"))
    {
        int i = 0;
        Error[i] = paramToDouble("HeatUpCrystTempErr"); i++;
        Error[i] = paramToDouble("HeatDownCrystTempErr"); i++;
        Error[i] = paramToDouble("HeatUpWashTempErr"); i++;
        Error[i] = paramToDouble("HeatDownWashTempErr"); i++;
    }

    std::vector<double> dout(size, 0);
    for (int i = 0; i<size; i++)
    {
        dout[i] = ( (TargetTemp[i]*((100.0 + Error[i])/100.0) ) - Out[i] )/ Inertion[i];
        Out[i] += dout[i]*h;
    }

    PortOut->setOut(Out);
    return true;
}


ICalcElement *Create()
{
    return new MP_CR_HeatSetter();
}

void Release(ICalcElement *block)
{
    if(block->type() == "MP_CR_HeatSetter") delete block;
}

std::string Type()
{
    return "MP_CR_HeatSetter";
}
