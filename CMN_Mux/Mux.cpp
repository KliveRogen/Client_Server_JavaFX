#include "Mux.h"

Mux::Mux()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
    createParameter("MUX_PORTS", "2");

	// Порты блока
    OutPort = createOutputPort(0, "Выход Mux", "INFO");
}

bool Mux::process(double t, double h, std::string &error)
{
    // Put your calculations here
    int index = 0;

    for(size_t i = 0; i < InPortsVect.size(); i++) {
//        std::vector<double> inV = InPortsVect[i]->getInput();
//        if(inV.empty()) OutVect[i] = 0;
//        else OutVect[i] = inV[0];

        IInputPort *inP = InPortsVect[i];
        if(inP->getConnectedPort()) {
            std::vector<double> in = inP->getConnectedPort()->getOut();
            for(size_t j = 0; j < in.size(); j++) {
                OutVect[index] = in[j];
                index++;
            }
        }
        else {
            OutVect[index] = 0;
            index++;
        }
    }

    OutPort->setOut(OutVect);

    return true;
}

bool Mux::init(std::string &error, double h)
{
    std::vector<std::string> dn;
    for(size_t i = 0; i < InPortsVect.size(); i++) {
        IInputPort *inPort = InPortsVect[i];

        if(inPort->getConnectedPort()) {
            std::vector<std::string> conDN = inPort->getConnectedPort()->getDataNames();
            for(size_t j = 0; j < conDN.size(); j++) {
                dn.push_back(conDN[j]);
            }
        }
        else dn.push_back("Не подключено");
    }

    OutPort->setDataNames(dn);
    OutVect.resize(dn.size());

//    std::vector<std::string> dn;
//    dn.resize(InPortsVect.size());
//    OutVect.resize(InPortsVect.size());
//    for(size_t i = 0; i < InPortsVect.size(); i++) {
//        if(InPortsVect[i]->getConnectedPort()) {
//            std::vector<std::string> conDN = InPortsVect[i]->getConnectedPort()->getDataNames();
//            if(conDN.empty()) dn[i] = "Без названия";
//            else dn[i] = conDN[0];
//            // dn[i] = InPortsVect[i]->getConnectedPort()->getDataNames().front();
//        }
//        else dn[i] = "Не подключено";
//    }

//    OutPort->setDataNames(dn);

    return true;
}

void Mux::loadParameters(QDomElement &domParams)
{
    CalcElement::loadParameters(domParams);

    int nPorts = paramToInt("MUX_PORTS");
    if(nPorts > 0) {
        InPortsVect.resize(nPorts);
        for(int i = 0; i < nPorts; i++) InPortsVect[i] = createInputPort(i+1, "MUX_IN_PORT", "INFO");
    }

}

ICalcElement *Create()
{
    return new Mux();
}

void Release(ICalcElement *block)
{
    if(block->type() == "Mux") delete block;
}

std::string Type()
{
    return "Mux";
}
