#include "KTS_AV_MIXER.h"

KTS_AV_MIXER::KTS_AV_MIXER()
{
	// Расчетный тип блока
    BlockCalcType = E_UNKNOWN;

    // Параметры блока
	createParameter("Koeff1", "9000");
	createParameter("Koeff2", "9000");

	// Сигналы блока
	createSignal("MixingSignal", Signal::ST_BOOL);
	createSignal("RotatingSignal", Signal::ST_BOOL);
	createSignal("ContainerSignal", Signal::ST_BOOL);

	// Порты блока
	createInputPort(0, "Гранулят", "INFO");
	createInputPort(1, "Хранилище", "INFO");
	createInputPort(2, "ЛСУ", "INFO");
	createOutputPort(3, "Выход контейнера", "INFO");
	createOutputPort(4, "на линию пресования", "INFO");
	createOutputPort(5, "В ЛСУ", "INFO");

	// Отказы блока
	createSituation("PowerOffErr");
	createSituation("DePressureErr");
	createSituation("PressureGrowUpErr");
	createSituation("MechAndSensorsErr");
	createSituation("LSUConnectionErr");

}

bool KTS_AV_MIXER::process(double t, double h, std::string &error)
{
    // Put your calculations here

    return true;
}

bool KTS_AV_MIXER::init(std::string &error, double h)
{
    // Put your initialization here
    return true;
}

ICalcElement *Create()
{
    return new KTS_AV_MIXER();
}

void Release(ICalcElement *block)
{
    if(block->type() == "KTS_AV_MIXER") delete block;
}

std::string Type()
{
    return "KTS_AV_MIXER";
}
