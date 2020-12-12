#ifndef PROJECT_H
#define PROJECT_H
#define PI 3.1415926535 //число ПИ
#define Norm 2*PI*0.01 // Нормирующий коэффициент для распреления иглпо углу
#define NumComp 5 //число компонентов в смеси
#define NumCompCrash 4 //число компонентов в смеси
#define TopValue 0.95 //доля от максимального значения радиуса
#define NormDistrib 18 //значение к-т на которое будет делиться выходная величина ф-ии rand(). Рекомендуется брать кратным 3, чтобы обеспечить равномерно интервала
#define TimeStep 0.1 // величина в ремени, в течение которого переход игл м/у слоями не происходит
#define LimBound 0.09 // граница для компенсации нелинейности нагрева на начальном этапе расчёта

#include "../../CalcElement/CalcElement.h"
#include "../../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class MFR_VortexLayer : public CalcElement
{
public:
    MFR_VortexLayer();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);
	void setDataNames();
	
private:
	// Put your variables here
    IInputPort *in;
    IOutputPort *out,*OUTInform,*contrToDisp ;
    double Concentration[5] = {0.10,0.20,0.40,0.20,0.10};
    double containerVolume, HeightOneCell, HeightSubmergedPart,powderMassOneCell,cellSurfaceArea,meanCapacity,remainingPowderMass;
    int NumCell,b,NumNotZero,nodesMix,currenNodesMix,startCalc;
    double *Efficiency;
    double **changePowderWeight;
    double *particleCrushing;
    double *temperature;
    double *temperatureChange;
    double **needleMovement;
    double *numberNeedlesOneLayer;
    double **powderWeightContainer; //%6 двумерных таблиц для смешивания. Первая отображает массу всех компонентов в кажой ячейки в зависимости от времени
    // % Последующие таблицы отображают массу i-ого компонента (UO2,PuO2,C,UN,PuN) в каждоя ячейки в % зависимости от времени

    struct container{
        int needleNum;
        double powderMaxMass; //Масса порошков в контейнере максимальная (в кг)
        double Height; // высота рабочей зоны контейнера
        double HeightSubmergedPart = 4; // высота рабочей зоны ABC
        double Amplitude = 0.2;
        double Square;
        int needleNumCrit; //критическое количество ферромагнитных игл для одного слоя %входной парметр???
        int timeMix = 7.5*60;
        double reverseFlow[5] = {0.1,0.15,0.15,0.15,0.15}; //Величина обратного потока i-ого компонента
        double particleCrushing[4] = {70,20,9,1};
        double selectionFunction[4] = {0.04, 0.01,0.008,0}; //Функция отбора 1-ого (самого крупного), 2-ого и 3-его класса крупности.
        double destructionFunction[4][4] = {{0.0,0.00,0.00,0},
                                            {0.6,0.00,0.00,0},
                                            {0.3,0.70,0.00,0},
                                            {0.1,0.30,1.00,0}}; //Функция разрушения из одного элемента в другой (Столбец - один класс, по строкам отложена доля
        //частиц, переходящих в новый класс из первоначального
        double heatCapacity[5] = {242,75,750,190,195}; //Теплоёмкости UO2,PuO2,C,UN,PuN Дж//кг*град
        double needleCurrent = 25; //Значение тока
        double contourIntegrityFactor = 0.8; //доля рассматриваемого промежутка, когда иглы образуют замкнутый контур
        double convectiveHeatTransferCoefficient = 70; //Коэ-т конвективного теплообмена
        double waterTemperature; //Температура охлаждающей воды %входной парметр
        double resistanceNeedle = 1.2; //Электрическое сопротивление ферромагнитных игл.
        double sumForcesRadius = 50;
        double sumForcesAngle = 100;
        double mNeedles = 5;
        double temperatureStart = 30;
        double powderHeatTransferCoefficient = 4;

    };


    container containerPar;
};

#endif // PROJECT_H
