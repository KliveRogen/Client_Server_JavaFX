#ifndef PROJECT_H
#define PROJECT_H

#include "../../CalcElement/CalcElement.h"
#include "../../include/IPort.h"
#include "../TR_VAPE_CALC/tr_vape_calc.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

namespace mnum //model numbers
{
    const int   dv = 0,     //кол-во датчиков
                dl = 0,     //кол-во индикаторов
                av = 0,     //кол-во ан.ИМ
                al = 0,     //кол-во диск.ИМ
                sit = 0,    //кол-во аварий
                dvsit = 0,  //аварий датч
                dlsit = 0,  //аварий инд
                avsit = 0,  //аварий ИМан
                alsit = 0,  //аварий ИМдиск
                par = 4,    //кол-во параметров
                cpar = 0,   //параметров контроля
                ivpar = 4,  //параметров НУ
                alpar = 0;  //параметров аварий
}
namespace unic //unique constants
{
    const double    no1 = 0;
}
namespace opar //object parameters
{
    double  no2 = 0;
}

using namespace mnum;
using namespace typc;
using namespace unic;
using namespace opar;

class TR_VAPE_16S_IS : public CalcElement
{
public:
    TR_VAPE_16S_IS();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);
	void setDataNames();

private:
    IOutputPort *out;
    int eflg;
};

#endif // PROJECT_H
