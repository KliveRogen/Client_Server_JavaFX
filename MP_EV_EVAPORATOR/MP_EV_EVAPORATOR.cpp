#include "MP_EV_EVAPORATOR.h"
#include "math.h"

MP_EV_EVAPORATOR::MP_EV_EVAPORATOR()
{
    // Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
    createParameter("m_Ts", "100");

    // Сигналы блока

    // Порты блока
    PortInLiq = createInputPort(0, "Раствор на конденсацию", "INFO");
    PortOutLiq = createOutputPort(1, "Сконденсированный раствор", "INFO");
    PortInPump = createInputPort(2, "Расход насоса на выдаче", "INFO");
    PortOutSensors = createOutputPort(3, "Датчики", "INFO");

    // Отказы блока

}

void setDataNames()
{
    std::vector<std::string> dn;
}

bool MP_EV_EVAPORATOR::init(std::string &error, double h)
{
    setDataNames();
    // Put your initialization here
    A=0.0038;
    Cf =4100;// %1.0875*1.9523e+004; %4100; %% [Dz/kg*C], udelnaia teploemkost rastvora na vhode
    Cp =3770;// %1.9523e+004; %3770; %% [Dz/kg*C], udelnaia teploemkost rastvora na vihode
    Cw = Cf;
    pr=977.71;//%(1/1.8)*1000;%% [kg/m^3] plotnost flegma (plotnost voda pri 100 0^C =958.38)
    lambda = 2300 ;//% [Dz/kg]   ydelnaya teplota paroobrazovania

    a[0] = 1023;
    a[1] = 29.36;
    a[2] = 1.313;
    a[3] = 0.4681;
    a[4] = 0.03475;

    m_Ps  = 350000;

    dy[0] = 0;
    dy[1] = 0;
    dy[2] = 0;
    dy[3] = 0;
    dy[4] = 0;

    double height = 0.2158*2;   //%начальный уровень
    double XX = 70;

    double Ws = 0;

    double c_U = 80;
    double c_HNO3 = 0.1;
    double pp = a[0] + a[1]*c_HNO3 + a[2]*c_U - (a[3] + a[4]*c_HNO3)*(m_T_in+273);


    double mp = pp*A*height;

    double Pv = 31.453*m_T_in*m_T_in - 2947.7*m_T_in + 82238;// % [Pa], preshure vapor for water
    double Tkip = -58.07119+(1.6035*1000)*(pow(Pv,0.19915))/(90.7189+pow(Pv,0.19915));//% [Grad]

    m_h = height;
    m_T = m_T_in;

    m_Concentrations_X_out[0] = m_Concentrations_X_in[0];
    m_Concentrations_X_out[1] = m_Concentrations_X_in[1];

    m_pp = pp;

    int i = 0;
    PortOutLiq->setOut(i, m_Qx_out); i++;
    PortOutLiq->setOut(i, m_T); i++;
    PortOutLiq->setOut(i, m_Concentrations_X_out[0]); i++;
    PortOutLiq->setOut(i, m_Concentrations_X_out[1]); i++;

    PortOutSensors->setOut(0, m_h);
    PortOutSensors->setOut(1, m_pp);

    return true;
}

bool MP_EV_EVAPORATOR::process(double t, double h, std::string &error)
{
    // Put your calculations here
    std::vector<double> y0, y1;
    y0 = PortInLiq->getInput();
    y1 = PortInPump->getInput();

    if (y0.size() == 0 || y1.size() == 0)
    {
        error = "Выпарной аппарат не подключен";
        return false;
    }

    m_Qx_in = y0[0];  // Объемный расход
    m_T_in = y0[1];  // Температура

    m_Concentrations_X_in[0] = y0[2];
    m_input_Components_Mass[0] += m_Concentrations_X_in[0]*m_Qx_in*h;

    m_Concentrations_X_in[1] = y0[3];
    m_input_Components_Mass[1] += m_Concentrations_X_in[1]*m_Qx_in*h;

    m_Qx_out = y1[0];

    if(1)
    {
        std::vector<double> outVar0, res; // Вектор начальных значений рассчитываемых переменных
        outVar0.resize(5); // Размер задается числом рассчитываемых переменных
        res.resize(5);

        outVar0[0] = m_h;
        outVar0[1] = m_T;
        outVar0[2] = m_Concentrations_X_out[0];
        outVar0[3] = m_Concentrations_X_out[1];
        outVar0[4] = m_pp;

        //TODO: Вызвать решение дифуравнения.
        res = dydt(outVar0, h);

        //конец функции решателя

        m_h = res[0];
        m_T = res[1];
        m_Concentrations_X_out[0] = res[2];
        m_Concentrations_X_out[1] = res[3];
        m_pp = res[4];

        //Упаренный раствор

        PortOutLiq->setNewOut(0, m_Qx_out);
        PortOutLiq->setNewOut(1, m_T);

        // Сделать покомпонентный вывод
        PortOutLiq->setNewOut(2, m_Concentrations_X_out[0]);
        PortOutLiq->setNewOut(3, m_Concentrations_X_out[1]);

        PortOutSensors->setNewOut(0, m_h);
        PortOutSensors->setNewOut(1, m_pp);
    }

    return true;
}

