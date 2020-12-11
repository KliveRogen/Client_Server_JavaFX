#include "MP_EX_EXTRACTOR.h"

#include <math.h>

MP_EX_EXTRACTOR::MP_EX_EXTRACTOR()
{
    // Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    createParameter("EX_VOLUME", "0.5");
    createParameter("OF_HERM_PERCENT", "20");
    createParameter("VF_HERM_PERCENT", "20");
    createParameter("OF_CHANNEL_PERCENT", "20");
    createParameter("VF_CHANNEL_PERCENT", "20");
    createParameter("ENG_FREQ", "20");

    createSituation("ES_ENGINE_CIRCUIT");
    createSituation("ES_ENGINE_SHORT");
    createSituation("ES_ENGINE_STOP");
    createSituation("ES_ENGINE_NOM");
    createSituation("ES_FEED_OF");
    createSituation("ES_FEED_VF");
    createSituation("ES_CHANNEL_OF");
    createSituation("ES_CHANNEL_VF");

    InEng = createInputPort(0, "Частота вращения", "INFO");
    InVF = createInputPort(1, "Вход ВФ", "INFO");
    InOF = createInputPort(2, "Вход ОФ", "INFO");

    OutVF = createOutputPort(3, "Выход ВФ", "INFO");
    OutOF = createOutputPort(4, "Выход ОФ", "INFO");
    OutSensors = createOutputPort(5, "Датчики", "INFO");
}

bool MP_EX_EXTRACTOR::init(std::string &error, double h)
{
    std::vector<std::string> dn;
    dn.push_back("Расход, мл/ч");
    dn.push_back("Температура, °С");
    dn.push_back("Концентрация U, г/л");
    dn.push_back("Концентрация Pu, г/л");
    dn.push_back("Концентрация HNO3, г/л");
    dn.push_back("Концентрация Np, г/л");

    OutVF->setDataNames(dn);
    OutOF->setDataNames(dn);

    m_volume = paramToDouble("EX_VOLUME");

    dn.clear();
    dn.resize(4);
    dn[0] = "m_Qx_out";
    dn[1] = "m_Qy_out";
    dn[2] = "m_Vx";
    dn[3] = "m_Vy";

    OutSensors->setDataNames(dn);

    m_Qx_in =0;
    m_Qx_out=0;
    m_Qy_in=0;
    m_Qy_out =0;

    m_f =0;

    m_dQx =0;
    m_dQy =0;
    m_dVx =0;
    m_dVy =0;

    //  m_Vx =0;
    //  m_Vy =0;

    m_temp_miss =0;

    m_py =0;
    m_sigma =0;
    m_dk =0;
    m_A =0;
    m_Tqx =0;
    m_Tqy =0;
    m_Tqvx =0;
    m_Tqvy =0;
    m_hs =0;

    m_Ko.resize(4);
    m_Ko[0] = 1e-6;   // Уран
    m_Ko[1] = 1.5e-4; // Плутоний
    m_Ko[2] = 1e-4;   // Азотная кислота
    m_Ko[3] = 1.5e-4; // Нептуний

    m_input_Components_Mass.resize(4);
    m_Concentrations_X_in.resize(4);
    m_Concentrations_Y_equilibrium.resize(4);
    m_Concentrations_Y_in.resize(4);
    m_Concentrations_X_out.resize(4);
    m_Concentrations_Y_out.resize(4);
    m_Concentrations_dX.resize(4);
    m_Concentrations_dY.resize(4);
    m_K.resize(4);
    m_G.resize(4);

    m_Concentrations_rk_dX.resize(4);
    m_Concentrations_rk_dY.resize(4);
    m_Concentrations_rk_X_out.resize(4);
    m_Concentrations_rk_Y_out.resize(4);


    m_input_Components_Mass.resize(4);
    m_current_Components_Mass.resize(4);
    m_current_Components_Concentration.resize(4);

    minQ = 0.5E-8;
    mint = 0.01;

    //Центробежный эксьрактор
    m_dm = 0.0092; // m (diametr meshalki)
    m_Vs = 0.000025; // m^3 (objem smesitelnoi kameri)
    m_Vr = 0.000027; // m^3 (objem razdelitelnoi kameri)

    m_Vx = 0.5*m_Vs;
    m_Vy = 0.5*m_Vs;
    m_currentvolume = 0;
    //m_Vx = 0;
    //m_Vy = 0;

    minV = 0.01*m_Vs;

    m_const_k  =0 ;
    // m_geometry_V0;

    m_geometry_S = 1;
    H = 0;
    m_geometry_H =  m_volume/m_geometry_S;

    Prev_H = H;
    m_T0 = 20;

    m_Tx =20;
    m_Ty =20;

    return true;
}

