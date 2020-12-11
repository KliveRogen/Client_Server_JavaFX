#include "MP_CR_Crystallizer.h"
#include <math.h>

MP_CR_Crystallizer::MP_CR_Crystallizer()
{
    // Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
    createParameter("CrystStartTemp", "40");
    createParameter("CrystStartCHNO3", "180");
    createParameter("WashStartTemp", "40");
    createParameter("WashStartCHNO3", "548.1");
    createParameter("CrystZoneHeight", "1.5");
    createParameter("CrystWallHeight", "0.4");
    createParameter("WashZoneHeight", "1.7");
    createParameter("WashWallHeight", "1.2");
    createParameter("Diam", "0.03");
    createParameter("LiquidStartLevel", "3.2");
    createParameter("OutletHoleDiam", "20E-3");
    createParameter("start_akl", "2.772E-4");
    createParameter("AHeatToPipe", "100");
    createParameter("APipeToLiquid", "100");
    createParameter("CrystCellNum", "10");
    createParameter("WashCellNum", "10");
    createParameter("KCryst", "4E-8");
    createParameter("CrystCenters", "4E10");
    createParameter("TempMelt", "34");
    createParameter("Kcore", "0.1");

    // Сигналы блока

    // Порты блока
    PortInWash = createInputPort(0, "Промывной раствор", "INFO");
    PortInLiq = createInputPort(1, "Питающий раствор", "INFO");
    PortInHeat = createInputPort(2, "Рубашки", "INFO");
    PortOutMPR = createOutputPort(3, "МПР", "INFO");
    PortOutCryst = createOutputPort(4, "Кристаллы", "INFO");
    PortOutSens = createOutputPort(5, "Датчики", "INFO");

    // Отказы блока

    createSituation("ES_CRANE_ERR");
    createSituation("ES_HIGHLEVELSENSORS_ERR");
    createSituation("ES_CONTAINERDRAIN_ERR");
    createSituation("ES_CONTAINERTEMPER_ERR");
    createSituation("ES_LINELEAK_ERR");
    createSituation("ES_POWEROFF_ERR");
}

void MP_CR_Crystallizer::setDataNames()
{
    std::vector<std::string> dnMPR, dnCryst, dnSens;

    dnMPR.push_back("Расход, м3/с");
    dnMPR.push_back("Температура раствора, °C");
    dnMPR.push_back("Концентрация урана, г/л");
    dnMPR.push_back("Концентрация плутония, г/л");
    dnMPR.push_back("Концентрация нептуния, г/л");
    dnMPR.push_back("Концентрация азотной кислоты, г/л");
    PortOutMPR->setDataNames(dnMPR);

    dnCryst.push_back("Выход кристаллов, кг/с");
    dnCryst.push_back("Объем кристаллов в левой накопительной емкости, %");
    dnCryst.push_back("Объем кристаллов в правой накопительной емкости, %");
    PortOutCryst->setDataNames(dnCryst);

    dnSens.push_back("Высота жидкости в кристаллизаторе, м");
    dnSens.push_back("Температура, °C");
    dnSens.push_back("Температура, °C");
    dnSens.push_back("Температура, °C");
    dnSens.push_back("Температура, °C");
    PortOutSens->setDataNames(dnSens);
}

