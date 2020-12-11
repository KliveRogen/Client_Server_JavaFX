#include "InputPort.h"

InputPort::InputPort(int id, ICalcElement *b, std::string name, std::string type)
{
    ID = id;
    Block = b;
    Name = name;
    Type = type;

    ConnectedPort = 0;
}

InputPort::~InputPort()
{

}

int InputPort::getID() const
{
    return ID;
}

ICalcElement *InputPort::getBlock() const
{
    return Block;
}

std::string InputPort::getName() const
{
    return Name;
}

std::string InputPort::getType() const
{
    return Type;
}

bool InputPort::isInput() const
{
    return true;
}

IOutputPort *InputPort::getConnectedPort() const
{
    return ConnectedPort;
}

void InputPort::setConnectedPort(IOutputPort *port)
{
    ConnectedPort = port;
}

std::vector<double> InputPort::getInput() const
{
    std::vector<double> vect;

    if(ConnectedPort) vect = ConnectedPort->getOut();
    return vect;
}

bool InputPort::hasConnection() const
{
    if(ConnectedPort) return true;
    else return false;
}

int InputPort::inputDataSize() const
{
    if(!hasConnection()) return 0;
    else return getInput().size();
}

IInputPort *createInputPort(int id, ICalcElement *b, std::string name, std::string type)
{
    return new InputPort(id, b, name, type);
}

void deleteInputPort(IInputPort *port)
{
    delete port;
}
