#include "EX_PUMP.h"

#include "../DynMath/InertOne.h"

EX_PUMP::EX_PUMP()
{
	// Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
    createParameter("T_F", "4");
    createParameter("T_P", "5");
    createParameter("T_I", "5");
	createParameter("K1", "0.007");
	createParameter("K2", "0.003");

	// Сигналы блока
    SP_FREQ = createSignal("S_FREQ", Signal::ST_DOUBLE);
    SP_I    = createSignal("S_I", Signal::ST_DOUBLE);
    SP_P    = createSignal("S_P", Signal::ST_DOUBLE);

	// Порты блока
    OnOffPort   = createInputPort(0, "Вкл/выкл", "INFO");
    SetFlowPort = createInputPort(1, "Уставка по расходу", "INFO");
    FlowPort    = createOutputPort(2, "Напор", "INFO");
    DataPort    = createOutputPort(3, "Данные", "INFO");

	// Отказы блока

}

bool EX_PUMP::process(double t, double h, std::string &error)
{
    // Put your calculations here
    double inOut = 0;
    double gf = 0, f;
    double p, i;

    if(OnOffPort->inputDataSize() >= 1)   inOut = OnOffPort->getInput()[0];
    if(SetFlowPort->inputDataSize() >= 1) gf = OnOffPort->getInput()[0];

    if(inOut == 0) gf = 0;
    f = gf;
    p = paramToDouble("K1") * f;
    i = paramToDouble("K2") * f;

    I_F->step(f, h);
    I_P->step(p, h);
    I_I->step(i, h);

    FlowPort->setNewOut(0, I_F->value());

    DataPort->setNewOut(0, I_F->value());
    DataPort->setNewOut(1, I_P->value());
    DataPort->setNewOut(2, I_I->value());

    SP_FREQ->Value.doubleVal = I_F->value();
    SP_I->Value.doubleVal    = I_P->value();
    SP_P->Value.doubleVal    = I_I->value();

    return true;
}

bool EX_PUMP::init(std::string &error, double h)
{
    // Put your initialization here
    std::vector<std::string> dn;
    dn.push_back("Давление");
    FlowPort->setDataNames(dn);

    dn.resize(3);
    dn[0] = "F";
    dn[1] = "P";
    dn[2] = "I";
    DataPort->setDataNames(dn);

    FREQ = 0;

    I_F = new InertOne();
    I_P = new InertOne();
    I_I = new InertOne();

    if(paramToDouble("T_F") <= 0) {
        error = "Пост. времени частоты вращения насоса должна быть больше нуля!";
        return false;
    }

    if(paramToDouble("T_P") <= 0) {
        error = "Пост. времени напора насоса должна быть больше нуля!";
        return false;
    }

    if(paramToDouble("T_I") <= 0) {
        error = "Пост. времени тока насоса должна быть больше нуля!";
        return false;
    }

    I_F->init(0, 1, paramToDouble("T_F"));
    I_P->init(0, 1, paramToDouble("T_P"));
    I_I->init(0, 1, paramToDouble("T_I"));

    return true;
}

ICalcElement *Create()
{
    return new EX_PUMP();
}

void Release(ICalcElement *block)
{
    if(block->type() == "EX_PUMP") delete block;
}

std::string Type()
{
    return "EX_PUMP";
}
