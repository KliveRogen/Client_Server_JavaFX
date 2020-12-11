#ifndef IPORT_H
#define IPORT_H

#include <vector>
#include <string>

class ICalcElement;

class IPort {
public:
	virtual ~IPort() {}
	
	virtual int getID() const = 0;
    virtual ICalcElement* getBlock() const = 0;
    virtual std::string getName() const = 0;
    virtual std::string getType() const = 0;
    virtual bool isInput() const = 0;
};

class IOutputPort : public IPort {
public:
	virtual ~IOutputPort() {}
	
	virtual void setDataNames(std::vector<std::string> dn) = 0;
	
	// Работа с данными
    virtual std::vector<double> getOut() const = 0;
    virtual std::vector<double> getNewOut() const = 0;
    virtual std::vector<std::string> getDataNames() const = 0;
    virtual void updateOut() = 0;

    virtual void setOut(int i, double v) = 0;
    virtual void setOut(std::vector<double> v) = 0;

    virtual void setNewOut(int i, double v) = 0;
    virtual void setNewOut(std::vector<double> v) = 0;

    virtual void setArchivable(bool v) = 0;
    virtual bool getArchivable() const = 0;
};

class IInputPort : public IPort {
public:
	virtual ~IInputPort() {}
	
	virtual IOutputPort *getConnectedPort() const = 0;
    virtual void setConnectedPort(IOutputPort *port) = 0;

    virtual bool hasConnection() const = 0;
    virtual int inputDataSize() const = 0;

    virtual std::vector<double> getInput() const = 0;
};

#endif
