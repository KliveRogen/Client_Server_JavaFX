#include "EX_REEXTRACTOR.h"

EX_REEXTRACTOR::EX_REEXTRACTOR()
{
	// Расчетный тип блока
    BlockCalcType = E_UNKNOWN;

    // Параметры блока
	createParameter("K1", "0.937");
	createParameter("K2", "1.711");
	createParameter("A", "104");
	createParameter("B", "-1.205");
	createParameter("C", "-4");
	createParameter("EX_D_NOZ", "1.8");
	createParameter("EX_D_SZN", "1.28");
	createParameter("EX_D_SZV", "1.28");
	createParameter("EX_D_VOZ", "1.6");
	createParameter("T_CONC", "50");
	createParameter("TAU_CONC", "200");
	createParameter("VOR_H", "0.6");

	// Сигналы блока
	createSignal("S_PK", Signal::ST_DOUBLE);
	createSignal("S_T", Signal::ST_DOUBLE);
	createSignal("S_GRF", Signal::ST_DOUBLE);
	createSignal("S_DUP62", Signal::ST_DOUBLE);
	createSignal("S_DUP47", Signal::ST_DOUBLE);

	// Порты блока
	createInputPort(0, "Пр.269", "INFO");
	createInputPort(1, "Пр.220", "INFO");
	createOutputPort(2, "Пр.230", "INFO");
	createOutputPort(3, "Пр.235", "INFO");
	createInputPort(4, "Давл. ресивера", "INFO");
	createInputPort(5, "Давл. насоса", "INFO");

	// Отказы блока

}

bool EX_REEXTRACTOR::process(double t, double h, std::string &error)
{
    // Put your calculations here

    return true;
}

bool EX_REEXTRACTOR::init()
{
    // Put your initialization here
    return true;
}

ICalcElement *Create()
{
    return new EX_REEXTRACTOR();
}

void Release(ICalcElement *block)
{
    if(block->type() == "EX_REEXTRACTOR") delete block;
}

std::string Type()
{
    return "EX_REEXTRACTOR";
}
