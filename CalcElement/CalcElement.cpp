#include "CalcElement.h"
#include "../include/IPort.h"
#include "../Port/InputPort.h"
#include "../Port/OutputPort.h"
#include "ExtData.h"
#include "extdatacontainer.h"
#include <QDomElement>

CalcElement::CalcElement()
{
    Name = "";
    Type = "";

    BlockCalcType = E_UNKNOWN;

    ID = -1;

    ExternalData = new ExtDataContainer;
}

CalcElement::~CalcElement()
{
    delete ExternalData;
}

std::string CalcElement::type() const
{
    return Type;
}

ICalcElement::CalcType CalcElement::getCalcType() const
{
    return BlockCalcType;
}

std::string CalcElement::getName() const
{
    return Name;
}

void CalcElement::setName(std::string name)
{
    Name = name;
}

int CalcElement::getID() const
{
    return ID;
}

void CalcElement::setID(int id)
{
    ID = id;
}

std::vector<IInputPort *> CalcElement::getInputPorts() const
{
    return InputPorts;
}

std::vector<IOutputPort *> CalcElement::getOutputPorts() const
{
    return OutputPorts;
}

IInputPort *CalcElement::getInputPort(int i) const
{
    for(size_t k = 0; k < InputPorts.size(); k++) {
        if(InputPorts[k]->getID() == i) return InputPorts[k];
    }

    return 0;
}

IOutputPort *CalcElement::getOutputPort(int i) const
{
    for(size_t k = 0; k < OutputPorts.size(); k++) {
        if(OutputPorts[k]->getID() == i) return OutputPorts[k];
    }

    return 0;
}

bool CalcElement::setParameter(std::string code, std::string value)
{
    for(size_t i = 0; i < Parameters.size(); i++) {
        if(Parameters[i].Code == code) {
            Parameters[i].Value = value;
            return true;
        }
    }

    return false;
}

bool CalcElement::setParameters(std::vector<std::string> values)
{
    if(Parameters.size() != values.size()) return false;

    for(size_t i = 0; i < Parameters.size(); i++) {
        Parameters[i].Value = values[i];
    }

    return true;
}

ICalcElement::Parameter CalcElement::getParameter(std::string code) const
{
    Parameter p;
    p.Code = "";
    p.Value = "";

    for(size_t i = 0; i < Parameters.size(); i++) {
        if(Parameters[i].Code == code) {
            p.Code = Parameters[i].Code;
            p.Value = Parameters[i].Value;
            break;
        }
    }

    return p;
}

std::vector<ICalcElement::Parameter> CalcElement::getParameters() const
{
    return Parameters;
}

bool CalcElement::setSituation(std::string code, bool active)
{
    for(size_t i = 0; i < Situations.size(); i++) {
        if(Situations[i].Code == code) {
            Situations[i].Active = active;
            return true;
        }
    }

    return false;
}

std::vector<ICalcElement::Situation> CalcElement::getSituations() const
{
    return Situations;
}

ICalcElement::Signal *CalcElement::getSignal(std::string code) const
{
    for(size_t i = 0; i < Signals.size(); i++) {
        if(Signals[i]->Code == code) return Signals[i];
    }

    return 0;
}

std::vector<ICalcElement::Signal *> CalcElement::getSignals() const
{
    return Signals;
}

void CalcElement::loadParameters(QDomElement &domParams)
{
    if(domParams.tagName() != "Parameters") return;

    QDomElement paramElem = domParams.firstChildElement("Parameter");
    while(!paramElem.isNull()) {
        QString code, value;
        code = paramElem.attribute("Code");
        value = paramElem.attribute("Value");

        setParameter(code.toStdString(), value.toStdString());

        paramElem = paramElem.nextSiblingElement("Parameter");
    }
}

void CalcElement::loadExtData(QDomElement &domExtdata)
{
    /*QDomNode opcInfo = domExtdata.firstChild();
    QDomElement data = opcInfo.toElement();

    if(!data.isNull())
    {
        ExtData* info = new ExtData();
        info->load(data);
        ExternalData.append(info);
    }*/

    ExternalData->loadExtData(domExtdata);
}

ExtData *CalcElement::getExtData(const QString &name)
{
    /*for (int i = 0; i != ExternalData.size(); ++i)
    {
        if (ExternalData[i]->getName() == name)
        {
            return ExternalData[i];
        }
    }
    ExternalData.append(new ExtData(name));
    return ExternalData.last();*/

    return ExternalData->getExtData(name);
}

void CalcElement::createParameter(std::string code, std::string value)
{
    Parameter p;
    p.Code = code;
    p.Value = value;

    Parameters.push_back(p);
}

void CalcElement::createSituation(std::string code)
{
    Situation sit;
    sit.Code = code;
    sit.Active = false;

    Situations.push_back(sit);
}

ICalcElement::Signal *CalcElement::createSignal(std::string code, Signal::SignalType type)
{
    Signal *s = new Signal();
    s->Code = code;
    s->Type = type;
    s->Value.longVal = 0;

    Signals.push_back(s);

    return s;
}

IInputPort *CalcElement::createInputPort(int id, std::string name, std::string type)
{
    IInputPort *port = ::createInputPort(id, this, name, type);

    InputPorts.push_back(port);
    return port;
}

IOutputPort *CalcElement::createOutputPort(int id, std::string name, std::string type)
{
    IOutputPort *port = ::createOutputPort(id, this, name, type);

    OutputPorts.push_back(port);
    return port;
}

double CalcElement::paramToDouble(ICalcElement::Parameter p)
{
    return atof(p.Value.c_str());
}

double CalcElement::paramToDouble(std::string code)
{
    ICalcElement::Parameter p = getParameter(code);

    if(p.Code != "")
    {
        return atof(p.Value.c_str());
    }
    // TODO: log(нет параметра с кодом код)
    return 0;
}

int CalcElement::paramToInt(ICalcElement::Parameter p)
{
    return atoi(p.Value.c_str());
}

int CalcElement::paramToInt(std::string code)
{
    ICalcElement::Parameter p = getParameter(code);

    if(p.Code != "")
    {
        return atoi(p.Value.c_str());
    }
    // TODO: log(нет параметра)
    return 0;
}

bool CalcElement::isSituationActive(std::string code)
{
    for(size_t i = 0; i < Situations.size(); i++) {
        if(Situations[i].Code == code) return Situations[i].Active;
    }

    return false;
}
