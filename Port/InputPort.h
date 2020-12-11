#ifndef PORT_H
#define PORT_H

#include "port_global.h"
#include "../include/IPort.h"

IInputPort* PORTSHARED_EXPORT createInputPort(int id, ICalcElement *b, std::string name, std::string type);
void PORTSHARED_EXPORT deleteInputPort(IInputPort *port);

class InputPort : public IInputPort {
public:
    InputPort(int id, ICalcElement *b, std::string name, std::string type);
    virtual ~InputPort();

    // IPort interface
public:
    virtual int getID() const;
    virtual ICalcElement *getBlock() const;
    virtual std::string getName() const;
    virtual std::string getType() const;
    virtual bool isInput() const;

    // IInputPort interface
public:
    virtual IOutputPort *getConnectedPort() const;
    virtual void setConnectedPort(IOutputPort *port);
    virtual std::vector<double> getInput() const;

    virtual bool hasConnection() const;
    virtual int inputDataSize() const;

private:
    int ID;
    ICalcElement *Block;
    std::string Name;
    std::string Type;

    IOutputPort *ConnectedPort;
};

#endif // PORT_H
