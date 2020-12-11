#include "TEST_AvariikaTest.h"

TEST_AvariikaTest::TEST_AvariikaTest()
{
	// Расчетный тип блока
    BlockCalcType = E_OUTPUTONLY;

    // Параметры блока
	createParameter("Par", "1");

	// Сигналы блока

	// Порты блока

    OutPort = createOutputPort(0, "UNKNOWN_NAME", "INFO");

	// Отказы блока
	createSituation("Mul2");
	createSituation("Plus");
	createSituation("Negative");

}

void TEST_AvariikaTest::setDataNames()
{
	std::vector<std::string> dn;
    dn.push_back("Res");
    OutPort->setDataNames(dn);
}

bool TEST_AvariikaTest::init(std::string &error, double h)
{
    // Put your initialization here

    Value = paramToDouble("Par");
    setDataNames();
    OutPort->setOut(0, Value);
    return true;
}

bool TEST_AvariikaTest::process(double t, double h, std::string &error)
{

    Value = paramToDouble("Par");
    // Put your calculations here
    if (isSituationActive("Mul2"))
    {
        Value = Value * 2;
    }
    if (isSituationActive("Plus"))
    {
        Value = Value + 2;
    }
    if (isSituationActive("Negative"))
    {
        Value = Value * -1;
    }

    OutPort->setOut(0, Value);
    return true;
}


ICalcElement *Create()
{
    return new TEST_AvariikaTest();
}

void Release(ICalcElement *block)
{
    if(block->type() == "TEST_AvariikaTest") delete block;
}

std::string Type()
{
    return "TEST_AvariikaTest";
}
