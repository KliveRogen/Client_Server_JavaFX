#include "HYDR_PIPE.h"
#include "math.h"

HYDR_PIPE::HYDR_PIPE()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
	createParameter("resistance", "1");
	createParameter("length", "10");
	createParameter("diameter", "0.35");

	// Сигналы блока

	// Порты блока
    in = createInputPort(0, "UNKNOWN_NAME", "INFO");
    out = createOutputPort(1, "UNKNOWN_NAME", "INFO");

	// Отказы блока

}

void HYDR_PIPE::setDataNames()
{
    std::vector<std::string> dn;
    dn.push_back("Расход, л/ч");
    dn.push_back("Кин. вязкость, Па*с");
    dn.push_back("Плотность, кг/м3");
    dn.push_back("Давление, кПа");
    dn.push_back("Концентрация, моль/л");
    out->setDataNames(dn);
}

bool HYDR_PIPE::init(std::string &error, double h)
{
    concOut = in->getInput()[4];
	setDataNames();
    return true;
}

bool HYDR_PIPE::process(double t, double h, std::string &error)
{
    double  resis,length,diam,flowIn,visc,dens,pressIn,concIn,
            Re,alpha,pressDelta,pressOut,timeConst,concDelta;
    // получение параметров
    resis = paramToDouble("resistance");
    length = paramToDouble("length");
    diam = paramToDouble("diameter");
    // получение входов
    flowIn = in->getInput()[0];
    visc = in->getInput()[1];
    dens = in->getInput()[2];
    pressIn = in->getInput()[3]*1e3;
    concIn = in->getInput()[4];
    // расчет
    // изменение давления
    Re = 4*flowIn/(3.14*diam*visc);
    alpha = 75/Re;
    if ( Re > 2300 ) {
        alpha = 0.3164 * pow(Re,-0.25);
    }
    pressDelta = resis*alpha*8*dens*length*flowIn*flowIn /( 3.14*3.14*pow(diam,5) );
    pressOut = pressIn - pressDelta;
    // изменение концентрации
    timeConst = 3.14*length*visc*diam /4 /(flowIn/3600);
    concDelta = (concIn - concOut)/timeConst;
    concOut = concOut + concDelta*h;
    // выходы
    out->setOut(0,in->getInput()[0]);
    out->setOut(1,in->getInput()[1]);
    out->setOut(2,in->getInput()[2]);
    out->setOut(3,pressOut/1e3);
    out->setOut(4,concOut);
    return true;
}


ICalcElement *Create()
{
    return new HYDR_PIPE();
}

void Release(ICalcElement *block)
{
    if(block->type() == "HYDR_PIPE") delete block;
}

std::string Type()
{
    return "HYDR_PIPE";
}