bool MP_CR_Crystallizer::init(std::string &error, double h)
{
    // Put your initialization here
    setDataNames();

    double CrystStartTemp = paramToDouble("CrystStartTemp");    //Начальная температура зоны кристаллизации, oC
    double CrystStartCHNO3 = paramToDouble("CrystStartCHNO3");    //Начальная концентрация HNO3 в зоне кристаллизации, г/л
    double WashStartTemp = paramToDouble("WashStartTemp");    //Начальная температура зоны промывки, oC
    double WashStartCHNO3 = paramToDouble("WashStartCHNO3");    //Начальная концентрация HNO3 в зоне промывки, г/л
    double CrystZoneHeight = paramToDouble("CrystZoneHeight");    //Высота кристаллизационной зоны, м
    double CrystWallHeight = paramToDouble("CrystWallHeight");    //Высота верхней рубашки охлаждения зоны кристаллизации, м
    double WashZoneHeight = paramToDouble("WashZoneHeight");   //Высота зоны промывки, м
    double WashWallHeight = paramToDouble("WashWallHeight") ;   //Высота верхней рубашки охлаждения зоны промывки, м
    double Diam = paramToDouble("Diam");    //Внутренний диаметр кристаллизатора, м
    double LiquidStartLevel = paramToDouble("LiquidStartLevel");    //Начальный уровень жидкой фазы в кристаллизаторе, м
    double OutletHoleDiam = paramToDouble("OutletHoleDiam");    //Диаметр отверстия для отвода маточного и промывного раствора, м
    double start_akl = paramToDouble("start_akl");    //Коэффициент расхода клапана для отвода промывного и маточного раствора
    double AHeatToPipe = paramToDouble("AHeatToPipe");    //Коэффициент теплопередачи от рубашки к трубе
    double APipeToLiquid = paramToDouble("APipeToLiquid");    //Коэффициент теплопередачи от трубы к раствору
    double CrystCellNum = paramToDouble("CrystCellNum");    //Количество ячеек в зоне кристаллизации
    double WashCellNum = paramToDouble("WashCellNum");    //Количество ячеек в зоне промывки
    double KCryst = paramToDouble("KCryst");    //Cкорость роста кристаллической фазы, м/с
    double CrystCenters = paramToDouble("CrystCenters");    //Количество центров кристаллизации
    double TempMelt = paramToDouble("TempMelt");    //Температура плавления кристаллов
    double Kcore = paramToDouble("Kcore");    //Коэффициент правки кривых растворимости

    Hliq = LiquidStartLevel;              //1.5+1.7   //начальное значение уровня жидкой фазы в кристаллизаторе, м
    double debug;
    debug = Hliq;

    num_dydt = 5*(CrystCellNum + WashCellNum) + 1; //рассчитываем число диф.ур (+1 для уровня)
    //m_Y_out.resize(num_dydt);
    int i = 0;
    for(int j=1; j <= CrystCellNum; j++)
    {
        m_Y_out[i] = 0;             // (1) dR
        m_Y_out[i+1] = CrystStartTemp;     // (2) Ttr
        m_Y_out[i+2] = CrystStartTemp;     // (3) Tout
        m_Y_out[i+3] = 0;           // (4) dCun
        m_Y_out[i+4] = CrystStartCHNO3; // (5) dChnon3
        i = i + 5;
    }
    for(int j=1; j <= WashCellNum; j++)
    {
        m_Y_out[i] = 0;                // (1) dR
        m_Y_out[i+1] = WashStartTemp;     // (2) Ttr
        m_Y_out[i+2] = WashStartTemp;     // (3) Tout
        m_Y_out[i+3] = 0;              // (4) dCun
        m_Y_out[i+4] = WashStartCHNO3; // (5) dChno3
        i = i + 5;
    }
    m_Y_out[i] = Hliq; //Уровень
    i = i + 1;

    Input_Fin = 0;

    Output_Fout = 0;
    Output_Tout = 0;
    Output_Cu = 0;
    Output_Cpu = 0;
    Output_Chno3 = 0;
    Output_Cnp = 0;

    Output_GNu = 0;
    Output_GNu_tmp = 0;
    realGNU = 0;
    Output_mat_GNu = 0;

    balance_error_mat = 0;
    balance_error_common = 0;
    balance_error_hno3_mat = 0;
    balance_error_hno3_common = 0;

    kCu_buf = 0;
    kCpu_buf = 0;
    kCnp_buf = 0;

    kprCu_buf = 0;
    kprCpu_buf = 0;
    kprCnp_buf = 0;

    // Контейнер
    double Rvol = 0.122;  //Характеристики накопительной емкости
    double Hvol = 0.00177;
    double densGNU = 2810;

    double Vvol = 3.14 * Rvol * Rvol * Hvol; // объем емкости, м3 (1л = 0.001м3);

    ContainerLeft = new Container();
    ContainerRight = new Container();

    Containers.append(ContainerLeft);
    Containers.append(ContainerRight);

    for (int i = 0; i < 2; i++)
    {
        Containers[i]->Volume = Vvol;
        Containers[i]->Temperature = WashStartTemp;
        Containers[i]->CGNU = 0;
        Containers[i]->CHNO3 = WashStartCHNO3;
        Containers[i]->VGNU = 0;
        Containers[i]->VHNO3 = Vvol;
    }

    ThreeWayCrane = 0; // левая накопительная емкость
    ThreeWayCraneError = false;

    return true;
}