double MP_EX_EXTRACTOR::Y_equilibrium(int ComponentId, int mytyper)
{
    /* must be */
    double Cykr = 0;

    if((m_Concentrations_X_out[0] > 0)&&(m_Concentrations_X_out[1] > 0)&&(m_Concentrations_X_out[2] > 0))
    {
        double   Cxu=m_Concentrations_X_out[0]/238;
        double   CxP=m_Concentrations_X_out[1]/244;
        double   Cxa=m_Concentrations_X_out[2]/63;

        //----------------отладка, закоментировать-----
        //Cxu = 260 / 238;
        //CxP = 40 / 244;
        //Cxa = 283.5 / 63;
        //---------------------------------------------

        double   K1=Cxa+2*Cxu+4*CxP;
        double   K2 = Cxa+3*Cxu+10*CxP;
        double   K_H = 0.18*Cxa*K1;
        double   Step_U=4.8-6.18*sqrt(K2)+4.85*K2-1.025*pow(K2,1.5)+0.025*pow(K2,2);
        double   Step_Pu=9.44-13.02*sqrt(K2)+5.71*K2-0.76*pow(K2,1.5);
        double   add_U=exp(Step_U);
        double   add_Pu=exp(Step_Pu);
        double   N_U=add_U*Cxu*pow(K1,2);
        double   N_Pu=add_Pu*CxP*pow(K1,4);
        double   prom1=0.018*K_H*(1+K_H)+N_U+N_Pu;
        double   prom2=2.*1.09/(1+K_H+sqrt(pow((1+K_H),2)+8.*1.09*prom1));
        prom2=prom2/(0.041*N_U*(pow(prom2,2))+1);

        double pU =0;
        double C_U_wat =0;
        double C_Pu_wat =0;


        if (Cxa<=10)
            pU=exp(-0.12*(pow(Cxa,3))*(pow(prom2,2))*(1-0.005*(pow(Cxa,2))));
        else
            pU=exp(-0.12*pow(Cxa,3)*pow(prom2,2));

        N_U = N_U*pU;

        double    C_h_wat = K_H*prom2;
        double    Ch_so_extr=0.004*C_h_wat*Cxa;
        double    Ch_so_extr_U=0.0045*N_U*pow(prom2,2)*Cxa*K1;
        switch (ComponentId)
        {
        case 0: //Uran
            C_U_wat=N_U*pow(prom2,2);
            Cykr = C_U_wat*238;
            if(mytyper == 2)
                Cykr = m_Concentrations_Y_out[0]; //всякая непонятная промывка (металл не выпадает)
            break;
        case 1:   //Pu->39/40
            C_Pu_wat=N_Pu*pow(prom2,2);
            if(mytyper == 0)
                Cykr = C_Pu_wat*244;
            if(mytyper == 1)
                Cykr = 0.01*C_Pu_wat*244;//для восстановительной реэкстракции Pu
            if(mytyper == 2)
                Cykr = m_Concentrations_Y_out[1]; //всякая непонятная промывка (металл не выпадает)
            break;
        case 2:  //HNO3
            C_h_wat=C_h_wat+Ch_so_extr+Ch_so_extr_U;
            Cykr = C_h_wat*63;
            break;
        default:
            Cykr = 0;
            break;
        }

    }

    return Cykr;
}

