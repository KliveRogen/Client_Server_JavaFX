#include <string>
#include <iostream>
#include <stdio.h>
#include <vector>

#define COEFF_MASS 1e-9
#define NUM_EL 3
#define DENS 1e3

struct digUnit {
    bool rawVal,outVal;
    int opt;
    bool trig;
    void rout();
};

struct analUnit {
    double inValBuf;
    bool lagFlag;
    double inVal,outVal,coeffMul,coeffSum,rawVal;
    int opt;
    bool trig;
    void rout(bool isAct);
};

struct flow {
    double mass,conc[NUM_EL],temp;
};

// tank
class Tank {
    public:
        double mass,conc[NUM_EL],temp;
        //double tempIn;
        std::vector<flow> flowIn;
        std::vector<flow> flowOut;
        //double massSum();
        void tempMix();
        void massCalc();
};

class Pump {
    public:
        flow flowIn,flowOut;
        double flowReq;
        void pumping();
};


class HeatEx {
    public:
        flow flowSolIn,flowSolOut;
        flow flowHeatIn,flowHeatOut;
        double coeffHeatEx;
        void heatExchange();
};

class Vape {
    public:

};

class Mixer {
    public:
        flow flowInSum;
        std::vector<flow> flowIn;
        std::vector<flow> flowOut;
        void mixFlowIn();
        void splitFlowOut();
        void setFlowOut(int num);
};





/*
double Tank::massSum() {
    double value;
    value = 0;
    for (int i = 0; i < NUM_EL; i++) {
        value = value + mass[i];
    }
    return value;
}
*/



//namespaces
namespace typc //typical constants
{
    const double    dens = 1e3,     // плотность воды
                    lamb = 2.3e6,   // теплота парообразования
                    ta = 20+273,    // температура окр
                    tb = 110+273,   // температура кипения
                    pa = 1e5,       // давление окр
                    hcap = 4220,    // теплоемкость
                    kcd = 1e-9,     // г/л vs масс%
                    lvltst[4] = {0.1,  0.9,  1,  1.1},
                    cap = 1e-12,
                    coeffPress = 0.098e6;
    const int       el = 3;
}
struct asinf //info for actuators and sensors
{
    double  bound[2],
            val[2];
};
struct alarm //info for alarms
{
    bool    trig,
            eflag;
    int     opt;
    double  ecou,
            eval_i[2];
};

struct actOrSen
{
    bool    senOrAct,   // 0 - Д, 1 - ИМ
            digOrAnal;  // 0 - цифр, 1 - анал
    double  analVal[2]; //значения 0 - пришедшее 1 - итоговое
    double  coeffMul,   //a*x+b
            coeffSum;
    bool    digVal[2];  //значения 0 - пришедшее 1 - итоговое
    bool    emerTrig;   //инициация отказа
    int     emerOpt;    //вариант отказа
    bool    emerFlag;   //флаг для обработки отказов
    int     emerCou;    //счетчик для обработки отказов
    double  emerBuf[2]; //массив  для обработки отказов
    bool    emerDigBuf;

};
//funcs
double dverr(double vol, int nerr, double h, alarm *AA);
int dlerr(int vol, int nerr);
double averr(double vol, int nerr, alarm *AA);
int alerr(int vol, int nerr, alarm *AA);
//void emerRouteFun(actOrSen *unit);
void actOrSenRoutine(actOrSen *unit);
std::string errorMsgCreator(char code, std::string name);
//void tankMassExchange(double massElem[5], flowIn, flowOut, );
