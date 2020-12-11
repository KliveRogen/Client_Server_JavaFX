#ifndef PROJECT_H
#define PROJECT_H

#include <map>
#include <QList>
#include "../CalcElement/CalcElement.h"
#include "../include/IPort.h"

extern "C" ICalcElement* __declspec(dllexport) Create();
extern "C" void __declspec(dllexport) Release(ICalcElement *block);
extern "C" std::string __declspec(dllexport) Type();

class MP_CR_Crystallizer : public CalcElement
{
public:
    MP_CR_Crystallizer();

    // ICalcElement interface
public:
    virtual bool process(double t, double h, std::string &error);
    virtual bool init(std::string &error, double h);

    struct Container
    {
        double Volume;
        double Temperature;
        double CGNU;
        double CHNO3;
        double VGNU;
        double VHNO3;
    };

private:
    void setDataNames();
    std::vector<double> solver();
private:

    // Put your variables here
    IInputPort *PortInLiq, *PortInWash, *PortInHeat;
    IOutputPort *PortOutMPR, *PortOutCryst, *PortOutSens;

    double step;
    int num_cell;
    int num_cellpr;
    int num_dydt;

    double Lwall_kr;
    double Lwall_pr;

    std::map<uint,double> m_Y_out;
    std::map<uint,double> dydt;

    // Накопительные емкости
    int ThreeWayCrane;
    bool ThreeWayCraneError;
    Container *ContainerLeft;
    Container *ContainerRight;

    QList<Container *> Containers;

    //!!!!!!!!!!!!!!!!входные переменные
    //входной исходный водный поток
    double Input_Fin; // расход, л/ч
    double Input_Tin; // температура раствора, oC
    double Input_Cu; // концентрация урана, г/л
    double Input_Cpu; // концентрация плутония, г/л
    double Input_Chno3; // концентрация азотной кислоты, г/л
    double Input_Cnp; // концентрация нептуния, г/л

    double Calculate_mat_Fout;
    double Hliq;
    double ukl;

    //входной промывной поток
    double Input_pr_Fin; // расход, л/ч
    double Input_pr_Tin; // температура раствора, oC
    double Input_pr_Cu; // концентрация урана, г/л
    double Input_pr_Cpu; // концентрация плутония, г/л
    double Input_pr_Chno3; // концентрация азотной кислоты, г/л
    double Input_pr_Cnp; // концентрация нептуния, г/л

    std::map<uint,double> Twall_in;
    std::map<uint,double> Twall;
    std::map<uint,double> Twallpr;

    //!!!!!!!!!!!!! Для кристаллизационной зоны !!!!!!!
    //переменные вход-выход
    std::map<uint,double> mF;      // м3/с
    std::map<uint,double> mTin;    // oС
    std::map<uint,double> mCu;     // доля
    std::map<uint,double> mChno3;  // доля
    std::map<uint,double> mzet;
    std::map<uint,double> mR;    // %м
    std::map<uint,double> mW;

    //выходные переменные
    std::map<uint,double> mTtr;
    std::map<uint,double> mTout;
    std::map<uint,double> mdCun;
    std::map<uint,double> mdChno3;
    std::map<uint,double> mUc;
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    //!!!!!!!!!!!!! Для промывной зоны !!!!!!!
    //переменные вход-выход
    std::map<uint,double> mprF;      // м3/с
    std::map<uint,double> mprTin;    // oС
    std::map<uint,double> mprCu;     // доля
    std::map<uint,double> mprChno3;  // доля
    std::map<uint,double> mprzet;
    std::map<uint,double> mprR;    // %м
    std::map<uint,double> mprW;

    //выходные переменные
    std::map<uint,double> mprTtr;
    std::map<uint,double> mprTout;
    std::map<uint,double> mprdCun;
    std::map<uint,double> mprdChno3;
    std::map<uint,double> mprUc;
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


    //выходной маточный поток
    double Output_mat_Fout;
    double Output_mat_Tout;
    double Output_mat_Cu;
    double Output_mat_Cpu;
    double Output_mat_Chno3;
    double Output_mat_Cnp;
    double Output_mat_GNu;

    //выходной промывной поток
    double Output_pr_Fout;
    double Output_pr_Tout;
    double Output_pr_Cu;
    double Output_pr_Cpu;
    double Output_pr_Chno3;
    double Output_pr_Cnp;

    //выходной маточный + промывной
    double Output_Fout;
    double Output_Tout;
    double Output_Cu;
    double Output_Cpu;
    double Output_Chno3;
    double Output_Cnp;

    //выходной поток твердой фазы
    double Output_GNu;
    double Output_GNu_tmp;
    double realGNU;

    //проверка баланса
    double balance_error_mat;
    double balance_error_common;
    double balance_error_hno3_mat;
    double balance_error_hno3_common;

    //параметры
    double _start_Lkrzone;
    double _start_Lprom;
    double _start_D;
    double _start_dotv_out;
    double _start_akl;
    double _start_ap;
    double _start_ag;
    double _start_Kkr;
    double _start_n;
    double _start_Tplav;
    double _start_Kcore;

    //буфферные переменные с сохранением между циклами
    double kCu_buf;
    double kCpu_buf;
    double kCnp_buf;

    double kprCu_buf;
    double kprCpu_buf;
    double kprCnp_buf;

};

#endif // PROJECT_H
