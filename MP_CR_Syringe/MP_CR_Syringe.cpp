#include "MP_CR_Syringe.h"
#include <qmath.h>

// NOTE: Необходима доработка


MP_CR_Syringe::MP_CR_Syringe()
{
    // Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
    createParameter("CylinderHeight", "145");
    createParameter("CylinderRadius", "61.5");
    createParameter("PistonHeight", "140");
    createParameter("PistonRadius", "56.5");
    createParameter("FeedHoleRadius", "6");
    createParameter("SensorHoleRadius", "5");

    // Сигналы блока
    PistonLevelSig = createSignal("PistonLevel", Signal::ST_DOUBLE);
    LiquidLevelSig = createSignal("LiquidLevel", Signal::ST_DOUBLE);

    // Порты блока
    PortInControl = createInputPort(0, "Управление", "INFO");
    PortInLiq = createInputPort(1, "Материал", "INFO");
    PortOutData = createOutputPort(2, "Данные", "INFO");
    PortOutLiq = createOutputPort(3, "Материал", "INFO");

    // Отказы блока

}

void MP_CR_Syringe::setDataNames()
{
    std::vector<std::string> dn1, dn2;

    dn1.push_back("Объем жидкости");
    dn1.push_back("Положение поршня");
    PortOutData->setDataNames(dn1);

    dn2.push_back("Расход, м3/с");
    dn2.push_back("Температура раствора, °C");
    dn2.push_back("Концентрация урана, г/л");
    dn2.push_back("Концентрация плутония, г/л");
    dn2.push_back("Концентрация нептуния, г/л");
    dn2.push_back("Концентрация азотной кислоты, г/л");
    PortOutLiq->setDataNames(dn2);
}

bool MP_CR_Syringe::init(std::string &error, double h)
{
    // Put your initialization here
    setDataNames();

    CylinderHeight = paramToDouble("CylinderHeight");
    CylinderRadius = paramToDouble("CylinderRadius");
    PistonHeight = paramToDouble("PistonHeight");
    PistonRadius = paramToDouble("PistonRadius");
    FeedHoleRadius = paramToDouble("FeedHoleRadius");
    SensorHoleRadius = paramToDouble("SensorHoleRadius");

    //Иницициализация
    LiqVol = 0;
    PistonLevel = 0;

    Flow = 0;
    Temperature = 0;
    mU = 0;
    mPu = 0;
    mNp = 0;
    mHNO3 = 0;

    dhPiston = 0.01;
    PistonLiftTime = 180;

    SqPiston = M_PI*PistonRadius*PistonRadius - M_PI*FeedHoleRadius*FeedHoleRadius - M_PI*SensorHoleRadius*SensorHoleRadius;
    VPiston = SqPiston*PistonHeight*(1e-3); // в мл
    VCylinder = M_PI*CylinderHeight*CylinderRadius*CylinderRadius*(1e-3); // в мл

    PortOutData->setOut(0, LiqVol);
    PortOutData->setOut(1, (100/PistonHeight)*PistonLevel);

    PortOutLiq->setOut(0, Flow);
    PortOutLiq->setOut(1, Temperature);
    PortOutLiq->setOut(2, mU);
    PortOutLiq->setOut(3, mPu);
    PortOutLiq->setOut(4, mNp);
    PortOutLiq->setOut(5, mHNO3);

    double pLevel, lLevel;
    pLevel = 100 - 100*PistonLevel/PistonHeight;
    lLevel = 100*1000*LiqVol/VCylinder;
    PistonLevelSig->Value.doubleVal = pLevel;
    LiquidLevelSig->Value.doubleVal = lLevel;

    return true;
}