std::vector<double> MP_EV_EVAPORATOR::dydt(std::vector<double> y0, double h)
{
    std::vector<double> out;
    out.resize(5);
    int i = 0;

    out[i] = dy[i]; i++;
    out[i] = dy[i]; i++;
    out[i] = dy[i]; i++;
    out[i] = dy[i]; i++;
    out[i] = dy[i]; i++;

    m_h = y0[0];
    m_T = y0[1];

    m_pp = y0[4];

    if(m_h < 0) m_h = 0;
    double height = m_h;

    double c_U =  m_Concentrations_X_out[0];
    double c_HNO3  =  m_Concentrations_X_out[1];

    double c_U_f =  m_Concentrations_X_in[0];
    double c_HNO3_f  =  m_Concentrations_X_in[1];

    double pp = m_pp;

    double pf = a[0] + a[1]*c_HNO3_f + a[2]*c_U_f - (a[3] + a[4]*c_HNO3_f)*(m_T_in+273);

    double   Pv = 31.453*m_T_in*m_T_in - 2947.7*m_T_in + 82238;// % [Pa], preshure vapor for water
    double Tkip = -58.07119+(1.6035*1000)*(pow(Pv,0.19915))/(90.7189+pow(Pv,0.19915));//% [Grad]



    double   Tsatv=2147/(10.76-log10(Pv))-273.2;//%[graduc]
    double   iv=2.5*1000000+1813*Tsatv+0.471*Tsatv*Tsatv-0.011*Tsatv*Tsatv*Tsatv+2090*(m_T-Tsatv);// %[Dz/kg], entalpia vtorichnogo para

    double m_Ps  = 350000;
    double  Tsat=2147/(10.76-log10(m_Ps))-273.2;//%%[graduc]
    double  is=2.5*1000000+1813*Tsat+0.471*Tsat*Tsat-0.011*Tsat*Tsat*Tsat+2090*(m_Ts-Tsat);//%%[Dz/kg], entalpia greuchego para
    double XX = Pv;
    pp = a[0] + a[1]*c_HNO3 + a[2]*c_U - (a[3] + a[4]*c_HNO3)*(m_T+273);

    double mp=pp*A*height;

    double FF = 1.6300e+03*(m_Ts-m_T);
    double Ws;
    if (m_T >= 95)
    {
        Ws = (1/(lambda + Cw*m_T))*((FF + m_Qx_in*pf*Cf*m_T_in)- (m_Qx_out*pp*Cp*m_T +height*A*m_T*dy[4]*Cp+ pp*m_T*A*dy[0]*Cp)) ;

        if( Ws < 0)
        { Ws = 0;}

    }
    else
    {
        Ws = 0;
    }

    out[1] = (1/(Cp*pp*A*height))*((FF + m_Qx_in*pf*Cf*m_T_in)- (m_Qx_out*pp*Cp*m_T +height*A*m_T*out[4]*Cp+ pp*m_T*A*out[0]*Cp) - Ws*(lambda + Cw*m_T));

    if (height>0.00001)
    {
        if(height>= 0.5)
        {
            //error = "Авария! Выпарной аппарат переполнен!";
            //result = false;
        }
        out[0] = (1/(pp*A))*( -A*height*out[4]+m_Qx_in*pf-m_Qx_out*pp -Ws);
    }
    else
    {
        out[0] = 0;

        //y.SetContState(0,0.00001);
        height=0.00001;
        //error = "Авария! Выпарной аппарат опустошен!";
        //result = false;
    }

    out[2] = (1/(A*height))*(m_Qx_in*(c_U_f -c_U*pf/pp) + c_U*(A*height*out[4]/(pp) + Ws/pp));
    out[3] = (1/(A*height))*(m_Qx_in*(c_HNO3_f -c_HNO3*pf/pp) + c_HNO3*(A*height*out[4]/(pp) + Ws/pp));

    out[4] = (a[1] - a[4]*(m_T+273))*out[3] + a[2]*out[2] - (a[3] + a[4]*c_HNO3)*out[1];

    for (int i = 0; i < out.size(); i++)
    {
        dy[i] = out[i];
        out[i] = y0[i] + out[i] * h;
    }

    return out;
}

ICalcElement *Create()
{
    return new MP_EV_EVAPORATOR();
}

void Release(ICalcElement *block)
{
    if(block->type() == "MP_EV_EVAPORATOR") delete block;
}

std::string Type()
{
    return "MP_EV_EVAPORATOR";
}
