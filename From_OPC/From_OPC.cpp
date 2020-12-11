#include "From_OPC.h"

From_OPC::From_OPC()
{
	// Расчетный тип блока
    //BlockCalcType = E_OUTPUTONLY;
    BlockCalcType = E_INITVALUES;

    Value = 0;

	// Сигналы блока
    FromOpcSig = createSignal("From_OPC", Signal::ST_DOUBLE);
    BoolFromOpcSig = createSignal("Bool_From_OPC", Signal::ST_BOOL);

	// Порты блока
    OutPort = createOutputPort(0, "RecieveFromOpc", "INFO");
    std::vector<std::string> dn;
    dn.push_back("From OPC");
    OutPort->setDataNames(dn);
}

bool From_OPC::init(std::string &error, double h)
{
    FromOpcSig->Value.doubleVal = 0;
    return true;
}

bool From_OPC::process(double t, double h, std::string &error)
{
    // Put your calculations here

    Value = FromOpcSig->Value.doubleVal;
    bool bv = BoolFromOpcSig->Value.charVal;
//    OutPort->setOut(0, Value);
  OutPort->setNewOut(0, Value);
    return true;
}


ICalcElement *Create()
{
    return new From_OPC();
}

void Release(ICalcElement *block)
{
    if(block->type() == "From_OPC") delete block;
}

std::string Type()
{
    return "From_OPC";
}
