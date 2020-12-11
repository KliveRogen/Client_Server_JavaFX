#include "TR_VAPE_16S_IS.h"

TR_VAPE_16S_IS::TR_VAPE_16S_IS()
{
	// Расчетный тип блока
    BlockCalcType = E_OUTPUTONLY;

    // Параметры блока
    createParameter("flow", "0");
    createParameter("concU", "0");
    createParameter("concHno3", "0");
    createParameter("temp", "0");

	// Сигналы блока

	// Порты блока
    out = createOutputPort(0, "out", "INFO");

    // Отказы блока

}

void TR_VAPE_16S_IS::setDataNames()
{
    std::vector<std::string> out1;
    out1.push_back("Расход раствора, кг/с");
    out1.push_back("Концентрация U, г/л");
    out1.push_back("Концентрация HNO3, г/л");
    out1.push_back("Температура раствора, оС");
    out->setDataNames(out1);
}

bool TR_VAPE_16S_IS::init(std::string &error, double h)
{
	setDataNames();
    for (int i = 0; i < Parameters.size(); i++) {
        if ( paramToDouble(Parameters[i]) < 0 ) {
            error = "Ошибка блока Нач.раств.:\nОшибка в заполнении исходных данных!";
            return false;
        }
        out->setOut(i,paramToDouble(Parameters[i]));
    }
    if ( (paramToDouble("concU") + paramToDouble("concHno3"))*COEFF_MASS > paramToDouble("flow") ) {
        error = "Ошибка блока Нач.раств.:\nОшибка в заполнении исходных данных!";
        return false;
    }
    return true;
}

bool TR_VAPE_16S_IS::process(double t, double h, std::string &error)
{
    for (int i = 0; i < Parameters.size(); i++) {
        if ( paramToDouble(Parameters[i]) < 0 ) {
            error = "Ошибка блока Нач.раств.:\nОшибка в заполнении исходных данных!";
            return false;
        }
        out->setOut(i,paramToDouble(Parameters[i]));
    }
    if ( (paramToDouble("concU") + paramToDouble("concHno3"))*kcd > paramToDouble("flow") ) {
        error = "Ошибка блока Нач.раств.:\nОшибка в заполнении исходных данных!";
        return false;
    }
    return true;
}


ICalcElement *Create()
{
    return new TR_VAPE_16S_IS();
}

void Release(ICalcElement *block)
{
    if(block->type() == "TR_VAPE_16S_IS") delete block;
}

std::string Type()
{
    return "TR_VAPE_16S_IS";
}
