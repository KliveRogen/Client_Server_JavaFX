#include "SignalTester.h"

SignalTester::SignalTester()
{
	// Расчетный тип блока
    BlockCalcType = E_OUTPUTONLY;

    // Параметры блока

	// Сигналы блока
    Scale = createSignal("SIG_DOUBLE", Signal::ST_DOUBLE);
    Lamp = createSignal("SIG_BOOL", Signal::ST_BOOL);

	// Порты блока

	// Отказы блока

}

bool SignalTester::process(double t, double h, std::string &error)
{
    // Put your calculations here
    T += h;
    if(T > 100.00) {
        T -= 100.00;
        LampVal = !LampVal;
    }

    ScaleVal += h;
    if(ScaleVal >= 100.00) ScaleVal = 0;

    Scale->Value.doubleVal = ScaleVal;
    Lamp->Value.boolVal    = LampVal;

    return true;
}

bool SignalTester::init(std::string &error, double h)
{
    T = 0;
    ScaleVal = 0.00;
    LampVal = false;

    // Put your initialization here
    return true;
}

ICalcElement *Create()
{
    return new SignalTester();
}

void Release(ICalcElement *block)
{
    if(block->type() == "SignalTester") delete block;
}

std::string Type()
{
    return "SignalTester";
}