bool MP_CR_Crystallizer::process(double t, double h, std::string &error)
{
    if (isSituationActive("ES_CRANE_ERR") || isSituationActive("ES_HIGHLEVELSENSORS_ERR"))
    {
        error = "Переполнение накопительной емкости!";
        return false;
    }

    if (isSituationActive("ES_CONTAINERDRAIN_ERR") || isSituationActive("ES_CONTAINERTEMPER_ERR"))
    {
        error = "Попытка накопления кристаллов в неготовую емкость!";
        return false;
    }
    if (isSituationActive("ES_POWEROFF_ERR"))
    {
        error = "Отсутствует подача электроэнергии!";
        return false;
    }

    // Put your calculations here
    double CrystStartTemp = paramToDouble("CrystStartTemp");    //Начальная температура зоны кристаллизации, oC
    double CrystStartCHNO3 = paramToDouble("CrystStartCHNO3");    //Начальная концентрация HNO3 в зоне кристаллизации, г/л
    double WashStartTemp = paramToDouble("WashStartTemp");    //Начальная температура зоны промывки, oC
    double WashStartCHNO3 = paramToDouble("WashStartCHNO3");    //Начальная концентрация HNO3 в зоне промывки, г/л
    double CrystZoneHeight = paramToDouble("CrystZoneHeight");    //Высота кристаллизационной зоны, м
    double CrystWallHeight = paramToDouble("CrystWallHeight");    //Высота верхней рубашки охлаждения зоны кристаллизации, м
    double WashZoneHeight = paramToDouble("WashZoneHeight");   //Высота зоны промывки, м
    double WashWallHeight = paramToDouble("WashWallHeight") ;   //Высота верхней рубашки охлаждения зоны промывки, м
    double Diam = paramToDouble("Diam");    //Внутренний диаметр кристаллизатора, м
    double LiquidStartLevel = paramToDouble("LiquidStartLevel");    //Начальный уровень жидкой фазы в кристаллизаторе, м
    double OutletHoleDiam = paramToDouble("OutletHoleDiam");    //Диаметр отверстия для отвода маточного и промывного раствора, м
    double start_akl = paramToDouble("start_akl");    //Коэффициент расхода клапана для отвода промывного и маточного раствора
    double AHeatToPipe = paramToDouble("AHeatToPipe");    //Коэффициент теплопередачи от рубашки к трубе
    double APipeToLiquid = paramToDouble("APipeToLiquid");    //Коэффициент теплопередачи от трубы к раствору
    double CrystCellNum = paramToDouble("CrystCellNum");    //Количество ячеек в зоне кристаллизации
    double WashCellNum = paramToDouble("WashCellNum");    //Количество ячеек в зоне промывки
    double KCryst = paramToDouble("KCryst");    //Cкорость роста кристаллической фазы, м/с
    double CrystCenters = paramToDouble("CrystCenters");    //Количество центров кристаллизации
    double TempMelt = paramToDouble("TempMelt");    //Температура плавления кристаллов
    double Kcore = paramToDouble("Kcore");    //Коэффициент правки кривых растворимости

    step = h;
    std::vector<double> valvePort = PortInHeat->getInput();

    if(valvePort.size() == 5)
    {
        ukl = valvePort[0]; // положение клапана нслив маточного и промывного раствора (0 -1)
        Twall_in[0] = valvePort[1];
        Twall_in[1] = valvePort[2];
        Twall_in[2] = valvePort[3];
        Twall_in[3] = valvePort[4];
    }
    else
    {
        ukl = 0;
        Twall_in[0] = 0;
        Twall_in[1] = 0;
        Twall_in[2] = 0;
        Twall_in[3] = 0;
    }
    std::vector<double> matPort = PortInLiq->getInput();
    // Поток маточного раствора
    //Input_Fin = 0;
    Input_Tin = 0;Input_Cu = 0;Input_Cpu = 0;Input_Chno3 = 0;Input_Cnp = 0;

    double T = 2;
    Input_Fin = matPort[0];
    //Input_Fin += ( (matPort[0] - Input_Fin)/T ) * h;
    Input_Tin = matPort[1];
    Input_Cu = matPort[2];
    Input_Cpu = matPort[3];
    Input_Cnp = matPort[4];
    Input_Chno3 = matPort[5];

    std::vector<double> promPort = PortInWash->getInput();
    // Поток промывного раствора
    Input_pr_Fin = 0;Input_pr_Tin = 0;Input_pr_Cu = 0;Input_pr_Cpu = 0;Input_pr_Chno3 = 0;Input_pr_Cnp = 0;

    Input_pr_Fin = promPort[0];
    Input_pr_Tin = promPort[1];
    Input_pr_Cu = promPort[2];
    Input_pr_Cpu = promPort[3];
    Input_pr_Cnp = promPort[4];
    Input_pr_Chno3 = promPort[5];

    // Выходные значения
    // промывной раствор
    std::vector<double> mprPortNewOut;
    mprPortNewOut.resize(6);
    mprPortNewOut[0] = Output_Fout;  //M[0,2]
    mprPortNewOut[1] = Output_Tout;  //M[0,3]
    mprPortNewOut[2] = Output_Cu;    //M[0,4]
    mprPortNewOut[3] = Output_Cpu;   //M[0,5]
    mprPortNewOut[4] = Output_Cnp;   //M[0,7]
    mprPortNewOut[5] = Output_Chno3; //M(0,6)
    PortOutMPR->setNewOut(mprPortNewOut);

    // Кристаллы
    std::vector<double> crystallsPortNewOut;
    crystallsPortNewOut.resize(3);
    crystallsPortNewOut[0] = Output_GNu;
    crystallsPortNewOut[1] = Containers[0]->VGNU/Containers[0]->Volume;
    crystallsPortNewOut[2] = Containers[1]->VGNU/Containers[1]->Volume;;
    PortOutCryst->setNewOut(crystallsPortNewOut);

    //
    std::vector<double> levelPortNewOut;
    levelPortNewOut.resize(5);
    levelPortNewOut[0] = Hliq;
    levelPortNewOut[1] = mTout[1];
    levelPortNewOut[2] = mTout[(int) (mTout.size()*1/4)];
    levelPortNewOut[3] = mTout[(int) (mTout.size()*2/4)];
    levelPortNewOut[4] = mprTout[2];
    PortOutSens->setNewOut(levelPortNewOut);


    // Запуск решателя
    std::vector<double> y0,res;
    y0.resize(num_dydt);
    res.resize(num_dydt);
    int i;
    for(i = 0; i < num_dydt; i++)
    {
        y0[i] = m_Y_out[i];
    }

    res = solver(); //запус решателя

    for(i = 0; i < num_dydt; i++)
    {
        m_Y_out[i] += res[i]*h;
    }



    double debug;
    debug = Hliq;
    debug++;

    return true;
}

