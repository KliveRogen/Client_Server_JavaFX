#ifndef CALCELEMENT_H
#define CALCELEMENT_H

#include "calcelement_global.h"
#include "../include/ICalcElement.h"

#include <string>
#include <QList>

class ExtData;
class ExtDataContainer;
class CALCELEMENTSHARED_EXPORT CalcElement : public ICalcElement
{

public:
    CalcElement();
    virtual ~CalcElement();

    // ICalcElement interface
public:
    virtual std::string type() const;
    virtual CalcType getCalcType() const;
    virtual std::string getName() const;
    virtual void setName(std::string name);
    virtual int getID() const;
    virtual void setID(int id);
    virtual std::vector<IInputPort *> getInputPorts() const;
    virtual std::vector<IOutputPort *> getOutputPorts() const;
    virtual IInputPort *getInputPort(int i) const;
    virtual IOutputPort *getOutputPort(int i) const;
    virtual bool setParameter(std::string code, std::string value);
    virtual bool setParameters(std::vector<std::string> values);
    virtual Parameter getParameter(std::string code) const;
    virtual std::vector<Parameter> getParameters() const;
    virtual bool setSituation(std::string code, bool active);
    virtual std::vector<Situation> getSituations() const;
    virtual Signal *getSignal(std::string code) const;
    virtual std::vector<Signal *> getSignals() const;
    virtual void loadParameters(QDomElement &domParams);

    //ExternalData
    virtual void loadExtData(QDomElement& domExtdata);
    virtual ExtData* getExtData(const QString &name);

protected:
    void createParameter(std::string code, std::string value);
    void createSituation(std::string code);
    Signal* createSignal(std::string code, Signal::SignalType type);
    IInputPort* createInputPort(int id, std::string name, std::string type);
    IOutputPort* createOutputPort(int id, std::string name, std::string type);
    double paramToDouble(Parameter p);
    double paramToDouble(std::string code);
    int paramToInt(Parameter p);
    int paramToInt(std::string code);

    bool isSituationActive(std::string code);

    std::string Type;
    std::string Name;

    int ID;
    CalcType BlockCalcType;

    // Порты
    std::vector<IInputPort*> InputPorts;
    std::vector<IOutputPort*> OutputPorts;
    std::vector<Parameter> Parameters;
    std::vector<Signal*> Signals;
    std::vector<Situation> Situations;

    //extdata
   // QList<ExtData*> ExternalData;
    ExtDataContainer *ExternalData;
};

#endif // CALCELEMENT_H
