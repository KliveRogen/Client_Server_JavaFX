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
    const int   dv = 2,     //кол-во датчиков
                dl = 0,     //кол-во индикаторов
                av = 0,     //кол-во ан.ИМ
                al = 1,     //кол-во диск.ИМ
                sit = 2,    //кол-во аварий
                dvsit = 2,  //аварий датч
                dlsit = 0,  //аварий инд
                avsit = 0,  //аварий ИМан
                alsit = 0,  //аварий ИМдиск
                par = 6,    //кол-во параметров
                cpar = 2,   //параметров контроля
                ivpar = 0,  //параметров НУ
                alpar = 2;  //параметров аварий
}

namespace unic //unique constants
{
    const int     tnum = 8;
}
namespace opar //object parameters
{
    double  lvl = 8, // высота аппарата
            lvlm = 8, // измеряемый уровень
            sef = 36.25, // эффективная площадь сечения аппарата
            ksheat = 400*170, //теплопередача на площадь нагрев
            kscool = 69; //теплопередача на площадь охлаждение

}
using namespace mnum;
using namespace typc;
using namespace unic;
using namespace opar;

class TR_VAPE_08T_AN3702 : public CalcElement
{
public:
    TR_VAPE_08T_AN3702();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);
	void setDataNames();

private:
    IInputPort *ins;
    IOutputPort *outinfo, *outinfosen;
    int eflg;

    double  MM[el][tnum],TT[tnum];

    asinf datv[tnum*2],
          datl[1],
          actv[1],
          actl[1];

    alarm AA[tnum*2];
};

#endif // PROJECT_H
