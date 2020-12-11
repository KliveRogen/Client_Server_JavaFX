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
    const int   dv = 3,     //кол-во датчиков
                dl = 1,     //кол-во индикаторов
                av = 1,     //кол-во ан.ИМ
                al = 0,     //кол-во диск.ИМ
                sit = 5,    //кол-во аварий
                dvsit = 3,  //аварий датч
                dlsit = 1,  //аварий инд
                avsit = 1,  //аварий ИМан
                alsit = 0,  //аварий ИМдиск
                par = 6,    //кол-во параметров
                cpar = 2,   //параметров контроля
                ivpar = 0,  //параметров НУ
                alpar = 5;  //параметров аварий
}
namespace unic //unique constants
{
    const double    kpow = 10,
                    tiner = 5,
                    ki = 10,
                    kp = 9,
                    kf = 3000;
}
namespace opar //object parameters
{
    double  lvl = 8, // высота аппарата
            lvlm = 8, // измеряемый уровень
            sef = 1.7, // эффективная площадь сечения аппарата
            ksheat = 400*170, //теплопередача на площадь нагрев
            kscool = 69; //теплопередача на площадь охлаждение

}
using namespace mnum;
using namespace typc;
using namespace unic;
using namespace opar;
class TR_VAPE_12P_AD37502 : public CalcElement
{
public:
    TR_VAPE_12P_AD37502();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);
	void setDataNames();

private:
    IInputPort *ins;
    IOutputPort *outp, *outs, *outinfo, *outinfosen;
    int eflg;

    double  MM[el],TT,fp;

    asinf datv[dv],
          datl[dl],
          actv[av],
          actl[al];

    alarm AA[sit];
};

#endif // PROJECT_H
