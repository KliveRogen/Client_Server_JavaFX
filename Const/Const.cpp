#include "Const.h"

Const::Const()
{
    BlockCalcType = E_OUTPUTONLY;

    Value = 0;

    OutPort = createOutputPort(0, "ConstOut", "INFO");
    std::vector<std::string> dn;
    dn.push_back("Константа");
    OutPort->setDataNames(dn);

    createParameter("CONST_VAL", "0");

    ConstSig = createSignal("ConstValue", Signal::ST_DOUBLE);
}

bool Const::process(double t, double h, std::string &error)
{
    Value = paramToDouble(Parameters[0]);
    ConstSig->Value.doubleVal = Value;
    OutPort->setOut(0, Value);

    return true;
}

bool Const::init(std::string &error, double h)
{
    double testVal = h;
    Value = paramToDouble(Parameters[0]);
    ConstSig->Value.doubleVal = Value;
    return true;
}

ICalcElement *Create()
{
    return new Const();
}

void Release(ICalcElement *block)
{
    if(block->type() == "Const") delete block;
}

std::string Type()
{
    return "Const";
}
