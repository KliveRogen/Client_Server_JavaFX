#ifndef PROJECT_H
#define PROJECT_H
#define NumComp 5 //число компонентов в смеси
#define TopValue 0.95 //верхняя граница (в долях) заполнения дозатора
#define MaxMassPowder 1.5 //максимальная масса порошка в контейнере

#include "../../CalcElement/CalcElement.h"
#include "../../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

struct dispenser {
    double maxCapac[NumComp]; // Максимальная вместимость каждого дозатора. В частном случае можно принять их равными
    double FillRate[NumComp]; //Массовый расход при заполеннии дозаторов кг/с
    double FillInert; //Постоянная времени дозаторов
    double delayFiling; // Время запаздывания для перемещения между дозаторами
    double containerFillingTime;// общее время заполнения контейнера
    double currentDispenCapac[NumComp]= {20,20,20,20,20};// текущее значение заполненности дозаторов
    bool FlagCalcTime;// разрешение на выполнение оперций блоком
    double ConcentrIn[NumComp];// Концентрация на входе аппарата (задана извне)
    bool ControlChanges;// Контроль смены сигнала на выходе и начало заполнения-формирования нового контейнера
    int PriorFill[NumComp] = {0,0,0,0,0};// Приоритет очередности заполнения дозаторов
    int NumNotZero;// число компонентов с ненулевой концентрацией
    int numCalcNodes;// число узлов для расчёта
    int currenCalcNodes;// текущее число узлов расчёта
    double PowderMass;// Масса порошка
    double MaxMass=MaxMassPowder;// Максимальная масса порошка в контейнере (по условиям)
};

class Dispenser : public CalcElement
{
public:
    Dispenser();
    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);
	void setDataNames();
	
private:
	// Put your variables here
    IInputPort *in, *contr, *contrFromVortex;
    IOutputPort *out, *OUTInform;
    Signal *UO2, *PuO2, *C, *UN , *PuN;
};

dispenser dispen;

#endif // PROJECT_H
