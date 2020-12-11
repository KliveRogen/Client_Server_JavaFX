#ifndef PROJECT_H
#define PROJECT_H

#include "../CalcElement/CalcElement.h"
#include "../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class EX_REEXTRACTOR : public CalcElement
{
public:
    EX_REEXTRACTOR();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init();

private:
	// Put your variables here
};

#endif // PROJECT_H