std::vector<double> MP_CR_Crystallizer::solver()
{
    double CrystStartTemp = paramToDouble("CrystStartTemp");    //Начальная температура зоны кристаллизации, oC
    double CrystStartCHNO3 = paramToDouble("CrystStartCHNO3");    //Начальная концентрация HNO3 в зоне кристаллизации, г/л
    double WashStartTemp = paramToDouble("WashStartTemp");    //Начальная температура зоны промывки, oC
    double WashStartCHNO3 = paramToDouble("WashStartCHNO3");    //Начальная концентрация HNO3 в зоне промывки, г/л
    double CrystZoneHeight = paramToDouble("CrystZoneHeight");    //Высота кристаллизационной зоны, м
    double CrystWallHeight = paramToDouble("CrystWallHeight");    //Высота верхней рубашки охлаждения зоны кристаллизации, м
    double WashZoneHeight = paramToDouble("WashZoneHeight");   //Высота зоны промывки, м
    double WashWallHeight = paramToDouble("WashWallHeight") ;   //Высота верхней рубашки охлаждения зоны промывки, м
    double Diam = paramToDouble("Diam");    //Внутренний диаметр кристаллизатора, м
    double LiquidStartLevel = paramToDouble("LiquidStartLevel");    //Начальный уровень жидкой фазы в кристаллизаторе, м
    double OutletHoleDiam = paramToDouble("OutletHoleDiam");    //Диаметр отверстия для отвода маточного и промывного раствора, м
    double start_akl = paramToDouble("start_akl");    //Коэффициент расхода клапана для отвода промывного и маточного раствора
    double AHeatToPipe = paramToDouble("AHeatToPipe");    //Коэффициент теплопередачи от рубашки к трубе
    double APipeToLiquid = paramToDouble("APipeToLiquid");    //Коэффициент теплопередачи от трубы к раствору
    double CrystCellNum = paramToDouble("CrystCellNum");    //Количество ячеек в зоне кристаллизации
    double WashCellNum = paramToDouble("WashCellNum");    //Количество ячеек в зоне промывки
    double KCryst = paramToDouble("KCryst");    //Cкорость роста кристаллической фазы, м/с
    double CrystCenters = paramToDouble("CrystCenters");    //Количество центров кристаллизации
    double TempMelt = paramToDouble("TempMelt");    //Температура плавления кристаллов
    double Kcore = paramToDouble("Kcore");    //Коэффициент правки кривых растворимости

    std::vector<double> y_out, dydt_out(num_dydt);
    double Err = 1E-6;

    double g=9.8;                           //м/с2;
    double oil=0.025E-4;                    //кинематическая вязкость жид фазы, м2/с
    double alpha = 0.274;                   //массовая доля воды, способной участвовать в образовании НУ из ГНУ (6*M_H20/M_НУ)

    double densGNU = 2810;                  //плотность твердой фазы, г/л (кг/м3)
    double densStal = 7900;                 //плотность материала трубы кристаллизатора, кг/м3
    double ltr = 0.003;                     //толщина трубы, м
    double pi=3.1415926535897932384626433832795;
    double Cliq=4142;                       //удельная теплоемкость жидкой фазы Дж/кгС ??????
    double Csol=440;                        //удельная теплоемкость твердой фазы Дж/кгС  ?????
    double Ctr = 462;                       // удельная теплоемкость стали, Дж/кгС

    double Tsat[13] = { 60, 50, 45, 40, 35, 25, 20, 15, 10, 5, 0, -5, -10 };

    //график растворимости
    double cNUsat[13][7] = { { 0.785, 0.785, 0.785, 0.785, 0.785, 0.785, 0.785 },
                             { 0.785, 0.660, 0.655, 0.650, 0.650, 0.650, 0.650 },
                             { 0.785, 0.600, 0.590, 0.588, 0.590, 0.592, 0.594 },
                             { 0.785, 0.520, 0.515, 0.510, 0.515, 0.520, 0.530 },
                             { 0.785, 0.450, 0.440, 0.440, 0.448, 0.460, 0.470 },
                             { 0.785, 0.356, 0.340, 0.332, 0.332, 0.340, 0.350 },
                             { 0.785, 0.320, 0.310, 0.300, 0.310, 0.310, 0.320 },
                             { 0.785, 0.230, 0.195, 0.180, 0.185, 0.200, 0.220 },
                             { 0.785, 0.215, 0.165, 0.150, 0.145, 0.155, 0.165 },
                             { 0.785, 0.200, 0.155, 0.135, 0.125, 0.120, 0.140 },
                             { 0.785, 0.185, 0.140, 0.120, 0.110, 0.110, 0.120 },
                             { 0.785, 0.130, 0.100, 0.080, 0.071, 0.070, 0.075 },
                             { 0.785, 0.100, 0.072, 0.060, 0.056, 0.056, 0.058 }
                           };

    // !!!!!!!!!!!!!!!!!!!! Локальные переменные !!!!!!!!!!!!!!!!!!!!
    int i,j,k;
    int ii;
    double sCu; double kCu; double kCpu; double kCnp;
    double Uliq;
    double dR;
    double Tout;
    double dCun;
    double dChno3;
    double dmtv_dt;

    double F;
    double Tin;
    double Cu;
    double Chno3;
    int zet;
    double Rin;

    double Win;

    int ket;
    double R;
    double pLiq;
    double buk;
    double Uc;

    double W;

    double Msum;
    double msum;
    double Csr;
    double Ttr;

    double tkr;
    double Csat;

    double ddR_dt;
    double dTtr_dt;
    double dTout_dt;
    double ddCun_dt;
    double ddChno3_dt;

    // !!!!!!!!!!!!!!!!!! Расчет кристаллизационной зоны ////////////////////////////////
    // !!!!!!!!!!!!!!!!!! Расчет дополнительных параметров
    double L = CrystZoneHeight / CrystCellNum; // длина ячейки, м
    double S=pi*Diam*Diam/4; // площадь сечения кристаллизатора m3
    double V = L * S; // объем ячейки
    double Str = pi*Diam*L; // площадь поверхности трубы кристаллизатора в ячейке (берем одинаковой для внутренней и внешней)
    double mtr =  pi*ltr*ltr*L*densStal; //масса трубы кристаллизатора в ячейке, кг
    double A=1.0/(1.0 + alpha);
    // !!!!!!!!!!!!!!!!!!! Расчет производных !!!!!!!!!!!!!!!!!!!!!!
    // Смешиваем концентрации
    sCu = Input_Cu + Input_Cpu + Input_Cnp;
    if  ((sCu > Err)&&(Input_Fin > 0))
    {
        kCu = Input_Cu/sCu; kCpu = Input_Cpu/sCu; kCnp = Input_Cnp/sCu;
        kCu_buf = kCu; kCpu_buf = kCpu; kCnp_buf = kCnp;
    } else
    {
        kCu = kCu_buf; kCpu = kCpu_buf; kCnp = kCnp_buf; sCu = 0;
    }

    // !!!!!!!!!!!!!!!!!!!!!!
    // Расчет касательной
    double K1 = 1.66;
    double K2 = 1;
    double Input_pLiq = 0;
    //mF[0] = Input_Fin/(1000*3600); // на входе л/ч
    // !!!!!!!!! Расчет расхода жидкой фазы в зоне кристаллизации !!!!!!!!!
    Hliq = m_Y_out[num_dydt-1];
    double Skl = pi*OutletHoleDiam*OutletHoleDiam / 4;
    Calculate_mat_Fout = ukl*start_akl*Skl*sqrt(2*g*(Hliq - WashZoneHeight));
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //mF[0] = Input_Fin; // расход жидкой фазы через зону крситаллизации (идеальное вытеснение), м3/с
    mF[0] = Calculate_mat_Fout; // расход жидкой фазы через зону крситаллизации (идеальное вытеснение), м3/с
    mTin[0] = Input_Tin;

    Input_pLiq=(1.023+(2.936E-2)*(Input_Chno3/63)+(1.313E-3)*sCu-((4.681E-4)+(3.475E-5)*Input_Chno3/63)*Input_Tin)*1E3;
    mCu[0] = sCu;
    mChno3[0] = Input_Chno3;

    double psi=(A-(K1*sCu/Input_pLiq))/(K2*Input_Chno3/Input_pLiq);
    if (((A / psi) > 0.34) && ((A / psi) < 0.36))
        mzet[0] = 0;
    else if (((A / psi) > 0.39) && ((A / psi) < 0.41))
        mzet[0] = 1;
    else if (((A / psi) > 0.42) && ((A / psi) < 0.44))
        mzet[0] = 2;
    else if (((A / psi) > 0.45) && ((A / psi) < 0.48))
        mzet[0] = 3;
    else if (((A / psi) > 0.49) && ((A / psi) < 0.51))
        mzet[0] = 4;
    else if (((A / psi) > 0.515) && ((A / psi) < 0.54))
        mzet[0] = 5;
    else  mzet[0] = 6;
    // !!!
    mR[0] = 0;
    mW[0] = 0;

    // !!!!!!!!!!!!!!!!!!
    // !!!!!!Коммутируем температуру рубашек кристаллизационной зоны
    k = (int)(CrystWallHeight/L); // ячейка начала второй рубашки охлаждения
    for(j=1;j<=k;j++)
        Twall[j] = Twall_in[0];
    for(j=k+1;j<=CrystCellNum;j++)
        Twall[j] = Twall_in[1];
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!!!!Рассчет ячеек кристаллизационной зоны !!!!!!!!!!!!!!!!!!!
    i = 0;
    Output_GNu = 0;
    for (j=1;j<=CrystCellNum;j++)
    {
        int filledCells = (Hliq - WashZoneHeight) / L;              // Кол-во заполненых ячеек
        int emptyCells = CrystCellNum - filledCells; // Кол-во пустых ячеек

        if(j <= emptyCells) // Если ячейка пустая
        {
            zet = mzet[j-1];

            dR = m_Y_out[i]; //if (dR < 0) dR = 0;
            Ttr = m_Y_out[i+1];
            Tout = m_Y_out[i+2];
            dCun = m_Y_out[i+3]; if (dCun < 0) dCun = 0;
            dChno3 = m_Y_out[i+4]; if (dChno3 < 0) dChno3 = 0;
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            ket = 0;

            dydt_out[i] = 0;
            //теплобаланс
            dydt_out[i+1] = 0;
            dydt_out[i+2] = 0;
            //сохранения вещества нитрат уранила
            dydt_out[i+3] = 0;
            //сохранения вещества азотки
            dydt_out[i+4] = 0;

            //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            //формируем переменные вход-выход
            mF[j] = 0;
            mTin[j] = Tout;
            mCu[j] = 0;
            mChno3[j] = 0;
            mzet[j] = zet;


            //формируем выходные переменные
            mTtr[j] = Ttr;
            mTout[j] = Tout;
            mdCun[j] = 0;
            mdChno3[j] = 0;
            mUc[j] = 0;
            mW[j] = 0;
            mR[j] = 0;

            m_Y_out[i] = 0;
            m_Y_out[i+3] = 0;
            m_Y_out[i+4] = 0;
        }
        else // Если ячейка заполнена
        {
            if(j == emptyCells + 1)
            {
                Uliq = Calculate_mat_Fout/S;     //скорость жидкой фазы m/c
                F = Calculate_mat_Fout;          // м3/с
                Tin = mTin[j-1];      // oС
                Cu = sCu;        // концентрация металла  на входе ячейки, г/л
                Chno3 = Input_Chno3;  // концентрация кислоты на входе ячейки, г/л
                zet = mzet[j-1];
                Rin = mR[j-1];      // %м
            }
            else
            {
                Uliq = mF[j-1]/S;     //скорость жидкой фазы m/c
                F = mF[j-1];          // м3/с
                Tin = mTin[j-1];      // oС
                Cu = mCu[j-1];        // концентрация металла  на входе ячейки, г/л
                Chno3 = mChno3[j-1];  // концентрация кислоты на входе ячейки, г/л
                zet = mzet[j-1];
                Rin = mR[j-1];      // %м
            }


            //копируем результаты решения диф.ур.
            dR = m_Y_out[i]; //if (dR < 0) dR = 0;
            Ttr = m_Y_out[i+1];
            Tout = m_Y_out[i+2];
            dCun = m_Y_out[i+3]; if (dCun < 0) dCun = 0;
            dChno3 = m_Y_out[i+4]; if (dChno3 < 0) dChno3 = 0;
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            ket = 0;

            // для скорости роста радиуса
            pLiq=(1.023+(2.936E-2)*(dChno3/63)+(1.313E-3)*dCun-((4.681E-4)+(3.475E-5)*dChno3/63)*Tout)*1E3;

            R = Rin + dR;
            if (R < 0) R = 0;

            //buk=(2/9)*(g/oil)*R*R*(densGNU/pLiq-1);
            buk = 0.2222*(g/oil)*R*R*((densGNU/pLiq)-1);
            Uc=Uliq+buk;

            for (ii=0;ii<13;ii++)
            {
                if (Tout <= Tsat[ii])
                    ket = ket + 1;
                else break;
            }


            if ((R >= 0)&&(ket > 0)&&(Uc > 0))
            {
                Csat = Kcore*cNUsat[ket][zet];
                tkr = L / Uc; // время прохождения кристалла ячейки, с
                ddR_dt = step*(KCryst*((K1*dCun/pLiq) - Csat)*tkr - dR)/(0.707*tkr);
                if ((dR<=0)&&(ddR_dt<0)&&(Tout <= TempMelt))
                    ddR_dt = 0;
                if ((R <= Err)&&(ddR_dt < 0))
                    ddR_dt = 0;
            } else
            {
                ddR_dt = 0;
                R = 0;
                tkr = 0;
                m_Y_out[i] = 0;
            }


            Win=1.333*pi*(Rin*Rin*Rin)*(CrystCenters/V); // доля твердой фазы на входе в ячейку
            W=1.333*pi*(R*R*R)*(CrystCenters/V); //доля твердой фазы на выходе ячейки

            // для теплового баланса
            Msum = F*pLiq + W*Uc*S*densGNU; // суммарный массовый расход жидкой и твердой фазы в ячейке
            Csr = Cliq *(1-W) + Csol*W; // средняя теплоемкость жидкой и твердой фазы в ячейке
            msum = pLiq*V*(1-W) + densGNU*V*W; //масса жидкой и твердой фазы в ячейке

            dTtr_dt = (AHeatToPipe*Str*(Twall[j] - Ttr)-APipeToLiquid*Str*(Ttr - Tout)) / (mtr*Ctr);
            dTout_dt = (APipeToLiquid*Str*(Ttr - Tout) - Msum*Csr*(Tout - Tin)) / (msum*Csr);

            // для материального баланса
            if (tkr > 0)
            {
                dmtv_dt = 1.333*pi*CrystCenters*densGNU*(R*R*R - Rin*Rin*Rin) / tkr;
                //dmtv_dt_deb1 = dmtv_dt;
                Output_GNu = Output_GNu + dmtv_dt;
                ddCun_dt = (F*Cu - F*dCun - dmtv_dt) / ((1-W)*V); //сохранения вещества нитрат уранила
            } else
            {
                ddCun_dt = (F*Cu - F*dCun ) / ((1-W)*V);
            }

            ddChno3_dt = (F*(Chno3 - dChno3)) /((1-W)*V); //сохранения вещества азотки

            if ((dCun <= Err)&&(ddCun_dt < 0))
            {
                dCun = Err; ddCun_dt = 0;
                m_Y_out[i+3] = Err;
            }
            if ((dChno3 <= Err)&&(ddChno3_dt < 0))
            {
                dChno3 = Err; ddChno3_dt = 0;
                m_Y_out[i+4] = Err;
            }

            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!
            // формируем вектор производных
            // радиус
            dydt_out[i] = ddR_dt;
            // теплобаланс
            dydt_out[i+1] = dTtr_dt;
            dydt_out[i+2] = dTout_dt;
            // сохранения вещества нитрат уранила
            dydt_out[i+3] = ddCun_dt;
            // сохранения вещества азотки
            dydt_out[i+4] = ddChno3_dt;



            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            // формируем переменные вход-выход
            mF[j] = F;
            mTin[j] = Tout;
            mCu[j] = dCun;
            mChno3[j] = dChno3;
            mzet[j] = zet;


            //формируем выходные переменные
            mTtr[j] = Ttr;
            mTout[j] = Tout;
            mdCun[j] = dCun;
            mdChno3[j] = dChno3;
            mUc[j] = Uc;
            mW[j] = W;
            mR[j] = R;
        }
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        i = i + 5;
    }

    //!!!!!!!!!!!!!!!!!! Расчет промывной зоны ////////////////////////////////
    //!!!!!!!!!!!!!!!!!! Расчет дополнительных параметров
    L = WashZoneHeight / WashCellNum; // длина ячейки, м
    S=pi*Diam*Diam/4; // площадь сечения кристаллизатора m3
    V = L * S; // объем ячейки
    Str = pi*Diam*L; // площадь поверхности трубы кристаллизатора в ячейке (берем одинаковой для внутренней и внешней)
    mtr =  pi*ltr*ltr*L*densStal; //масса трубы кристаллизатора в ячейке, кг
    A=1.0/(1.0 + alpha);
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //!!!!!!Дополнительные переменные !!!!!!!!!!!!!
    double sprCu; double kprCu; double kprCpu; double kprCnp;
    //!!!!!!Коммутируем температуру рубашек промывной зоны
    k = (int)(WashWallHeight/L); // ячейка начала второй рубашки охлаждения
    for(j=1;j<=k;j++)
        Twallpr[j] = Twall_in[2];
    for(j=k+1;j<=WashCellNum;j++)
        Twallpr[j] = Twall_in[3];
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //!!!!!!!!!!!!!!!!!!! Расчет производных !!!!!!!!!!!!!!!!!!!!!!
    //Смешиваем концентрации
    sprCu = Input_pr_Cu + Input_pr_Cpu + Input_pr_Cnp;
    if ((sprCu > Err)&&(Input_pr_Fin > 0))
    {
        kprCu = Input_pr_Cu/sprCu; kprCpu = Input_pr_Cpu/sprCu; kprCnp = Input_pr_Cnp/sprCu;
        kprCu_buf = kprCu; kprCpu_buf = kprCpu; kprCnp_buf = kprCnp;
    }
    else
    {
        kprCu = kprCu_buf; kprCpu = kprCpu_buf; kprCnp = kprCnp_buf; sprCu = 0;
    }
    //!!!!!!!!!!!!!!!!!!!!!!
    // Промывной раствор идет противотоком
    for (j=0;j<=WashCellNum;j++)
    {
        //mprF[j] = Input_pr_Fin/(1000*3600); //водная фаза (промывной поток) - сплошная, применяем модель идеального вытеснения
        if (ukl > 0)
        {
            mprF[j] = Input_pr_Fin; // м3/с
        } else
        {
            mprF[j] = 0;
        }
    }

    ii = CrystCellNum*5+2;
    for (j=1;j<=WashCellNum;j++)
    {
        mprTin[j-1] = m_Y_out[ii];
        ii = ii + 5;
    }
    mprTin[WashCellNum] = Input_pr_Tin;

    mprCu[WashCellNum] = sprCu;
    ii = CrystCellNum*5+3;
    for (j=1;j<=WashCellNum;j++)
    {
        mprCu[j-1] = m_Y_out[ii];
        ii = ii + 5;
    }

    mprChno3[WashCellNum] = Input_pr_Chno3;
    ii = CrystCellNum*5+4;
    for (j=1;j<=WashCellNum;j++)
    {
        mprChno3[j-1] = m_Y_out[ii];
        ii = ii + 5;
    }

    Input_pLiq=(1.023+(2.936E-2)*(Input_pr_Chno3/63)+(1.313E-3)*sprCu-((4.681E-4)+(3.475E-5)*Input_pr_Chno3/63)*Input_pr_Tin)*1E3;
    psi=(A-K1*(mprCu[WashCellNum])/Input_pLiq) / (K2*(mprChno3[WashCellNum])/Input_pLiq);
    if (((A / psi) > 0.34) && ((A / psi) < 0.36))
        mprzet[0] = 0;
    else if (((A / psi) > 0.39) && ((A / psi) < 0.41))
        mprzet[0] = 1;
    else if (((A / psi) > 0.42) && ((A / psi) < 0.44))
        mprzet[0] = 2;
    else if (((A / psi) > 0.45) && ((A / psi) < 0.48))
        mprzet[0] = 3;
    else if (((A / psi) > 0.49) && ((A / psi) < 0.51))
        mprzet[0] = 4;
    else if (((A / psi) > 0.515) && ((A / psi) < 0.54))
        mprzet[0] = 5;
    else  mprzet[0] = 6;
    //!!!
    mprR[0] = mR[CrystCellNum];
    mprW[0] = mW[CrystCellNum];
    // !!!!!!!!Расчет ячеек промывной зоны !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // i берем последнее после зоны кристаллизации
    Output_mat_GNu = Output_GNu;
    for (j=1;j<=WashCellNum;j++)
    {
        Uliq = mprF[j]/S;     //скорость жидкой фазы m/c
        F = mprF[j];          // м3/с
        Tin = mprTin[j];      // oС
        Cu = mprCu[j];        // концентрация урана на входе ячейки, г/л
        Chno3 = mprChno3[j];  // концентрация кислоты на входе ячейки, г/л
        zet = mprzet[j-1];
        Rin = mprR[j-1];      // %м



        //копируем результаты решения диф.ур.
        dR = m_Y_out[i]; //if (dR < 0) dR = 0;
        Ttr = m_Y_out[i+1];
        Tout = m_Y_out[i+2];
        dCun = m_Y_out[i+3]; if (dCun < 0) dCun = 0;
        dChno3 = m_Y_out[i+4]; if (dChno3 < 0) dChno3 = 0;
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        ket = 0;

        // для скорости роста радиуса
        pLiq=(1.023+(2.936E-2)*(dChno3/63)+(1.313E-3)*dCun-((4.681E-4)+(3.475E-5)*dChno3/63)*Tout)*1E3;

        R = Rin + dR;
        if (R < 0) R = 0;

        buk = 0.2222*(g/oil)*R*R*((densGNU/pLiq)-1);
        Uc = buk + mF[0]/S - Uliq; //buk - Uliq + mUc[num_cell]; ///Нужно корректировать выражение, так как противоток
        if (Uc < 0) Uc = 0;

        for (ii=0;ii<13;ii++)
        {
            if (Tout <= Tsat[ii])
                ket = ket + 1;
            else break;
        }


        if ((R >= 0)&&(ket > 0)&&(Uc > 0))
        {
            Csat = Kcore*cNUsat[ket][zet];
            tkr = L / Uc; // время прохождения кристалла ячейки, с
            ddR_dt = step*(KCryst*((K1*dCun/pLiq) - Csat)*tkr - dR)/(0.707*tkr);
            if ((dR<=0)&&(ddR_dt<0)&&(Tout <= TempMelt))
                ddR_dt = 0;
            if ((R <= Err)&&(ddR_dt < 0))
                ddR_dt = 0;
        } else
        {
            ddR_dt = 0;
            R = 0;
            tkr = 0;
            m_Y_out[i] = 0;
        }


        Win=1.333*pi*(Rin*Rin*Rin)*(CrystCenters/V); // доля твердой фазы на входе в ячейку
        W=1.333*pi*(R*R*R)*(CrystCenters/V); //доля твердой фазы на выходе ячейки

        // для теплового баланса
        Msum = F*pLiq + W*Uc*S*densGNU; // суммарный массовый расход жидкой и твердой фазы в ячейке
        Csr = Cliq *(1-W) + Csol*W; // средняя теплоемкость жидкой и твердой фазы в ячейке
        msum = pLiq*V*(1-W) + densGNU*V*W; //масса жидкой и твердой фазы в ячейке

        dTtr_dt = (AHeatToPipe*Str*(Twallpr[j] - Ttr)-APipeToLiquid*Str*(Ttr - Tout)) / (mtr*Ctr);
        dTout_dt = (APipeToLiquid*Str*(Ttr - Tout) - Msum*Csr*(Tout - Tin)) / (msum*Csr);


        // для материального баланса
        if (tkr > 0)
        {
            dmtv_dt = 1.333*pi*CrystCenters*densGNU*(R*R*R - Rin*Rin*Rin) / tkr;
            //dmtv_dt_deb2 = dmtv_dt;
            Output_GNu = Output_GNu + dmtv_dt;
            realGNU = 1.333*pi*CrystCenters*densGNU*(R*R*R);
            ddCun_dt = (F*Cu - F*dCun - dmtv_dt) / ((1-W)*V); //сохранения вещества нитрат уранила
        } else
        {
            ddCun_dt = (F*Cu - F*dCun ) / ((1-W)*V);
        }

        ddChno3_dt = (F*(Chno3 - dChno3)) /((1-W)*V); //сохранения вещества азотки

        if ((dCun <= Err)&&(ddCun_dt < 0))
        {
            dCun = Err; ddCun_dt = 0;
            m_Y_out[i+3] = Err;
        }
        if ((dChno3 <= Err)&&(ddChno3_dt < 0))
        {
            dChno3 = Err; ddChno3_dt = 0;
            m_Y_out[i+4] = Err;
        }

        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //формируем вектор производных
        //радиус
        dydt_out[i] = ddR_dt;
        //теплобаланс
        dydt_out[i+1] = dTtr_dt;
        dydt_out[i+2] = dTout_dt;
        //сохранения вещества нитрат уранила
        dydt_out[i+3] = ddCun_dt;
        //сохранения вещества азотки
        dydt_out[i+4] = ddChno3_dt;
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //формируем переменный вход-выход
        mprzet[j] = zet;


        //формируем выходные переменные
        mprTtr[j] = Ttr;
        mprTout[j] = Tout;
        mprdCun[j] = dCun;
        mprdChno3[j] = dChno3;
        mprUc[j] = Uc;
        mprW[j] = W;
        mprR[j] = R;
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        i = i + 5;
    }
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //расчет концентраций в выходном маточном потоке
    double Output_mat_Cu = kCu * mdCun[CrystCellNum];
    double Output_mat_Cpu = kCpu * mdCun[CrystCellNum];
    double Output_mat_Cnp = kCnp * mdCun[CrystCellNum];
    double Output_mat_Chno3 = mdChno3[CrystCellNum];
    double Output_mat_Fout = mF[CrystCellNum];
    double Output_mat_Tout = mTout[CrystCellNum];
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    //расчет концентраций в выходном промывном потоке потоке
    if ((mF[0] > 0)&&(mprF[0] > 0))
    {
        double Output_pr_Cu = ((kCu*mF[0]/(mF[0]+mprF[0]))+(kprCu*mprF[0]/(mF[0]+mprF[0]))) * mprCu[0];
        double Output_pr_Cpu = ((kCpu*mF[0]/(mF[0]+mprF[0]))+(kprCpu*mprF[0]/(mF[0]+mprF[0]))) * mprCu[0];
        double Output_pr_Cnp = ((kCnp*mF[0]/(mF[0]+mprF[0]))+(kprCnp*mprF[0]/(mF[0]+mprF[0]))) * mprCu[0];
        double Output_pr_Chno3 = mprChno3[0];
        double Output_pr_Fout = mprF[0];
        double Output_pr_Tout = mprTin[0];
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        Output_Cu = Output_mat_Cu*(mF[0]/(mF[0]+mprF[0])) + Output_pr_Cu*(mprF[0]/(mF[0]+mprF[0]));
        Output_Cpu = Output_mat_Cpu*(mF[0]/(mF[0]+mprF[0])) + Output_pr_Cpu*(mprF[0]/(mF[0]+mprF[0]));
        Output_Cnp = Output_mat_Cnp*(mF[0]/(mF[0]+mprF[0])) + Output_pr_Cnp*(mprF[0]/(mF[0]+mprF[0]));
        Output_Chno3 = Output_mat_Chno3*(mF[0]/(mF[0]+mprF[0])) + Output_pr_Chno3*(mprF[0]/(mF[0]+mprF[0]));
        Output_Fout = Output_mat_Fout + Output_pr_Fout;
        Output_Tout = Output_mat_Tout*(mF[0]/(mF[0]+mprF[0])) + Output_pr_Tout*(mprF[0]/(mF[0]+mprF[0]));
    } else
    {
        Output_Cu = 0;
        Output_Cpu = 0;
        Output_Cnp = 0;
        Output_Chno3 = 0;
        Output_Fout = 0;
        Output_Tout = 0;
    }

    /*
            Output_Cu = Output_mat_Cu;
            Output_Cpu = Output_mat_Cpu;
            Output_Cnp = Output_mat_Cnp;
            Output_Chno3 = Output_mat_Chno3;
            Output_Fout = Output_mat_Fout;
            Output_Tout = Output_mat_Tout;
            Output_GNu = Output_mat_GNu; */
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!!!!!! Модель уровня жидкой фазы в крсталлизаторе !!!!!!!!!!!!!!!!!
    dydt_out[i] = (Input_Fin + Input_pr_Fin - Output_Fout)/S;
    i = i + 1;
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //Проверка материального баланса
    double matpotok = (Input_Cu + Input_Cpu + Input_Cnp)*mF[0] - (Output_mat_Cu + Output_mat_Cpu + Output_mat_Cnp)*Output_mat_Fout;
    if (matpotok > 0)
        balance_error_mat = 100*(matpotok - Output_mat_GNu)/matpotok; // %

    double commonpotok = (Input_Cu + Input_Cpu + Input_Cnp)*mF[0] - (Output_Cu + Output_Cpu + Output_Cnp)*Output_Fout;
    if (commonpotok > 0)
        balance_error_common = 100*(commonpotok - Output_GNu)/commonpotok; // %


    balance_error_hno3_mat = (Input_Chno3)*mF[0] - (Output_mat_Chno3)*Output_mat_Fout; // кг/с
    balance_error_hno3_common = (Input_Chno3)*mF[0] - (Output_Chno3)*Output_Fout; // кг/с

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //заполняем вектор производных
    for(i=0; i < num_dydt;i++)
    {
        y_out.push_back(dydt_out[i]);
    }
    // Стопер !!!!!
    /*    if (t >= 3900)
            {
                i = 0;
            } */
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //пример использования ограничения
    // if( y0[1]>1.5 )
    //     y.SetContState(1,1.5);

    return y_out;
}

ICalcElement *Create()
{
    return new MP_CR_Crystallizer();
}

void Release(ICalcElement *block)
{
    if(block->type() == "MP_CR_Crystallizer") delete block;
}

std::string Type()
{
    return "MP_CR_Crystallizer";
}
