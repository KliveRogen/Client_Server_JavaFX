#ifndef ICALCELEMENT_H
#define ICALCELEMENT_H

class IInputPort;
class IOutputPort;
class QDomElement;

#include <vector>
#include <string>

class ICalcElement {
public:
	// Тип расчетного блока
    enum CalcType
    {
        E_UNKNOWN = 0,
        E_INITVALUES,
        E_NEEDINPUT,
        E_OUTPUTONLY,
        E_INDICATOR,
        E_CHART,
        E_HYDRAULIC
    };

    struct Parameter
    {
        std::string Code;
        std::string Value;
    };

    struct Signal
    {
        enum SignalType
        {
            ST_BOOL = 0,
            ST_CHAR,
            ST_INT,
            ST_LONG,
            ST_FLOAT,
            ST_DOUBLE,
            ST_UCHAR,
            ST_UINT,
            ST_ULONG
        };

        union SignalValue {
            bool boolVal;
            char charVal;
            int intVal;
            long longVal;
            float floatVal;
            double doubleVal;
            unsigned char ucharVal;
            unsigned int uintVal;
            unsigned long ulongVal;
        };

        std::string Code;
        SignalType Type;
        SignalValue Value;
    };

    struct Situation
    {
        std::string Code;
        bool Active;
    };

	virtual ~ICalcElement() {}
	
	// Тип блока
	virtual std::string type() const = 0;
	virtual CalcType getCalcType() const = 0;
	
	// Имя блока
	virtual std::string getName() const = 0;
	virtual void setName(std::string name) = 0;
	
	// ID блока
	virtual int getID() const = 0;
	virtual void setID(int id) = 0;
	
	// Порты блока
	virtual std::vector<IInputPort*> getInputPorts() const = 0;
	virtual std::vector<IOutputPort*> getOutputPorts() const = 0;
	virtual IInputPort* getInputPort(int i) const = 0;
	virtual IOutputPort* getOutputPort(int i) const = 0;
	
	// Параметры блока
	virtual bool setParameter(std::string code, std::string value) = 0;
	virtual bool setParameters(std::vector<std::string> values) = 0;
	virtual Parameter getParameter(std::string code) const = 0;
	virtual std::vector<Parameter> getParameters() const = 0;
    virtual void loadParameters(QDomElement &domParams) = 0;
	
	// Сигналы блока
    virtual Signal* getSignal(std::string code) const = 0;
    virtual std::vector<Signal*> getSignals() const = 0;
	
	// Аварийные ситуации
	virtual bool setSituation(std::string code, bool active) = 0;
	virtual std::vector<Situation> getSituations() const = 0;

    virtual bool process(double t, double h, std::string &error) = 0;
    virtual bool init(std::string &error, double h) = 0;
};

#endif
