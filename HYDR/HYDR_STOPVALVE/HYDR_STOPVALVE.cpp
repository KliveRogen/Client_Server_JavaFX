#include "HYDR_STOPVALVE.h"
#include "math.h"

HYDR_STOPVALVE::HYDR_STOPVALVE()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
	createParameter("rateCap", "10");
	createParameter("leakClose", "0.000001");

	// Сигналы блока

	// Порты блока
    in = createInputPort(0, "UNKNOWN_NAME", "INFO");
    ctrl = createInputPort(1, "UNKNOWN_NAME", "INFO");
    out = createOutputPort(2, "UNKNOWN_NAME", "INFO");

	// Отказы блока

}

void HYDR_STOPVALVE::setDataNames()
{
    std::vector<std::string> dn;
    dn.push_back("Расход, л/ч");
    dn.push_back("Давление, Па");
    dn.push_back("Гидр.сопрот.");
    out->setDataNames(dn);
}

bool HYDR_STOPVALVE::init(std::string &error, double h)
{
	setDataNames();
    return true;
}

bool HYDR_STOPVALVE::process(double t, double h, std::string &error)
{
    double  press1,press2,roConst,rateCap,leakClose,
            coeff,g100Const,flowOut,resis,valvePos;
    //входы
    press1 = in->getInput()[0];
    press2 = in->getInput()[1];
    roConst = in->getInput()[2];
    valvePos = ctrl->getInput()[0];
    if (valvePos >= 1) {
        valvePos = 100;
    }
    if (valvePos < 1) {
        valvePos = 0;
    }
    //параметры
    rateCap = paramToDouble("rateCap");
    leakClose = paramToDouble("leakClose");
    //проверка введенных данных
    //расчет
    coeff = 1;
    if (press1 < press2) {
        coeff = -1;
    }
    g100Const = rateCap * coeff * sqrt(fabs(press1 - press2) * 1000 / roConst);
    flowOut = g100Const*valvePos/100 + leakClose;
    resis = fabs(press1 - press2) /pow(flowOut,2);
    //выводы
    out->setOut(0,flowOut);
    out->setOut(1,press2);
    out->setOut(2,resis);
    return true;
}


ICalcElement *Create()
{
    return new HYDR_STOPVALVE();
}

void Release(ICalcElement *block)
{
    if(block->type() == "HYDR_STOPVALVE") delete block;
}

std::string Type()
{
    return "HYDR_STOPVALVE";
}
