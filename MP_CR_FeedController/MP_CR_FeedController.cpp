#include "MP_CR_FeedController.h"

MP_CR_FeedController::MP_CR_FeedController()
{
    // Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
    createParameter("FIMPeriodUpSyr1", "10");
    createParameter("FIMPeriodUpSyr2", "10");
    createParameter("FIMPeriodDownSyr1", "205");
    createParameter("FIMPeriodDownSyr2", "205");
    createParameter("FlowPump1", "0.000001");
    createParameter("FlowPump2", "0.000001");

    // Сигналы блока

    // Порты блока
    PortIn = createInputPort(0, "", "INFO");
    PortOut = createOutputPort(1, "", "INFO");

    // Отказы блока

}

void MP_CR_FeedController::setDataNames()
{
    std::vector<std::string> dn;
    dn.push_back("Расход насоса 1, л/ч");
    dn.push_back("ЧИМ(шприц 1)");
    dn.push_back("Расход насоса 2, л/ч");
    dn.push_back("ЧИМ(шприц 2)");
    PortOut->setDataNames(dn);
}

bool MP_CR_FeedController::init(std::string &error, double h)
{
    // Put your initialization here
    setDataNames();

    setup = 0;

    return true;
}

bool MP_CR_FeedController::process(double t, double h, std::string &error)
{
    per_FIM1_p = paramToDouble("FIMPeriodUpSyr1");
    per_FIM2_p = paramToDouble("FIMPeriodUpSyr2");
    per_FIM1_o = paramToDouble("FIMPeriodDownSyr1");
    per_FIM2_o = paramToDouble("FIMPeriodDownSyr2");
    Q_pump1 = paramToDouble("FlowPump1");
    Q_pump2 = paramToDouble("FlowPump2");

    // Put your calculations here
    std::vector<double> in, out;
    out.resize(4);
    in = PortIn->getInput();
    double L1, L2, V1, V2, kris, h_cil, r_cil, h_por, r_por, r_ras, r_yr, dh_por, S_por, V_cil, V_por, pi, dV, L_kris;
    // Геометрия дозаторов (шприцов)  вынести в параметры или вход?
    h_cil=145;      // высота цилиндра, мм
    r_cil=123/2;     // радиус цилиндра, мм
    h_por=140;   // высота поршня, мм
    r_por=113/2; // радиус поршня, мм
    r_ras=12/2;      // радиус отв. для подачи р-ра, мм
    r_yr=10/2;       // радиус отв. для датчика уровня, мм
    dh_por=0.01;   // шаг поршня, мм
    pi=3.14159265358979323846;          // число Пи

    S_por=pi*r_por*r_por-pi*r_ras*r_ras-pi*r_yr*r_yr; //  площадь основания поршня в мм^2
    V_cil=pi*h_cil*r_cil*r_cil*0.000001; // объем цилиндра в л
    V_por=(pi*h_por*r_por*r_por-pi*h_por*r_ras*r_ras-pi*h_por*r_yr*r_yr)*0.000001; // объем поршня в л
    dV=V_cil-V_por; // л


    L_kris=3.2;  // контролируемый уровень в кристаллизаторе, л

    if (in.size()!=0)
    {
        V1=in[0];
        V2=in[1];
        L1=in[2];
        L2=in[3];
        kris=in[4];
    }
    else
    {
        error = "Блок управления дозаторами: Не подключен вход!";
        return false;
    }

    if( setup == 0)
    {
        out[0] = Q_pump1;
        out[1] = 0;
        out[2] = Q_pump2;
        out[3] = 0;

        if(V1 >= V_cil && V2 >= V_cil)
        {
            setup = 1;
        }
    }

    if( setup == 1 )
    {
        out[0] = 0;
        out[1] = per_FIM1_o;
        out[2] = 0;
        out[3] = 0;

        if(L1 == 100)
        {
            setup = 2;
        }
        if( kris > L_kris)
        {
            out[1] = 0;
            out[3] = 0;
        }
    }

    if( setup == 2)
    {
        if(L1 > 0)  //&& V1 <=  dV
        {
            out[1] = -per_FIM1_p;
            out[0] = 0;
        }

        if(L1 == 0 && V1 < V_cil)
        {
            out[1] = 0;
            out[0] = Q_pump1;
        }

        if( L1 == 0 && V1 >= V_cil)
        {
            out[0] = 0;
            out[1] = 0;
        }

        out[2] = 0;
        out[3] = per_FIM2_o;

        if ( kris > L_kris)
        {
            out[3] = 0;
        }

        if(L2 == 100)
        {
            setup = 3;
        }
    }

    if( setup == 3)
    {
        if(L2 > 0) //&& V2 <=  dV
        {
            out[3] = -per_FIM2_p;
            out[2] = 0;
        }
        if(L2 == 0 && V2 < V_cil)
        {
            out[3] = 0;
            out[2] = Q_pump2;
        }
        if( L2 == 0 && V2 >= V_cil)
        {
            out[2] = 0;
            out[3] = 0;
        }

        out[0] = 0;
        out[1] = per_FIM1_o;


        if ( kris > L_kris)
        {
            out[1] = 0;
        }

        if(L1 == 100)
        {
            setup = 2;
        }
    }
    PortOut->setOut(out);
    return true;
}


ICalcElement *Create()
{
    return new MP_CR_FeedController();
}

void Release(ICalcElement *block)
{
    if(block->type() == "MP_CR_FeedController") delete block;
}

std::string Type()
{
    return "MP_CR_FeedController";
}
