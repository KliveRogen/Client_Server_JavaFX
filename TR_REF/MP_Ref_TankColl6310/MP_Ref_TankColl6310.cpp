#include "MP_Ref_TankColl6310.h"
#include <qmath.h>
MP_Ref_TankColl6310::MP_Ref_TankColl6310()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
	createParameter("GF", "0");
	createParameter("Power", "0");

	// Сигналы блока

	// Порты блока
    PortIn1=createInputPort(0, "In1", "INFO");
    PortOut1=createOutputPort(1, "Out1", "INFO");
    PortOut2=createOutputPort(2, "Out2", "INFO");

	// Отказы блока
	createSituation("kavar");
    createSituation("kraz");

}

void MP_Ref_TankColl6310::setDataNames()
{
    std::vector<std::string> dn1,dn2;
    dn1.push_back("Объемный расход продукта 200, м3\ч");
    dn1.push_back("Температура продукта 200, °С");
    dn1.push_back("Концентрация урана в продукте 200, г\л");
    PortOut1->setDataNames(dn1);
    dn2.push_back("Уровень продукта в баке-сборнике АД-6310/1, м3");
    PortOut2->setDataNames(dn2);
}

bool MP_Ref_TankColl6310::init(std::string &error, double h)
{
    // Put your initialization here
	
	setDataNames();

    GF=paramToDouble("GF");
    Power=paramToDouble("Power");
    inVect1 = PortIn1->getInput();
    Qin=inVect1[0];
    Tin=inVect1[1];
    Cin=inVect1[2];
    kavar=1;
    kraz=0;
    return true;
}

bool MP_Ref_TankColl6310::process(double t, double h, std::string &error)
{
    GF=paramToDouble("GF");
    Power=paramToDouble("Power");
    inVect1 = PortIn1->getInput();
    Qin=inVect1[0];
    Tin=inVect1[1];
    Cin=inVect1[2];
    // Put your calculations here
    if (isSituationActive("kavar")) {
        kavar=0.6;
    }
    if (isSituationActive("kraz")) {
        kraz=1;
    }
    F = kavar*GF;
    P = 0.002181*F+(3.567/1e17);
    I= (2.168/1e10)*F*F*F - (1.276/1e6)*F*F + 0.0066*F + (8.58/1e4);

    Qout = -0.7321*P*P + 5.59*P + 0.1061;
    std::vector<double> res(3);
    res = dydt();
    Tout=res[0]*h;
    Cout=res[1]*h;
    L=res[2]*h;
    PortOut1->setOut(0, Qout);
    PortOut1->setOut(1, Tout);
    PortOut1->setOut(2, Cout);
    PortOut2->setOut(0, L);
    return true;
}

std::vector<double> MP_Ref_TankColl6310::dydt()
{
    std::vector<double> derivatives;
    derivatives.resize(3);
    derivatives[0]=(Qin*(Tin-Tout))/(Qin-Qout);
    derivatives[1]=(Qin*(Cin-Cout))/(Qin-Qout);
    derivatives[2]=Qin-Qout-kraz*5;
    return derivatives;

}
ICalcElement *Create()
{
    return new MP_Ref_TankColl6310();
}

void Release(ICalcElement *block)
{
    if(block->type() == "MP_Ref_TankColl6310") delete block;
}

std::string Type()
{
    return "MP_Ref_TankColl6310";
}