bool MP_CR_Syringe::process(double t, double h, std::string &error)
{
    // Получаем вход с портов
    std::vector<double> VectInControl, VectInLiq;
    VectInControl = PortInControl->getInput();
    VectInLiq = PortInLiq->getInput();

    perFIM = 0;
    if (VectInControl.size() != 0)
        perFIM = VectInControl[0];

    inFlow = 0;
    if (VectInLiq.size() != 0)
    {
        inFlow = VectInLiq[0]*1000*3600;
    }
    inTemp = VectInLiq[1];
    inCU = VectInLiq[2];
    inCPu = VectInLiq[3];
    inCNp = VectInLiq[4];
    inCHNO3 = VectInLiq[5];

    std::vector<double> dydt_vect(6);
    std::vector<double> res(6);

    res = dydt();

    LiqVol += res[0]*h;
    PistonLevel += res[1]*h;
    mU += res[2]*h;
    mPu += res[3]*h;
    mNp += res[4]*h;
    mHNO3 += res[5]*h;

    if(PistonLevel >= PistonHeight) PistonLevel = PistonHeight;
    if(PistonLevel < 0) PistonLevel = 0;

    if (LiqVol<0) LiqVol = 0;
    if ( (Flow-inFlow)*h/3600 > LiqVol)
        Flow = LiqVol + inFlow*h/3600;

    if (LiqVol * 1000 > VCylinder)
    {
        error = "Дозатор переполнен";
        return false;
    }

    double pLevel, lLevel;
    pLevel = 100*PistonLevel/PistonHeight;
    lLevel = 100*1000*LiqVol/VCylinder;



    // Установка значений на выходе
    PortOutData->setNewOut(0, LiqVol);
    PortOutData->setNewOut(1, pLevel);

    PortOutLiq->setNewOut(0, 0.00000027778*Flow);
    PortOutLiq->setNewOut(1, inTemp);
    PortOutLiq->setNewOut(2, mU/LiqVol);
    PortOutLiq->setNewOut(3, mPu/LiqVol);
    PortOutLiq->setNewOut(4, mNp/LiqVol);
    PortOutLiq->setNewOut(5, mHNO3/LiqVol);

    // Установка сигналов
    PistonLevelSig->Value.doubleVal = 100 - pLevel;
    LiquidLevelSig->Value.doubleVal = lLevel;
    return true;
}

std::vector<double> MP_CR_Syringe::dydt()
{
    std::vector<double> derivatives;
    derivatives.resize(6);

    double pistondh = 0;
    if (perFIM == 0) pistondh = 0;
    else pistondh = 10/perFIM;

    if (PistonHeight - PistonLevel < pistondh)
    {
        pistondh = PistonHeight - PistonLevel;
    }
    double oldFlow = 3.6*SqPiston*dhPiston/perFIM;
    // Расчет расхода по ШИМ
    if(perFIM <= 0)
    {
        Flow = 0;
    }
    else
    {
        Flow = 0.36*pistondh*SqPiston*dhPiston; // расход в [л/ч] dhPiston - коэффициэнт, pistondh - на сколько опустился поршень
    }

    derivatives[0] = (inFlow-Flow)/3600;

    if (perFIM == 0) derivatives[1] = 0;
    else derivatives[1] = pistondh;

    if(LiqVol == 0)
    {
        derivatives[2] = (inCU*inFlow)/3600;
        derivatives[3] = (inCPu*inFlow)/3600;
        derivatives[4] = (inCNp*inFlow)/3600;
        derivatives[5] = (inCHNO3*inFlow)/3600;
    }
    else
    {
        derivatives[2] = (inCU*inFlow - (mU*Flow)/LiqVol)/3600;
        derivatives[3] = (inCPu*inFlow - (mPu*Flow)/LiqVol)/3600;
        derivatives[4] = (inCNp*inFlow - (mNp*Flow)/LiqVol)/3600;
        derivatives[5] = (inCHNO3*inFlow - (mHNO3*Flow)/LiqVol)/3600;
    }

    return derivatives;
}

ICalcElement *Create()
{
    return new MP_CR_Syringe();
}

void Release(ICalcElement *block)
{
    if(block->type() == "MP_CR_Syringe") delete block;
}

std::string Type()
{
    return "MP_CR_Syringe";
}
