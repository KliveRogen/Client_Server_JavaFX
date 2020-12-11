#include "MP_Ref_TankBuff6372.h"

MP_Ref_TankBuff6372::MP_Ref_TankBuff6372()
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

}

void MP_Ref_TankBuff6372::setDataNames()
{
    std::vector<std::string> dn1,dn2;
    dn1.push_back("Объемный расход продукта 210, м3\ч");
    dn1.push_back("Температура продукта 210, °С");
    dn1.push_back("Концентрация урана в продукте 210, г\л");
    PortOut1->setDataNames(dn1);
    dn2.push_back("Уровень продукта в буферной емкости АД-6372/2, м3");
    PortOut2->setDataNames(dn2);
}

bool MP_Ref_TankBuff6372::init(std::string &error, double h)
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
    r=0.406;
    hh=2.225;
    return true;
}

bool MP_Ref_TankBuff6372::process(double t, double h, std::string &error)
{
    // Put your calculations here
    GF=paramToDouble("GF");
    Power=paramToDouble("Power");
    inVect1 = PortIn1->getInput();
    Qin=inVect1[0];
    Tin=inVect1[1];
    Cin=inVect1[2];
    if (isSituationActive("kavar")) {
        kavar=0.6;
    }
    F = kavar*GF;
    P = (2.593/1e7)*F*F-(2.347/1e4)*F-0.004009;
    I = (2.168/1e10)*F*F*F - (1.276/1e6)*F*F + 0.006595*F + 0.0008576;
    Qout = 9.714*P*P*P - 31.55*P*P + 33.2*P - 0.5055;
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

std::vector<double> MP_Ref_TankBuff6372::dydt()
{
    std::vector<double> derivatives;
    derivatives.resize(3);
    derivatives[0]=(Qin*(Tin-Tout))/(Qin-Qout);
    derivatives[1]=(Qin*(Cin-Cout))/(Qin-Qout);
    derivatives[2]=((Qin-Qout)*100)/(3.14*r*r*hh);

    return derivatives;
}

ICalcElement *Create()
{
    return new MP_Ref_TankBuff6372();
}

void Release(ICalcElement *block)
{
    if(block->type() == "MP_Ref_TankBuff6372") delete block;
}

std::string Type()
{
    return "MP_Ref_TankBuff6372";
}
