#include "HYDR_TANK.h"

HYDR_TANK::HYDR_TANK()
{
	// Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
	createParameter("height", "1");
	createParameter("diameter", "1");
	createParameter("portInLevel", "1");
	createParameter("portOutLevel", "0.5");
	createParameter("initValue", "0");
	createParameter("initConc", "0");
	createParameter("hydrOutResist", "100");
	createParameter("valvePos", "0");

	// Сигналы блока

	// Порты блока
    in = createInputPort(0, "in", "INFO");
    out = createOutputPort(1, "out", "INFO");

	// Отказы блока

}

void HYDR_TANK::setDataNames()
{
	std::vector<std::string> dn;
    dn.push_back("Высота столба в емкости");
    dn.push_back("Концентрация");
    dn.push_back("Расход на выходе");
    dn.push_back("Давление на выходе");
    out->setDataNames(dn);
}

bool HYDR_TANK::init(std::string &error, double h)
{
	setDataNames();
    double  tankHeight,tankDiam,portInLvl,portOutLvl,initValue,
            initConc,hydrOutRes,valvePos,flowIn,viscIn,densIn,
            pressIn,concIn,tankSquare,tankVol,liquidLvl,pressOut,
            flowOut;
    // получение параметров
    tankHeight = paramToDouble("height");
    tankDiam = paramToDouble("diameter");
    portInLvl = paramToDouble("portInLevel");
    portOutLvl = paramToDouble("portOutLevel");
    initValue = paramToDouble("initValue");
    initConc = paramToDouble("initConc");
    hydrOutRes = paramToDouble("hydrOutResist");
    valvePos = paramToDouble("valvePos")/100;
    // получение входов
    flowIn = in->getInput()[0] *h /3600;
    viscIn = in->getInput()[1];
    densIn = in->getInput()[2];
    pressIn = in->getInput()[3];
    concIn = in->getInput()[4];
    //доп расчет
    //площадь сечения емкости
    tankSquare = 3.14*tankDiam*tankDiam /4;
    //объем емкости
    tankVol = tankSquare*tankHeight;
    //уровень жидкости в емкости
    liquidLvl = liquidVol /tankSquare;
    //основной расчет
    // инициализации
    liquidConc = initConc;
    liquidVol = tankVol*initValue/100;
    //выходное давление
    pressOut = 101300;
    if ( liquidLvl > portOutLvl ) {
        pressOut += densIn*9.81*(liquidLvl - portOutLvl*tankHeight);
    }
    //выходной расход
    flowOut = valvePos*(pressOut - 101300)*h /hydrOutRes;
    // изменение объема жидкости
    if ( liquidVol > tankVol ) {
        liquidVol = tankVol;
        flowIn = 0;
    }
    if ( liquidVol < 0 ) {
        liquidVol = 0;
        flowOut = 0;
    }
    //вывод результатов
    out->setOut(0,liquidVol/tankSquare);
    out->setOut(1,liquidConc);
    out->setOut(2,flowOut*3600);
    out->setOut(3,pressOut);
    return true;
}

bool HYDR_TANK::process(double t, double h, std::string &error)
{
    double  tankHeight,tankDiam,portInLvl,portOutLvl,initValue,
            initConc,hydrOutRes,valvePos,flowIn,viscIn,densIn,
            pressIn,concIn,tankSquare,tankVol,liquidLvl,pressOut,
            flowOut;
    // получение параметров
    tankHeight = paramToDouble("height");
    tankDiam = paramToDouble("diameter");
    portInLvl = paramToDouble("portInLevel");
    portOutLvl = paramToDouble("portOutLevel");
    initValue = paramToDouble("initValue");
    initConc = paramToDouble("initConc");
    hydrOutRes = paramToDouble("hydrOutResist");
    valvePos = paramToDouble("valvePos");
    // получение входов
    flowIn = in->getInput()[0] *h /3600;
    viscIn = in->getInput()[1];
    densIn = in->getInput()[2];
    pressIn = in->getInput()[3]*1e3;
    concIn = in->getInput()[4];
    //доп расчет
    //площадь сечения емкости
    tankSquare = 3.14*tankDiam*tankDiam /4;
    //объем емкости
    tankVol = tankSquare*tankHeight;
    //уровень жидкости в емкости
    liquidLvl = liquidVol /tankSquare;
    //основной расчет
    //выходное давление
    pressOut = 101300;
    if ( liquidLvl > portOutLvl ) {
        pressOut += densIn*9.81*(liquidLvl - portOutLvl*tankHeight);
    }
    //выходной расход
    flowOut = valvePos*(pressOut - 101300)*h /hydrOutRes;
    // изменение концентрации
    liquidConc = (liquidConc*liquidVol + concIn*flowIn) / (liquidVol + flowIn);
    // изменение объема жидкости
    liquidVol = liquidVol + (flowIn - flowOut);
    if ( liquidVol > tankVol ) {
        liquidVol = tankVol;
        flowIn = 0;
    }
    if ( liquidVol < 0 ) {
        liquidVol = 0;
        flowOut = 0;
    }
    //вывод результатов
    out->setNewOut(0,liquidVol/tankSquare);
    out->setNewOut(1,liquidConc);
    out->setNewOut(2,flowOut*3600);
    out->setNewOut(3,pressOut);
    return true;
}


ICalcElement *Create()
{
    return new HYDR_TANK();
}

void Release(ICalcElement *block)
{
    if(block->type() == "HYDR_TANK") delete block;
}

std::string Type()
{
    return "HYDR_TANK";
}
