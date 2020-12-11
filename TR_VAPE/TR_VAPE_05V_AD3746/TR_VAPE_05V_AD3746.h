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
    const int   dv = 9,     //кол-во датчиков
                dl = 0,     //кол-во индикаторов
                av = 2,     //кол-во ан.ИМ
                al = 0,     //кол-во диск.ИМ
                sit = 11,    //кол-во аварий
                dvsit = 9,  //аварий датч
                dlsit = 0,  //аварий инд
                avsit = 2,  //аварий ИМан
                alsit = 0,  //аварий ИМдиск
                par = 31,    //кол-во параметров
                cpar = 3,   //параметров контроля
                ivpar = 6,  //параметров НУ
                alpar = 11;  //параметров аварий
}

namespace unic //unique constants
{
    const double    qcool = 1e-2;
}
namespace opar //object parameters
{
    double  lvl = 8, // высота аппарата
            lvlm = 4.7, // измеряемый уровень
            sef = 1.7, // эффективная площадь сечения аппарата
            ksheat = 400*170, //теплопередача на площадь нагрев
            kscool = 69; //теплопередача на площадь охлаждение
}
using namespace mnum;
using namespace typc;
using namespace unic;
using namespace opar;
class TR_VAPE_05V_AD3746 : public CalcElement
{
public:
    TR_VAPE_05V_AD3746();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);
    void setDataNames();

private:
    IInputPort *ins,*inv;
    IOutputPort *outv, *outs, *outinfo, *outinfosen;
    int eflg;

    double  MM[el],TT,kmout[el],kmvape[el],ph1_i;

    asinf datv[dv],
          datl[dl],
          actv[av],
          actl[al];

    alarm AA[sit];
};

#endif // PROJECT_H
