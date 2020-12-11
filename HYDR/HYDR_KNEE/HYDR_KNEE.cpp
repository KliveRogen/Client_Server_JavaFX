#include "HYDR_KNEE.h"
#include "math.h"

HYDR_KNEE::HYDR_KNEE()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
	createParameter("diameter", "0.3");
	createParameter("radius", "0.6");
	createParameter("angle", "90");

	// Сигналы блока

	// Порты блока
    in = createInputPort(0, "UNKNOWN_NAME", "INFO");
    out = createOutputPort(1, "UNKNOWN_NAME", "INFO");

	// Отказы блока

}

void HYDR_KNEE::setDataNames()
{
    std::vector<std::string> dn;
    dn.push_back("Расход, л/ч");
    dn.push_back("Кин. вязкость, Па*с");
    dn.push_back("Плотность, кг/м3");
    dn.push_back("Давление, кПа");
    dn.push_back("Концентрация, моль/л");
    out->setDataNames(dn);
}

bool HYDR_KNEE::init(std::string &error, double h)
{
    // Put your initialization here
	
	setDataNames();
    return true;
}

bool HYDR_KNEE::process(double t, double h, std::string &error)
{
    double  diam,radius,angle,flowIn,visc,dens,pressIn,concIn,
            square,wConst,Re,alpha,S90Const,aConst,pressDelta,
            pressOut;
    //получение параметров
    diam = paramToDouble("diameter");
    radius = paramToDouble("radius");
    angle = paramToDouble("angle");
    //получение входов
    flowIn = in->getInput()[0];
    visc = in->getInput()[1];
    dens = in->getInput()[2];
    pressIn = in->getInput()[3]*1e3;
    concIn = in->getInput()[4];
    //расчет
    // изменение давления
    flowIn = flowIn/(3600*1000);
    square = 3.14*diam*diam/4;
    wConst = flowIn/square;
    Re = dens*wConst*diam/visc;
    alpha = 75/Re;
    if ( Re > 2300 ) {
        alpha = 0.3164 * pow(Re,-0.25);
    }
    S90Const = 2000 * pow(alpha, 2.5) + 0.106 * pow(diam/radius, 2.5);
    if ( Re > 2e5 ) {
        S90Const = 0.05 + 0.19*diam/radius;
    }
    aConst = sin(angle*3.14/180);
    if (angle > 90) {
        aConst = 0.7 + 0.35*angle/90;
    }
    pressDelta = S90Const * aConst * dens * wConst * wConst /2;
    pressOut = pressIn - pressDelta;
    // выходы
    out->setOut(0,in->getInput()[0]);
    out->setOut(1,in->getInput()[1]);
    out->setOut(2,in->getInput()[2]);
    out->setOut(3,pressOut/1e3);
    out->setOut(4,in->getInput()[4]);
    return true;
}


ICalcElement *Create()
{
    return new HYDR_KNEE();
}

void Release(ICalcElement *block)
{
    if(block->type() == "HYDR_KNEE") delete block;
}

std::string Type()
{
    return "HYDR_KNEE";
}
