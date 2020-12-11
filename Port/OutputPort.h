#ifndef OUTPUTPORT_H
#define OUTPUTPORT_H

#include "port_global.h"
#include "../include/IPort.h"

IOutputPort* PORTSHARED_EXPORT createOutputPort(int id, ICalcElement *b, std::string name, std::string type);
void PORTSHARED_EXPORT deleteOutputPort(IOutputPort *port);

class OutputPort : public IOutputPort
{
public:
    OutputPort(int id, ICalcElement *b, std::string name, std::string type);
    virtual ~OutputPort();

    // IPort interface
public:
    virtual int getID() const;
    virtual ICalcElement *getBlock() const;
    virtual std::string getName() const;
    virtual std::string getType() const;
    virtual bool isInput() const;

    // IOutputPort interface
public:
    virtual void setDataNames(std::vector<std::string> dn);
    virtual std::vector<double> getOut() const;
    virtual std::vector<double> getNewOut() const;
    virtual std::vector<std::string> getDataNames() const;
    virtual void updateOut();
    virtual void setOut(int i, double v);
    virtual void setOut(std::vector<double> v);
    virtual void setNewOut(int i, double v);
    virtual void setNewOut(std::vector<double> v);
    virtual void setArchivable(bool v);
    virtual bool getArchivable() const;

private:
    int ID;
    ICalcElement *Block;
    std::string Name;
    std::string Type;

    std::vector<std::string> DataNames;
    std::vector<double> Out;
    std::vector<double> NewOut;

    bool IsArchivable;
};

#endif // OUTPUTPORT_H
