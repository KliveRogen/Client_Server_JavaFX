#include "TR_VAPE_13T_AD8445.h"

TR_VAPE_13T_AD8445::TR_VAPE_13T_AD8445()
{
	// Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
    createParameter("ctrlSignal", "0");
    createParameter("flowOut", "0");
    createParameter("iv_mall", "0");
    createParameter("iv_t", "0");
    createParameter("iv_cu", "0");
    createParameter("iv_chno3", "0");
    createParameter("iv_ctbf", "0");
    createParameter("iv_ccnhn", "0");
    createParameter("as_dv_l", "0");
    createParameter("al_dv_l", "0");

	// Сигналы блока
    createSignal("sig_flowOut", Signal::ST_DOUBLE);
	createSignal("sm_l", Signal::ST_DOUBLE);

	// Порты блока
    ins = createInputPort(0, "ins", "INFO");
    outinfo = createOutputPort(1, "outinfo", "INFO");
    outinfosen = createOutputPort(2, "outinfosen", "INFO");

	// Отказы блока

}

void TR_VAPE_13T_AD8445::setDataNames()
{
    std::vector<std::string> out,out1;
    out.push_back("Количество раствора в емкости, кг");
    out.push_back("Температура раствора, оС");
    out.push_back("Концентрация U, г/л");
    out.push_back("Концентрация HNO3, г/л");
    out.push_back("Концентрация ТБФ, г/л");
    out.push_back("Концентрация CnHn, г/л");
    outinfo->setDataNames(out);
    out1.push_back("Уровень, м3");
    outinfosen->setDataNames(out1);
}

bool TR_VAPE_13T_AD8445::init(std::string &error, double h)
{
    //инициализации
        for(int l=0; l<tnum; l++)
        {
            for(int i=0;i<el;i++)
                MM[i] = 0;
            TT = 0;
        }
        // датчики
        for (int i=0;i<dv;i++) std::fill(datv[i].val,datv[i].val+2,0);
        // 0 - датчик L
        int cou=0;
        datv[cou].bound[0] = 0;
        datv[cou].bound[1] = 27;
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
    //начальные условия
        double mas = paramToDouble(Parameters[0]),
               buf = 0;
        for (int i=0;i<el+1;i++) if (paramToDouble(Parameters[i])<0)
        {
            eflg = 1;
            break;
        }
        for (int i=0;i<el-1;i++)
        {
            MM[i+1] = mas*paramToDouble(Parameters[2+i])*kcd;
            buf = buf + MM[i+1];
            if (buf>mas) eflg = 1;
        }
        MM[0] = mas - buf;
        TT = paramToDouble(Parameters[1]) + 273;
	setDataNames();
    return true;
}

bool TR_VAPE_13T_AD8445::process(double t, double h, std::string &error)
{
    //инициализации
        //перем
        double  dMM[el],MMin1[el],MMin2[el],
                MMsum,fin1,fin2,CCin1[el],CCin2[el],
                tin1,tin2,buf,ll,lvlchk[4];
        //входы
        fin1 = ins->getInput()[0]/2;
        tin1 = ins->getInput()[1]+273;
        buf = 0;
        for(int i=0;i<el-1;i++)
        {
            CCin1[1+i] = ins->getInput()[2+i]*kcd;
            buf = buf + CCin1[1+i];
        }
        CCin1[0] = 1 - buf;
        fin2 = ins->getInput()[0+el+1]/2;
        tin2 = ins->getInput()[1+el+1]+273;
        buf = 0;
        for(int i=0;i<el-1;i++)
        {
            CCin2[1+i] = ins->getInput()[2+i+el+1]*kcd;
            buf = buf + CCin2[1+i];
        }
        CCin2[0] = 1 - buf;
        //аварии
        AA[0].trig = paramToInt("al_dv_l");
        AA[0].opt = paramToInt("as_dv_l");

        /*
        for (int i=0;i<alpar;i++)
        {
            AA[i].trig = paramToInt(Parameters[cpar+ivpar+alpar+i]);
            AA[i].opt = paramToInt(Parameters[cpar+ivpar+i]);
        }
        */
    //вспомогательный расчет
        //масса
        MMsum = 0;
        for(int i=0;i<el;i++)
            MMsum = MMsum + MM[i];
    //основной расчет
        //температура
        if (MMsum < cap)
            TT = 20 + 273;
        else
            TT = (TT*MMsum+tin1*fin1*h+tin2*fin2*h)/(MMsum+fin1*h+fin2*h);
        //масса
        for(int i=0;i<el;i++)
        {
            MMin1[i] = CCin1[i]*fin1;
            MMin2[i] = CCin2[i]*fin2;
            dMM[i] = MMin1[i]+MMin2[i];
            MM[i] = MM[i] + dMM[i]*h;
            if (MM[i] < cap) MM[i] = 0;
        }
    //выходы
        //датчики
        for (int i = 0; i < 4; i++)
            lvlchk[i] = lvl*lvltst[i];
        ll = MMsum/dens;
        if ( ll/sef > lvlchk[2] ) eflg = 2;
        datv[0].val[0] = ll;
        for (int i=0;i<dv;i++)  //дискр
        {
            // копирование
            datv[i].val[1] = datv[i].val[0];
            // аварии
            if (AA[i].trig==1) {
                datv[i].val[1] = dverr(datv[i].val[1], AA[i].opt, h, &AA[i]);
            }
            // ограничение
            if (datv[i].val[1] > datv[i].bound[1]) {
                datv[i].val[1] = datv[i].bound[1];
            }
            if (datv[i].val[1] < datv[i].bound[0]) {
                datv[i].val[1] = datv[i].bound[0];
            }
        }
        Signals[0]->Value.doubleVal = datv[0].val[1];
        //выходы
        outinfosen->setNewOut(0,datv[0].val[1]);
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
            error = "Ошибка блока 13)АД-8445(емк.):\n";

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
    return new TR_VAPE_13T_AD8445();
}

void Release(ICalcElement *block)
{
    if(block->type() == "TR_VAPE_13T_AD8445") delete block;
}

std::string Type()
{
    return "TR_VAPE_13T_AD8445";
}
