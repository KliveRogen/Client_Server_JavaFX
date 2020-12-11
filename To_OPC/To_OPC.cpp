#include "To_OPC.h"

To_OPC::To_OPC()
{
    // Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
    //createParameter("to_OPC", "0");

    // Сигналы блока
    ToOpcSig = createSignal("To_OPC", Signal::ST_DOUBLE);
    BoolToOpcSig = createSignal("Bool_To_OPC", Signal::ST_BOOL);

    // Порты блокa
    InPort = createInputPort(0, "SendToOpc", "INFO");

    /* std::vector<std::string> dn;
    dn.push_back("Отправить в ОРС");
    InPort->setDataNames(dn);*/
}

bool To_OPC::init(std::string &error, double h)
{
    // Put your initialization here
    std::vector<double> InpVal;
    InpVal = InPort->getInput();
    if( InpVal.size())
    {
        // Value = paramToDouble(InpVal[0]);
        ToOpcSig->Value.doubleVal = InpVal[0];
    }

    return true;
}

bool To_OPC::process(double t, double h, std::string &error)
{
    std::vector<double> InpVal;
    InpVal = InPort->getInput();
    if( InpVal.size())
    {
        // Value = paramToDouble(InpVal[0]);
        ToOpcSig->Value.doubleVal = InpVal[0];
        BoolToOpcSig->Value.boolVal = InpVal[0]>0?true:false;
        //BoolToOpcSig->Value.uintVal = InpVal[0]>0?1:0;
    }

    return true;
}


ICalcElement *Create()
{
    return new To_OPC();
}

void Release(ICalcElement *block)
{
    if(block->type() == "To_OPC") delete block;
}

std::string Type()
{
    return "To_OPC";
}
