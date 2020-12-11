#include "TR_VAPE_05V_AD3746.h"

TR_VAPE_05V_AD3746::TR_VAPE_05V_AD3746()
{
	// Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
    createParameter("spcont", "0");         //0
    createParameter("pc_kl0", "0");         //1
    createParameter("pc_kl1", "0");         //2
    createParameter("iv_mall", "0");        //3
    createParameter("iv_t", "0");           //4
    createParameter("iv_cu", "0");          //5
    createParameter("iv_chno3", "0");       //6
    createParameter("iv_ctbf", "0");        //7
    createParameter("iv_ccnhn", "0");       //8
    createParameter("as_dv_p1", "0");       //9
    createParameter("as_dv_p2", "0");       //10
    createParameter("as_dv_p3", "0");       //11
    createParameter("as_dv_t1", "0");       //12
    createParameter("as_dv_t2", "0");       //13
    createParameter("as_dv_t3", "0");       //14
    createParameter("as_dv_lvl", "0");      //15
    createParameter("as_dv_kl0", "0");      //16
    createParameter("as_dv_kl1", "0");      //17
    createParameter("as_av_kl0", "0");      //18
    createParameter("as_av_kl1", "0");      //19
    createParameter("al_dv_p1", "0");       //20
    createParameter("al_dv_p2", "0");       //21
    createParameter("al_dv_p3", "0");       //22
    createParameter("al_dv_t1", "0");       //23
    createParameter("al_dv_t2", "0");       //24
    createParameter("al_dv_t3", "0");       //25
    createParameter("al_dv_lvl", "0");      //26
    createParameter("al_dv_kl0", "0");      //27
    createParameter("al_dv_kl1", "0");      //28
    createParameter("al_av_kl0", "0");      //29
    createParameter("al_av_kl1", "0");      //30

	// Сигналы блока
    createSignal("sm_p1", Signal::ST_DOUBLE);       //0
    createSignal("sm_p2", Signal::ST_DOUBLE);       //1
    createSignal("sm_p3", Signal::ST_DOUBLE);       //2
    createSignal("sm_t1", Signal::ST_DOUBLE);       //3
    createSignal("sm_t2", Signal::ST_DOUBLE);       //4
    createSignal("sm_t3", Signal::ST_DOUBLE);       //5
    createSignal("sm_lvl", Signal::ST_DOUBLE);      //6
    createSignal("sm_kl0", Signal::ST_DOUBLE);      //7
    createSignal("sm_kl1", Signal::ST_DOUBLE);      //8
    createSignal("sc_kl0", Signal::ST_DOUBLE);      //9
    createSignal("sc_kl1", Signal::ST_DOUBLE);      //10

	// Порты блока
    ins = createInputPort(0, "ins", "INFO");
    inv = createInputPort(1, "inv", "INFO");
    outv = createOutputPort(2, "outv", "INFO");
    outs = createOutputPort(3, "outs", "INFO");
    outinfo = createOutputPort(4, "outinfo", "INFO");
    outinfosen = createOutputPort(5, "outinfosen", "INFO");

	// Отказы блока

}

void TR_VAPE_05V_AD3746::setDataNames()
{
    std::vector<std::string> out1,out2,out3,out4;
    out1.push_back("Расход сок. пара, кг/с");
    out1.push_back("Температура сок. пара, оС");
    out1.push_back("Концентрация U, г/л");
    out1.push_back("Концентрация HNO3, г/л");
    out1.push_back("Концентрация ТБФ, г/л");
    out1.push_back("Концентрация СnHn, г/л");
    outv->setDataNames(out1);
    out2.push_back("Расход упар. раствора, кг/с");
    out2.push_back("Температура упар. раствора, оС");
    out2.push_back("Концентрация U, г/л");
    out2.push_back("Концентрация HNO3, г/л");
    out2.push_back("Концентрация ТБФ, г/л");
    out2.push_back("Концентрация СnHn, г/л");
    outs->setDataNames(out2);
    out3.push_back("Количество раствора в емкости, кг");
    out3.push_back("Температура раствора, оС");
    out3.push_back("Концентрация U, г/л");
    out3.push_back("Концентрация HNO3, г/л");
    out3.push_back("Концентрация ТБФ, г/л");
    out3.push_back("Концентрация CnHn, г/л");
    outinfo->setDataNames(out3);
    out4.push_back("Давление 1, кгс/см2");
    out4.push_back("Давление 2, кгс/см2");
    out4.push_back("Давление 3, кгс/см2");
    out4.push_back("Температура 1, оС");
    out4.push_back("Температура 2, оС");
    out4.push_back("Температура 3, оС");
    out4.push_back("Уровень, %");
    out4.push_back("Клапан 1, %");
    out4.push_back("Клапан 2, %");
    outinfosen->setDataNames(out4);
}