std::vector<double> MP_EX_EXTRACTOR::dydt(double t, std::vector<double> &y0)
{
    double Kf;

    std::vector<double> y_out;
    y_out.clear();

    // зануляем отрицательные величины
    if(m_Qx_out <0) m_Qx_out =0;
    if(m_Qy_out <0) m_Qy_out =0;
    if(m_Vx < 0)    m_Vx =0;
    if(m_Vy < 0)    m_Vy =0;
    if(m_Ty< 0)     m_Ty = 20;
    if(m_Tx< 0)     m_Tx = 20;

    for(int i = 0; i < m_Concentrations_X_out.size(); i++) if(m_Concentrations_X_out[i] < 0) m_Concentrations_X_out[i] = 0;
    for(int i = 0; i < m_Concentrations_Y_out.size(); i++) if(m_Concentrations_Y_out[i] < 0) m_Concentrations_Y_out[i] = 0;
    if(m_Qx_in + m_Qy_in > 2*minQ)
        m_T = (m_Qx_in/(m_Qx_in + m_Qy_in))*m_Tx + (m_Qy_in/(m_Qx_in + m_Qy_in))*m_Ty;
    else
        m_T = 20;

    // Суммарные концентрации металла в фазах
    double Cmex = 0;
    Cmex += m_Concentrations_X_out[0];
    Cmex += m_Concentrations_X_out[1];

    double Cmey = 0;
    Cmey += m_Concentrations_Y_out[0];
    Cmey += m_Concentrations_Y_out[1];

    //-----------------------------------------------------------------------------------------------------------
    //-------------------------------------Непосредственный код модели экстрактора-------------------------------
    //В этой модели органическая фаза легкая (подготовка дял смесительной каморы)

    //считаем плотность сплошной фазы
    m_py = 800 + 1.26*Cmey - 0.94*(m_T - 20);
    //double hno3 = m_Concentrations_X_in[12];;
    //m_px = 998 + 0.81*Cmex + 0.52*hno3 - 0.938*(m_T - 20);  // не используется

    m_sigma = 2.73*0.01-8.8*0.00001*(m_T-20);  //insted of .00005

    m_dk = pow(m_sigma,0.6)*pow(m_py,-0.6)*pow(m_f,-1.2);

    // dk = ((sigma)^0.6)*((py)^-0.6)*((n)^-1.2);

    if (m_Vx <= m_Vy)
        m_A = (6*m_Vx)/m_dk; // экстракция
    else
        m_A = (6*m_Vy)/m_dk; // реэкстракция

    if(m_Qx_in + m_Qy_in > 2*minQ)
    {
        //m_Tq = 0.1*m_Vs/(m_Qx_in + m_Qy_in);
        m_Tqx = 0.1*m_Vx/m_Qx_in;
        m_Tqy = 0.1*m_Vy/m_Qy_in;
        m_Tqvx = 0.01*m_Tqx;
        m_Tqvy = 0.01*m_Tqy;
    }
    else
    {
        //m_Tq = 0;
        m_Tqx = 0;
        m_Tqy = 0;
        m_Tqvx = 0;
        m_Tqvy = 0;
    }

    ////////////////////////////////////////////////////////////////////////////

    if (m_Tqx > 0 && m_Tqy > 0 && m_Tqvx > 0 && m_Tqvy > 0)
    {
        if(m_Vx + m_Vy < 0.9*m_Vs)//емкость наполняется  !!!!изменение
        {
            m_dQx = 0;       // 1 - я производная - расход x
            m_dQy = 0;       // 2 - я производная - расход у
            m_Qx_out = 0;
            m_Qy_out = 0;
        }
        else
        {
            if(isSituationActive("ES_CHANNEL_OF")) {
                double coeff = paramToDouble("OF_CHANNEL_PERCENT");
                if(coeff < 0.00)   coeff = 0.00;
                if(coeff > 100.00) coeff = 100.00;

                m_dQx = (((100.00 - coeff)/ 100.00) * m_Qx_in - m_Qx_out)/m_Tqx;
            }
            else m_dQx = (m_Qx_in - m_Qx_out)/m_Tqx;

            if(isSituationActive("ES_CHANNEL_VF")) {
                double coeff = paramToDouble("VF_CHANNEL_PERCENT");
                if(coeff < 0.00)   coeff = 0.00;
                if(coeff > 100.00) coeff = 100.00;

                m_dQy = (((100.00 - coeff)/ 100.00) * m_Qy_in - m_Qy_out)/m_Tqy;
            }
            else m_dQy = (m_Qy_in - m_Qy_out)/m_Tqy;

        }
        y_out.push_back(m_dQx);
        y_out.push_back(m_dQy);

        m_Vx_stat = (m_Qx_in*m_Vs)/(m_Qx_in + m_Qy_in);
        m_dVx =(m_Vx_stat - m_Vx)/m_Tqvx;
        if (m_Vx <= 0 && m_dVx < 0)  m_dVx =0;    // 3 - я производная - объем x   */
        // m_dVx = m_Qx_in-m_Qx_out;
        y_out.push_back(m_dVx);


        m_Vy_stat = (m_Qy_in*m_Vs)/(m_Qx_in + m_Qy_in);
        m_dVy =(m_Vy_stat - m_Vy)/m_Tqvy;
        if (m_Vy <= 0 && m_dVy < 0)  m_dVy =0;    // 4 - я производная - объем y  */
        //  m_dVy = m_Qy_in-m_Qy_out;
        y_out.push_back(m_dVy);

        Kf =0;

        for(int i = 0; i < m_Concentrations_Y_out.size(); i++) {
            if(m_Vx > 0.1*m_Vx_stat && m_Vy> 0.1*m_Vy_stat && m_Vx > minV && m_Vy > minV) {
                Kf = m_Vx/m_Vy;

                //расчет равновесной концентравции компонента
                int typer = 0;
                if(m_volume < 1) typer = 0;//Обычная экстракция/реэкстракция
                if((m_volume >= 1)&&(m_volume <2)) typer = 1;//восстановительная реэкстракция Pu
                if((m_volume >= 2)&&(m_volume <3)) typer = 2;//всякая непонятная промывка (металл не выпадает)
                m_Concentrations_Y_equilibrium[i] = Y_equilibrium(i,typer);

                //расчет потока G
                m_G[i] = m_Ko[i]*m_f*m_dm*m_A*(m_Concentrations_Y_equilibrium[i] - m_Concentrations_Y_out[i]);

                //Расчет производных для фаз Х и У
                m_Concentrations_dX[i] = (m_Qx_in*m_Concentrations_X_in[i]-m_Qx_out*m_Concentrations_X_out[i]- m_G[i])/m_Vx;
                if(m_Concentrations_dX[i]<0 && m_Concentrations_X_out[i] <=0)
                {
                    m_Concentrations_dX[i]=0;
                    m_Concentrations_X_out[i] = 0;
                    m_G[i] = 0;
                }
                m_Concentrations_dY[i] = (m_Qy_in*m_Concentrations_Y_in[i]-m_Qy_out*m_Concentrations_Y_out[i]+ m_G[i])/m_Vy;
                if(m_Concentrations_dY[i]<0 && m_Concentrations_Y_out[i] <=0)
                {
                    m_Concentrations_dY[i]=0;
                    m_Concentrations_Y_out[i] = 0;
                    m_G[i] = 0;
                    m_Concentrations_dX[i] = (m_Qx_in*m_Concentrations_X_in[i]-m_Qx_out*m_Concentrations_X_out[i]- m_G[i])/m_Vx;
                }
            }
            else {
                m_Concentrations_dX[i]=0;

                m_Concentrations_X_out[i] = m_Concentrations_X_in[i];  //заполняется объем исходной концентрацией в фазе X
                m_Concentrations_dY[i]=0;
                m_Concentrations_Y_out[i] = m_Concentrations_Y_in[i]; //заполняется объем исходной концентрацией в фазе Y
            }

            y_out.push_back(m_Concentrations_dX[i]);
            y_out.push_back(m_Concentrations_dY[i]);
        }

        double  Vt; double  Vl; double tt; double tl;
        if (Kf > 0)
        {
            Vt = Kf*m_Vr/(1+Kf);

            if(Vt != Vt)
            {
                int a =66;
                a++;
            }

            Vl = m_Vr - Vt;
            tt = 0.01*Vt/m_Qx_out;
            tl = 0.01*Vl/m_Qy_out;
        }
        else
        {
            tt = mint;
            tl = mint;
        }

        for(int i = 0; i < m_Concentrations_Y_out.size(); i++) {
            if(m_Qx_out>0) {
                m_Concentrations_rk_dX[i]=(m_Concentrations_X_out[i]-m_Concentrations_rk_X_out[i])/tt;
            }
            else {
                m_Concentrations_rk_dX[i]=0;
                m_Concentrations_rk_X_out[i] = m_Concentrations_X_out[i];  //заполняется объем исходной концентрацией в фазе X

                m_Qx_out =0;
            }

            if(m_Qy_out>0) {
                m_Concentrations_rk_dY[i]=(m_Concentrations_Y_out[i]-m_Concentrations_rk_Y_out[i])/tl;
            }
            else {
                m_Concentrations_rk_dY[i]=0;
                m_Concentrations_rk_Y_out[i] = m_Concentrations_Y_out[i];  //заполняется объем исходной концентрацией в фазе Y

                m_Qy_out =0;
            }

            y_out.push_back(m_Concentrations_rk_dX[i]);
            y_out.push_back(m_Concentrations_rk_dY[i]);
        }
    }
    else {
        y_out.push_back(0);
        y_out.push_back(0);
        y_out.push_back(0);
        y_out.push_back(0);
        for(int i = 0; i< m_Concentrations_Y_out.size()*4;i++)
            y_out.push_back(0);
    }

    return y_out;
}

