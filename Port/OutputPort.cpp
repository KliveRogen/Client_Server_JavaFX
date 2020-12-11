#include "OutputPort.h"

OutputPort::OutputPort(int id, ICalcElement *b, std::string name, std::string type)
{
    ID = id;
    Block = b;
    Name = name;
    Type = type;

    IsArchivable = true;
}

OutputPort::~OutputPort()
{

}

int OutputPort::getID() const
{
    return ID;
}

ICalcElement *OutputPort::getBlock() const
{
    return Block;
}

std::string OutputPort::getName() const
{
    return Name;
}

std::string OutputPort::getType() const
{
    return Type;
}

bool OutputPort::isInput() const
{
    return false;
}

void OutputPort::setDataNames(std::vector<std::string> dn)
{
    if(!DataNames.empty()) return;
    DataNames = dn;

    Out.resize(DataNames.size());
    NewOut.resize(DataNames.size());
}

std::vector<double> OutputPort::getOut() const
{
    return Out;
}

std::vector<double> OutputPort::getNewOut() const
{
    return NewOut;
}

std::vector<std::string> OutputPort::getDataNames() const
{
    return DataNames;
}

void OutputPort::updateOut()
{
    Out = NewOut;
}

void OutputPort::setOut(int i, double v)
{
    Out[i] = v;
}

void OutputPort::setOut(std::vector<double> v)
{
    if(Out.size() == v.size()) Out = v;
}

void OutputPort::setNewOut(int i, double v)
{
    NewOut[i] = v;
}

void OutputPort::setNewOut(std::vector<double> v)
{
    if(NewOut.size() == v.size()) NewOut = v;
}
IOutputPort *createOutputPort(int id, ICalcElement *b, std::string name, std::string type)
{
    return new OutputPort(id, b, name, type);
}

void deleteOutputPort(IOutputPort *port)
{
    delete port;
}


void OutputPort::setArchivable(bool v)
{
    IsArchivable = v;
}

bool OutputPort::getArchivable() const
{
    return IsArchivable;
}