bool TR_VAPE_05V_AD3746::init(std::string &error, double h)
{
    //инициализации
        for(int i=0;i<el;i++)
            MM[i] = 0;
        TT = 0;
        // датчики
        for (int i=0;i<dv;i++) std::fill(datv[i].val,datv[i].val+2,0);
        int cou=0;
        datv[cou].bound[0] = -1;
        datv[cou].bound[1] = 9;//5e2;
        cou++;
        datv[cou].bound[0] = 0;
        datv[cou].bound[1] = 9;//5e2;
        cou++;
        datv[cou].bound[0] = -1;
        datv[cou].bound[1] = 1.5;//5e2;
        cou++;
        datv[cou].bound[0] = 0;
        datv[cou].bound[1] = 200;
        cou++;
        datv[cou].bound[0] = 0;
        datv[cou].bound[1] = 200;
        cou++;
        datv[cou].bound[0] = 0;
        datv[cou].bound[1] = 200;
        cou++;
        datv[cou].bound[0] = 0;
        datv[cou].bound[1] = 100;
        cou++;
        datv[cou].bound[0] = 0;
        datv[cou].bound[1] = 100;
        cou++;
        datv[cou].bound[0] = 0;
        datv[cou].bound[1] = 100;
        for (int i=0;i<av;i++) std::fill(actv[i].val,actv[i].val+2,0);
        // 0 - каньон
        cou=0;
        actv[cou].bound[0] = 0;
        actv[cou].bound[1] = 1;
        cou++;
        actv[cou].bound[0] = 0;
        actv[cou].bound[1] = 1;
        //аварии
        for (int i=0;i<sit;i++)
        {
            AA[i].trig = 0;
            AA[i].eflag = 0;
            AA[i].opt = 0;
            AA[i].ecou = 0;
            AA[i].eval_i[0] = 0;
            AA[i].eval_i[1] = 0;
        }
        eflg = 0;
        kmout[0] = 1;
        kmout[1] = 1;
        kmout[2] = 1;
        kmout[3] = 1;
        kmout[4] = 1;
        kmvape[0] = 1;
        kmvape[1] = 0;
        kmvape[2] = 0.1;
        kmvape[3] = 1;
        kmvape[4] = 1;
    //начальные условия
        double mas = paramToDouble(Parameters[0+cpar]),
               buf = 0;
        for (int i=0;i<el+1;i++) if (paramToDouble(Parameters[i+cpar])<0)
        {
            eflg = 1;
            break;
        }
        for (int i=0;i<el-1;i++)
        {
            MM[i+1] = mas*paramToDouble(Parameters[2+i+cpar])*kcd;
            buf = buf + MM[i+1];
            if (buf>mas) eflg = 1;
        }
        MM[0] = mas - buf;
        TT = paramToDouble(Parameters[1+cpar]) + 273;
        ph1_i = pa;
	setDataNames();
    return true;
}

