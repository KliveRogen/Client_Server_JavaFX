#include "Demux_alt.h"

Demux_alt::Demux_alt()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
    createParameter("Demux_alt_PORTS", "2");

	// Порты блока
    InPort = createInputPort(0, "Выход Demux_alt", "INFO");
}

bool Demux_alt::process(double t, double h, std::string &error)
{
    // Put your calculations here
    std::vector<double> inVect = InPort->getInput();

    size_t i;
    for(i = 0; i < inVect.size(); i++)  OutPortsVect[i]->setOut(0, inVect[i]);
    for(; i < OutPortsVect.size(); i++) OutPortsVect[i]->setOut(0, 0.00);

    return true;
}

bool Demux_alt::init(std::string &error, double h)
{
    std::vector<std::string> dn;
    if(InPort->getConnectedPort()) {
        std::vector<std::string> inDn = InPort->getConnectedPort()->getDataNames();

        size_t i;
        for(size_t i = 0; i < inDn.size(); i++) {
            std::vector<std::string> outDn;
            outDn.resize(1);
            outDn[0] = inDn[i];
            OutPortsVect[i]->setDataNames(outDn);
        }

        for(; i < OutPortsVect.size(); i++) {
            std::vector<std::string> outDn;
            outDn.resize(1);
            outDn[0] = "Не подключено";
            OutPortsVect[i]->setDataNames(outDn);
        }
    }
    else {
        for(size_t i = 0; i < OutPortsVect.size(); i++) {
            std::vector<std::string> outDn;
            outDn.resize(1);
            outDn[0] = "Не подключено";
            OutPortsVect[i]->setDataNames(outDn);
        }
    }

    return true;
}

void Demux_alt::loadParameters(QDomElement &domParams)
{
    CalcElement::loadParameters(domParams);

    int nPorts = paramToInt("Demux_alt_PORTS");
    if(nPorts > 0) {
        OutPortsVect.resize(nPorts);
        for(int i = 0; i < nPorts; i++) OutPortsVect[i] = createOutputPort(i+1, "Demux_alt_OUT_PORT", "INFO");
    }

}

ICalcElement *Create()
{
    return new Demux_alt();
}

void Release(ICalcElement *block)
{
    if(block->type() == "Demux_alt") delete block;
}

std::string Type()
{
    return "Demux_alt";
}
