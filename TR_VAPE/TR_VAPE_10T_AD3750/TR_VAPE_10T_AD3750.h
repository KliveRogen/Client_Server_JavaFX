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
                dl = 2,     //кол-во индикаторов
                av = 0,     //кол-во ан.ИМ
                al = 0,     //кол-во диск.ИМ
                sit = 4,    //кол-во аварий
                dvsit = 2,  //аварий датч
                dlsit = 2,  //аварий инд
                avsit = 0,  //аварий ИМан
                alsit = 0,  //аварий ИМдиск
                par = 8,    //кол-во параметров
                cpar = 0,   //параметров контроля
                ivpar = 6,  //параметров НУ
                alpar = 4;  //параметров аварий
}

namespace unic //unique constants
{
    const double    none1 = 0;
}
namespace opar //object parameters
{
    double  lvl = 8, // высота аппарата
            lvlm = 8, // измеряемый уровень
            sef = 0.6375, // эффективная площадь сечения аппарата
            ksheat = 400*170, //теплопередача на площадь нагрев
            kscool = 69; //теплопередача на площадь охлаждение
}
using namespace mnum;
using namespace typc;
using namespace unic;
using namespace opar;
class TR_VAPE_10T_AD3750 : public CalcElement
{
public:
    TR_VAPE_10T_AD3750();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);
	void setDataNames();

private:
    IInputPort *inp1,*inp2,*ins;
    IOutputPort *outp1, *outp2, *outinfo, *outinfosen;
    int eflg;

    double  MM[el],TT;

    asinf datv[dv],
          datl[dl],
          actv[av],
          actl[al];

    alarm AA[sit];
};

#endif // PROJECT_H