bool TR_VAPE_05V_AD3746::process(double t, double h, std::string &error)
{
    //инициализации
        //перем
        double  dMM[el],MMin[el],ph0,th0,kl1,kl2,
                MMsum,fin,CCin[el],tin,buf,ktem1,ktem2,
                ll,lvlchk[4],qheat,MMout[el],llb,llt,
                MMvape[el],fvape,fout,dt,ph1,ps;
        //входы
        fin = ins->getInput()[0];
        tin = ins->getInput()[1]+273;
        buf = 0;
        for(int i=0;i<el-1;i++)
        {
            CCin[1+i] = ins->getInput()[2+i]*kcd;
            buf = buf + CCin[1+i];
        }
        CCin[0] = 1 - buf;
        ph0 = inv->getInput()[0]*1e3;
        th0 = inv->getInput()[1]+273;
        //аварии
        for (int i=0;i<alpar;i++)
        {
            AA[i].trig = paramToInt(Parameters[cpar+ivpar+alpar+i]);
            AA[i].opt = paramToInt(Parameters[cpar+ivpar+i]);
        }
        // параметры
        if (paramToInt(Parameters[0]) == 0)
        {
            actv[0].val[0] = paramToDouble(Parameters[1]);
            actv[1].val[0] = paramToDouble(Parameters[2]);
        }
        else
        {
            actv[0].val[0] = Signals[9]->Value.doubleVal;
            actv[1].val[0] = Signals[10]->Value.doubleVal;
        }
        actv[0].val[0] = actv[0].val[0]/100;
        actv[1].val[0] = actv[1].val[0]/100;
        for (int i=0;i<av;i++)
        {
            // копирование
            actv[i].val[1] = actv[i].val[0];
            // аварии
            if (AA[dvsit+dlsit+i].trig == 1){}
                //actv[i].val[1] = averr(actv[i].val[1], AA[dvsit+dlsit+i].opt, &AA[dvsit+dlsit+i]);
            // ограничение
            if (actv[i].val[1] > actv[i].bound[1]) actv[i].val[1] = actv[i].bound[1];
            if (actv[i].val[1] < actv[i].bound[0]) actv[i].val[1] = actv[i].bound[0];
        }
        kl1 = actv[0].val[1];
        kl2 = actv[1].val[1];
    //вспомогательный расчет
        MMsum = 0;
        for (int i = 0; i < el; i++)
            MMsum = MMsum + MM[i];
        qheat = ksheat*(th0-TT)*kl1*kl2;
        ktem1 = (TT-273)/(tb-273);
        ktem2 = (ta-273)/(TT-273);
        llb = lvl - lvlm;
        ll = MMsum/(dens*sef);
        llt = ll - llb;
        fvape = qheat*ktem1/lamb;
        fout = 3.5*997*llt/3600;
        kmout[3] = 1;
        kmout[4] = 1;
        if (TT>95+273) // улетучивание тбф и CnHn при начале кипения
        {
            kmout[3] = 0;
            kmout[4] = 0;
        }
    //основной расчет
        //температура
        dt = (qheat*(1-ktem1) - qcool*(1-ktem2))/hcap*MMsum;
        TT = TT +dt*h;
        if (MMsum < cap)
            TT = 20 + 273;
        else
            TT = (TT*MMsum+tin*fin*h)/(MMsum+fin*h);
        if (TT > tb) TT = tb;
        //масса
        for(int i=0;i<el;i++)
        {
            MMout[i] = 0;
            MMvape[i] = 0;
            MMin[i] = CCin[i]*fin;
            if (MMsum > cap)
            {
                MMout[i] = kmout[i]*fout*MM[i]/MMsum;
                MMvape[i] = kmvape[i]*fvape*MM[i]/MMsum;
            }
            dMM[i] = MMin[i] - MMout[i] - MMvape[i];
            MM[i] = MM[i] + dMM[i]*h;
            if (MM[i] < cap) MM[i] = 0;
        }
        //давление
        ph1 = pa;
        if ((kl1 <= cap) && (kl2 <= cap)) ph1 = ph1_i;
        if ((kl1 <= cap) && (kl2 >= cap)) ph1 = pa;
        if ((kl1 >= cap) && (kl2 <= cap)) ph1 = ph0;
        if ((kl1 >= cap) && (kl2 >= cap)) ph1 = ph0*0.7;
        ph1_i = ph1;
        ps = 1e5 - 3.3e3*ktem1;
    //выгрузка
        //датчики
        for (int i = 0; i < 4 ; i++)
            lvlchk[i] = lvl*lvltst[i];
        if ( ll > lvlchk[3] ) eflg = 2;
        datv[0].val[0] = (ph0-pa)/coeffPress;
        datv[1].val[0] = (ph1-pa)/coeffPress;
        datv[2].val[0] = (ps-pa)/coeffPress;
        datv[3].val[0] = TT-273;
        datv[4].val[0] = TT-273-0.5*ktem1;
        datv[5].val[0] = TT-273-3*ktem1;
        datv[6].val[0] = llt/lvlm*100;
        datv[7].val[0] = kl1*1e2;
        datv[8].val[0] = kl2*1e2;
        for (int i=0;i<dv;i++)  //дискр
        {
            // копирование
            datv[i].val[1] = datv[i].val[0];
            // аварии
            if (AA[i].trig==1) {}
                //datv[i].val[1] = dverr(datv[i].val[1], AA[i].opt, h, &AA[i]);
            // ограничение
            if (datv[i].val[1]>datv[i].bound[1]) datv[i].val[1] = datv[i].bound[1];
                else if(datv[i].val[1]<datv[i].bound[0]) datv[i].val[1]=datv[i].bound[0];
            //вывод
            Signals[i]->Value.doubleVal = datv[i].val[1];
        }
        outinfosen->setNewOut(0,datv[0].val[1]);
        outinfosen->setNewOut(1,datv[1].val[1]);
        outinfosen->setNewOut(2,datv[2].val[1]);
        outinfosen->setNewOut(3,datv[3].val[1]);
        outinfosen->setNewOut(4,datv[4].val[1]);
        outinfosen->setNewOut(5,datv[5].val[1]);
        outinfosen->setNewOut(6,datv[6].val[1]);
        outinfosen->setNewOut(7,datv[7].val[1]);
        outinfosen->setNewOut(8,datv[8].val[1]);
        // отвод пар
        buf = 0;
        for(int i = 0; i < el; i++)
            buf = buf + MMvape[i];
        if (buf > cap)
        {
            outv->setNewOut(0,buf);
            outv->setNewOut(1,TT-273);
            outv->setNewOut(2,MMvape[1]/buf/kcd);
            outv->setNewOut(3,MMvape[2]/buf/kcd);
            outv->setNewOut(4,MMvape[3]/buf/kcd);
            outv->setNewOut(5,MMvape[4]/buf/kcd);
        }
        else
            for (int i = 0; i < (el+1); i++)
                outv->setNewOut(i,0);
        // отвод раст
        buf = 0;
        for(int i = 0; i < el; i++)
            buf = buf + MMout[i];
        if (buf > cap)
        {
            outs->setNewOut(0,buf);
            outs->setNewOut(1,TT-273);
            outs->setNewOut(2,MMout[1]/buf/kcd);
            outs->setNewOut(3,MMout[2]/buf/kcd);
            outs->setNewOut(4,MMout[3]/buf/kcd);
            outs->setNewOut(5,MMout[4]/buf/kcd);
        }
        else
            for (int i = 0; i < (el+1); i++)
                outs->setNewOut(i,0);
        // информационный
        buf = 0;
        for(int i = 0; i < el; i++)
            buf = buf + MM[i];
        if (buf > cap)
        {
            outinfo->setNewOut(0,buf);
            outinfo->setNewOut(1,TT-273);
            outinfo->setNewOut(2,MM[1]/buf/kcd);
            outinfo->setNewOut(3,MM[2]/buf/kcd);
            outinfo->setNewOut(4,MM[3]/buf/kcd);
            outinfo->setNewOut(5,MM[4]/buf/kcd);
        }
        else
            for (int i = 0; i < (el+1); i++)
                outinfo->setNewOut(i,0);
        //Отказы
        if(eflg!=0)
        {
            error = "Ошибка блока 05)АД-3746(вып.):\n";
            if(eflg==1)
            {
                error = error + "Ошибка в заполнении исходных данных!";
            }
            if(eflg==2)
            {
                error = error + "Переполнение емкости!";
            }
            if(eflg==3)
            {
                error = error + "Ошибка в заполнении управляющих сигналов!";
            }
            return false;
        }
    return true;
}


ICalcElement *Create()
{
    return new TR_VAPE_05V_AD3746();
}

void Release(ICalcElement *block)
{
    if(block->type() == "TR_VAPE_05V_AD3746") delete block;
}

std::string Type()
{
    return "TR_VAPE_05V_AD3746";
}