void MP_EX_EXTRACTOR::ode1(std::vector<double> &y0, std::vector<double> &y, double t, double h)
{
    std::vector<double> dy = dydt(t, y0);
    if(dy.size() != y0.size()) return;
    else {
        y.resize(y0.size());

        for(size_t i = 0; i < y.size(); i++) {
            y[i] = y0[i] + h*dy[i];
        }
    }
}

bool MP_EX_EXTRACTOR::process(double t, double h, std::string &error)
{
    double Q_in_summ =0;
    //  double Q_out_svobodhiy =0;
    //  double Q_out_summ;

    double W_in =0;
    double W_in_summ =0;

    // Частота вращения
    if(InEng->getInput().empty()) m_f = 45;
    else m_f = InEng->getInput()[0];

    if(isSituationActive("ES_ENGINE_NOM")) {
        double nw;
        nw = m_f*(100.00 - paramToDouble("ENG_FREQ")) / 100.00;

        if(nw <= 0) {
            error = "Остановка электродвигателя!";
            return false;
        }
        if(nw > m_f) nw = m_f;

        m_f = nw;
    }

    if(m_f <= 0) {
        error = "Остановка электродвигателя!";
        return false;
    }

    if(isSituationActive("ES_ENGINE_CIRCUIT")) {
        error = "Обрыв цепи электропитания двигателя!";
        return false;
    }

    if(isSituationActive("ES_ENGINE_SHORT")) {
        error = "Короткое замыкание электродвигателя!";
        return false;
    }

    if(isSituationActive("ES_ENGINE_STOP")) {
        error = "Заклинивание электродвигателя!";
        return false;
    }

    // Входные компоненты
    // зануляем все текущие входные массы компонентотв
    for(int i = 0; i < m_input_Components_Mass.size(); i++) m_input_Components_Mass[i] = 0.00;

    // Водная фаза
    std::vector<double> VFin = InVF->getInput();
    if(VFin.size() == 6) {
        double flowIn = VFin[0]; // Входной расход в мл/ч
        m_Qx_in = flowIn / 3.6e9;

        if(isSituationActive("ES_FEED_VF")) {
            double coeff = paramToDouble("VF_HERM_PERCENT");
            if(coeff < 0.00) coeff = 0.00;
            if(coeff > 100.00) coeff = 100.00;

            m_Qx_in *= (100.00 - coeff) / 100.00;
        }
        if(m_Qx_in < 0) m_Qx_in = 0;

        m_Tx = VFin[1];
        W_in = m_Qx_in*h*4200*(m_Tx-m_T);

        m_input_Components_Mass[0] += VFin[2]*m_Qx_in*h;
        m_Concentrations_X_in[0] = VFin[2];

        m_input_Components_Mass[1] += VFin[3]*m_Qx_in*h;
        m_Concentrations_X_in[1] = VFin[3];

        m_input_Components_Mass[2] += VFin[4]*m_Qx_in*h;
        m_Concentrations_X_in[2] = VFin[4];

        m_input_Components_Mass[3] += VFin[5]*m_Qx_in*h;
        m_Concentrations_X_in[3] = VFin[5];
    }
    else {
        error = "На подачу ВФ экстрактора ничего не подключено!";
        return false;
    }

    Q_in_summ += m_Qx_in;
    W_in_summ += W_in;

    // Органическая фаза
    std::vector<double> OFin = InOF->getInput();
    if(OFin.size() == 6) {
        double flowIn = OFin[0];

        // m_Qy_in = OFin[0];
        m_Qy_in = flowIn / 3.6e9;

        if(isSituationActive("ES_FEED_OF")) {
            double coeff = paramToDouble("OF_HERM_PERCENT");
            if(coeff < 0.00) coeff = 0.00;
            if(coeff > 100.00) coeff = 100.00;

            m_Qy_in *= (100.00 - coeff) / 100.00;
        }
        if(m_Qy_in < 0) m_Qy_in = 0;

        m_Ty = OFin[1];
        W_in = m_Qy_in*h*4200*(m_Ty-m_T);

        m_input_Components_Mass[0] += OFin[2]*m_Qy_in*h;
        m_Concentrations_Y_in[0] = OFin[2];

        m_input_Components_Mass[1] += OFin[3]*m_Qy_in*h;
        m_Concentrations_Y_in[1] = OFin[3];

        m_input_Components_Mass[2] += OFin[4]*m_Qy_in*h;
        m_Concentrations_Y_in[2] = OFin[4];

        m_input_Components_Mass[3] += OFin[5]*m_Qy_in*h;
        m_Concentrations_Y_in[3] = OFin[5];
    }
    else {
        error = "На подачу ОФ экстрактора ничего не подключено!";
        return false;
    }

    Q_in_summ += m_Qy_in;
    W_in_summ += W_in;

    H = Prev_H+h*((m_Qx_in+m_Qy_in) - (m_Qx_out+m_Qy_out))/m_geometry_S;   //уровень жидкости в аппарате

    if(H < 0) H=0;
    if(H > m_geometry_H) H = m_geometry_H;
    Prev_H = H;

    m_T_prev = m_T;

    // Расчет массы компонентов
    for(int i = 0; i < m_input_Components_Mass.size(); i++) {
        m_current_Components_Mass[i] += m_input_Components_Mass[i];
        m_current_Components_Concentration[i] = m_current_Components_Mass[i] / (H*m_geometry_S);
    }

    // Вывод компонентов Х разделительной камеры
    OutVF->setNewOut(0, m_Qx_out*3.6e9);
    OutVF->setNewOut(1, m_T);

    if(m_Qx_out > 0) {
        OutVF->setNewOut(2, m_Concentrations_rk_X_out[0]);
        OutVF->setNewOut(3, m_Concentrations_rk_X_out[1]);
        OutVF->setNewOut(4, m_Concentrations_rk_X_out[2]);
        OutVF->setNewOut(5, m_Concentrations_rk_X_out[3]);
    }
    else {
        OutVF->setNewOut(2, 0);
        OutVF->setNewOut(3, 0);
        OutVF->setNewOut(4, 0);
        OutVF->setNewOut(5, 0);
    }

    // Вывод компонентов У разделительной камеры
    OutOF->setNewOut(0, m_Qy_out*3.6e9);
    OutOF->setNewOut(1, m_T);

    if(m_Qy_out > 0) {
        OutOF->setNewOut(2, m_Concentrations_rk_Y_out[0]);
        OutOF->setNewOut(3, m_Concentrations_rk_Y_out[1]);
        OutOF->setNewOut(4, m_Concentrations_rk_Y_out[2]);
        OutOF->setNewOut(5, m_Concentrations_rk_Y_out[3]);
    }
    else {
        OutOF->setNewOut(2, 0);
        OutOF->setNewOut(3, 0);
        OutOF->setNewOut(4, 0);
        OutOF->setNewOut(5, 0);
    }

    // Дополнительный вывод
    OutSensors->setNewOut(0, m_Qx_out);
    OutSensors->setNewOut(1, m_Qy_out);
    OutSensors->setNewOut(2, m_Vx);
    OutSensors->setNewOut(3, m_Vy);

    std::vector<double> y0;
    y0.clear();
    y0.push_back(m_Qx_out);
    y0.push_back(m_Qy_out);
    y0.push_back(m_Vx);
    y0.push_back(m_Vy);

    for(int i = 0; i < m_Concentrations_Y_out.size(); i++) {
        y0.push_back(m_Concentrations_X_out[i]);
        y0.push_back(m_Concentrations_Y_out[i]);
    }

    for(int i = 0; i < m_Concentrations_Y_out.size(); i++) {
        y0.push_back(m_Concentrations_rk_X_out[i]);
        y0.push_back(m_Concentrations_rk_Y_out[i]);
    }

    // Вызов решателя
    std::vector<double> y;
    y.clear();

    ode1(y0, y, t, h);
    if(y.empty()) {
        error = "Не удалось решить ДУ экстрактора!";
        return false;
    }

    std::vector<double>::iterator it_y = y.begin();
    m_Qx_out = *it_y; it_y++;
    m_Qy_out = *it_y; it_y++;
    m_Vx = *it_y; it_y++;
    m_Vy = *it_y; it_y++;

    for(int i = 0; i < m_Concentrations_Y_out.size(); i++) {
        m_Concentrations_X_out[i] = *it_y; it_y++;
        m_Concentrations_Y_out[i] = *it_y; it_y++;
    }

    for(int i = 0; i < m_Concentrations_Y_out.size(); i++) {
        m_Concentrations_rk_X_out[i] = *it_y; it_y++;
        m_Concentrations_rk_Y_out[i] = *it_y; it_y++;
    }

    m_currentvolume += h*(m_Qx_in + m_Qy_in - m_Qx_out - m_Qy_out);
    if(m_currentvolume > m_Vr + m_Vs) {
        error = "Переполнение аппарата!";
        return false;
    }

    return true;
}

ICalcElement *Create()
{
    return new MP_EX_EXTRACTOR();
}

void Release(ICalcElement *block)
{
    if(block->type() == "MP_EX_EXTRACTOR") delete block;
}

std::string Type()
{
    return "MP_EX_EXTRACTOR";
}
