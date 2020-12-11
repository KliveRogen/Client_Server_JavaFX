#include "Demux.h"

Demux::Demux()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
    createParameter("DEMUX_PORTS", "2");
    createParameter("DEMUX_GROUPING", "1");

	// Порты блока
    InPort = createInputPort(0, "Выход Demux", "INFO");
}

bool Demux::process(double t, double h, std::string &error)
{
    // Put your calculations here
    std::vector<double> inVect = InPort->getInput();

    size_t i;
    /*
    for(i = 0; i < inVect.size(); i++)  OutPortsVect[i]->setOut(0, inVect[i]); // выгрузка значений
    for(; i < OutPortsVect.size(); i++) OutPortsVect[i]->setOut(0, 0.00); // зануление остатков (не работает)
    */

    int couPort = 0;
    int couGroup = 0;
    double outVal = 0;
    for(i = 0; i < OutPortsVect.size()*paramToInt("DEMUX_GROUPING"); i++) {
        if (i < inVect.size()) {
            outVal = inVect[i];
        } else {
            outVal = 0;
        }
        if (couPort!=OutPortsVect.size()) OutPortsVect[couPort]->setOut(couGroup, outVal);
        couGroup++;
        if (couGroup >= paramToInt("DEMUX_GROUPING")) {
            couGroup = 0;
            couPort++;
        }
    }

    return true;
}

bool Demux::init(std::string &error, double h)
{
    std::vector<std::string> dn;
    if(InPort->getConnectedPort()) { // если подключен вход
        std::vector<std::string> inDn = InPort->getConnectedPort()->getDataNames(); // получили вектор имен

        size_t i = 0;
        std::vector<std::string> outDn;
        int couPort = 0;
        int couGroup = 0;
        std::vector<std::string> groupNames;

        for(i = 0; i < OutPortsVect.size()*paramToInt("DEMUX_GROUPING"); i++) {
            outDn.resize(1);
            if (i < inDn.size()) {
                outDn[0] = inDn[i]; // последовательное присвоение !!!
            } else {
                outDn[0] = "Не подключено";
            }
            groupNames.push_back(outDn[0]);
            couGroup++;
            if (couGroup >= paramToInt("DEMUX_GROUPING")) {
                if (couPort!=OutPortsVect.size()) OutPortsVect[couPort]->setDataNames(groupNames); //установка имен
                couGroup = 0;
                couPort++;
            }
            if (couGroup == 0) {
                groupNames.clear();
            }
        }

        // работает!!
        /*
        for(i = 0; i < OutPortsVect.size(); i++) {
            outDn.resize(1);
            if (i < inDn.size()) {
                outDn[0] = inDn[i]; // последовательное присвоение !!!
            } else {
                outDn[0] = "Не подключено";
            }
            OutPortsVect[i]->setDataNames(outDn); //установка имен
        }
        */

        /*
        //for(size_t i = 0; i < inDn.size(); i++) {
        for(i = 0; i < inDn.size(); i++) {
            //std::vector<std::string> outDn;
            outDn.resize(1);
            outDn[0] = inDn[i]; // последовательное присвоение !!!
            OutPortsVect[i]->setDataNames(outDn); //установка имен
        }
        //i--;

        for(; i < OutPortsVect.size(); i++) {
            //std::vector<std::string> outDn;
            outDn.resize(1);
            outDn[0] = "Не подключено";
            OutPortsVect[i]->setDataNames(outDn);
        }
        */
    } else { // если не подключен то выдаем нули
        for(size_t i = 0; i < OutPortsVect.size(); i++) {
            std::vector<std::string> outDn;
            outDn.resize(1);
            outDn[0] = "Не подключено";
            OutPortsVect[i]->setDataNames(outDn); // ИСПРАВИТЬ!!
        }
    }

    return true;
}

void Demux::loadParameters(QDomElement &domParams)
{
    CalcElement::loadParameters(domParams);

    int nPorts = paramToInt("DEMUX_PORTS"); //сколько портов надо?
    if(nPorts > 0) { // если портов больше 0
        OutPortsVect.resize(nPorts); //кол-во выходных портов под количество указанных
        for(int i = 0; i < nPorts; i++) OutPortsVect[i] = createOutputPort(i+1, "DEMUX_OUT_PORT", "INFO"); // создать необходимое кол-во выходов
    }

}

ICalcElement *Create()
{
    return new Demux();
}

void Release(ICalcElement *block)
{
    if(block->type() == "Demux") delete block;
}

std::string Type()
{
    return "Demux";
}
