#include "MP_CR_MIXER.h"

MP_CR_MIXER::MP_CR_MIXER()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока

	// Сигналы блока

	// Порты блока
    PortIn1 = createInputPort(0, "", "INFO");
    PortIn2 = createInputPort(1, "", "INFO");
    PortOut = createOutputPort(2, "", "INFO");

	// Отказы блока

}

void MP_CR_MIXER::setDataNames()
{
    PortOut->setDataNames(PortIn1->getConnectedPort()->getDataNames());
}

bool MP_CR_MIXER::init(std::string &error, double h)
{
    // Put your initialization here
	
	setDataNames();
    return true;
}

bool MP_CR_MIXER::process(double t, double h, std::string &error)
{
    // Put your calculations here
    std::vector<double> in1, in2, out;
    in1 = PortIn1->getInput();
    in2 = PortIn2->getInput();

    if (in1.size() != in2.size())
    {
        error = "Подключены потоки разного размера!";
        return false;
    }

    out = in1;
    out[0] = in1[0]+in2[0];
    PortOut->setOut(out);
    return true;
}


ICalcElement *Create()
{
    return new MP_CR_MIXER();
}

void Release(ICalcElement *block)
{
    if(block->type() == "MP_CR_MIXER") delete block;
}

std::string Type()
{
    return "MP_CR_MIXER";
}
