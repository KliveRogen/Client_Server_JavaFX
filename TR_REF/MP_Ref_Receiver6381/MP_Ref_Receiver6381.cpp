#include "MP_Ref_Receiver6381.h"

MP_Ref_Receiver6381::MP_Ref_Receiver6381()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока

	// Сигналы блока

	// Порты блока
    PortIn1=createInputPort(0, "In1", "INFO");
    PortIn2=createInputPort(1, "In2", "INFO");
    PortOut1=createOutputPort(2, "Out1", "INFO");

	// Отказы блока
    createSituation("kmag");

}

void MP_Ref_Receiver6381::setDataNames()
{
	std::vector<std::string> dn;
    dn.push_back("Давление в ресивере, кгс/см2");
    PortOut1->setDataNames(dn);
}

bool MP_Ref_Receiver6381::init(std::string &error, double h)
{
    // Put your initialization here
	
	setDataNames();
    inVect1 = PortIn1->getInput();
    DYP06 = inVect1[0];
    inVect2 = PortIn2->getInput();
    DYP07 = inVect2[0];
  //  PortOut1->setOut(0,P); //?????

    kmag=1;
    P=3;
    return true;
}

bool MP_Ref_Receiver6381::process(double t, double h, std::string &error)
{
    // Put your calculations here
    // инициализация отказа
    inVect1 = PortIn1->getInput();
    DYP06 = inVect1[0];
    inVect2 = PortIn2->getInput();
    DYP07 = inVect2[0];
    if (isSituationActive("kmag")) {
        kmag=0;
    }
    Pres=kmag*3;
    std::vector<double> res(1);
    Pl=P;
    res = dydt();
    P += res[0]*h;
    PortOut1->setOut(0, Pl);
    return true;
}

std::vector<double> MP_Ref_Receiver6381::dydt()
{
    std::vector<double> derivatives;
    derivatives.resize(1);
    if (DYP06==0 && DYP07==0)
    {
        derivatives[0]=Pl;  // !!!!(t-1)
    }
    if (DYP06!=0 && DYP07==0)
    {
        derivatives[0]=(-P)/(10-0.1*DYP06);
    }
    if (DYP07 !=0)
    {
        derivatives[0]=(Pres-P)/(10-0.1*DYP07+0.2*DYP06);
    }
    return derivatives;
}

ICalcElement *Create()
{
    return new MP_Ref_Receiver6381();
}

void Release(ICalcElement *block)
{
    if(block->type() == "MP_Ref_Receiver6381") delete block;
}

std::string Type()
{
    return "MP_Ref_Receiver6381";
}
