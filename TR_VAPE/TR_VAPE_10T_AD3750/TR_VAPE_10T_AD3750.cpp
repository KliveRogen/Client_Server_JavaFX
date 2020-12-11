#include "TR_VAPE_10T_AD3750.h"

TR_VAPE_10T_AD3750::TR_VAPE_10T_AD3750()
{
	// Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
    createParameter("iv_mall", "0");        //0
    createParameter("iv_t", "0");           //1
    createParameter("iv_cu", "0");          //2
    createParameter("iv_chno3", "0");       //3
    createParameter("iv_ctbf", "0");        //4
    createParameter("iv_ccnhn", "0");       //5
    createParameter("as_dv_lvl", "0");      //6
    createParameter("as_dv_t", "0");        //7
    createParameter("as_dl_lo", "0");       //8
    createParameter("as_dl_lt", "0");       //9
    createParameter("al_dv_lvl", "0");      //10
    createParameter("al_dv_t", "0");        //11
    createParameter("al_dl_lo", "0");       //12
    createParameter("al_dl_lt", "0");       //13

	// Сигналы блока
	createSignal("sm_lvl", Signal::ST_DOUBLE);
	createSignal("sm_t", Signal::ST_DOUBLE);
	createSignal("sm_lo", Signal::ST_DOUBLE);
	createSignal("sm_lt", Signal::ST_DOUBLE);

	// Порты блока
    inp1 = createInputPort(0, "inp1", "INFO");
    inp2 = createInputPort(1, "inp2", "INFO");
    ins = createInputPort(2, "ins", "INFO");
    outp1 = createOutputPort(3, "outp1", "INFO");
    outp2 = createOutputPort(4, "outp2", "INFO");
    outinfo = createOutputPort(5, "outinfo", "INFO");
    outinfosen = createOutputPort(6, "outinfosen", "INFO");

	// Отказы блока

}

void TR_VAPE_10T_AD3750::setDataNames()
{
    std::vector<std::string> out1,out2,out3,out4;
    out1.push_back("Расход раствора, кг/с");
    out1.push_back("Температура раствора, оС");
    out1.push_back("Концентрация U, г/л");
    out1.push_back("Концентрация HNO3, г/л");
    out1.push_back("Концентрация ТБФ, г/л");
    out1.push_back("Концентрация СnHn, г/л");
    outp1->setDataNames(out1);
    out2.push_back("Расход раствора, кг/с");
    out2.push_back("Температура раствора, оС");
    out2.push_back("Концентрация U, г/л");
    out2.push_back("Концентрация HNO3, г/л");
    out2.push_back("Концентрация ТБФ, г/л");
    out2.push_back("Концентрация СnHn, г/л");
    outp2->setDataNames(out2);
    out3.push_back("Количество раствора в емкости, кг");
    out3.push_back("Температура раствора, оС");
    out3.push_back("Концентрация U, г/л");
    out3.push_back("Концентрация HNO3, г/л");
    out3.push_back("Концентрация ТБФ, г/л");
    out3.push_back("Концентрация CnHn, г/л");
    outinfo->setDataNames(out3);
    out4.push_back("Уровень, м3");
    out4.push_back("Температура, оС");
    out4.push_back("П");
    out4.push_back("ВУ");
    outinfosen->setDataNames(out4);
}

bool TR_VAPE_10T_AD3750::init(std::string &error, double h)
{
    //инициализации
        for(int i=0;i<el;i++)
            MM[i] = 0;
        TT = 0;
        // датчики
        for (int i=0;i<dv;i++) std::fill(datv[i].val,datv[i].val+2,0);
        // 0 - датчик L
        int cou=0;
        datv[cou].bound[0] = 0;
        datv[cou].bound[1] = 5.1;
        // 0 - датчик L
        cou++;
        datv[cou].bound[0] = 0;
        datv[cou].bound[1] = 150;
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

bool TR_VAPE_10T_AD3750::process(double t, double h, std::string &error)
{
    //инициализации
        //перем
        double  dMM[el],MMin[el],MMout1[el],MMout2[el],
                MMsum,fin,CCin[el],tin,buf,foutp1,foutp2,
                ll,lvlchk[4],fout1,fout2;
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
        foutp1 = inp1->getInput()[0];
        foutp2 = inp2->getInput()[0];
        //аварии
        for (int i=0;i<alpar;i++)
        {
            AA[i].trig = paramToInt(Parameters[ivpar+alpar+i]);
            AA[i].opt = paramToInt(Parameters[ivpar+i]);
        }
    //вспомогательный расчет
        //масса
        MMsum = 0;
        for(int i=0;i<el;i++)
            MMsum = MMsum + MM[i];
        //расходы
        fout1 = 0;
        fout2 = 0;
        if((foutp1+foutp2)*h<MMsum)
        {
            fout1 = foutp1;
            fout2 = foutp2;
        }
        else
        {
            if((foutp1>cap) && (foutp2>cap))
            {
                fout1 = MMsum/h/2;
                fout2 = MMsum/h/2;
            }
            if((foutp1<cap) && (foutp2>cap))
            {
                fout1 = 0;
                fout2 = MMsum/h;
            }
            if((foutp1>cap) && (foutp2<cap))
            {
                fout1 = MMsum/h;
                fout2 = 0;
            }
        }
    //основной расчет
        //температура
        if (MMsum < cap)
            TT = 20 + 273;
        else
            TT = (TT*MMsum+tin*fin*h)/(MMsum+fin*h);
        //масса
        for(int i=0;i<el;i++)
        {
            MMout1[i] = 0;
            MMout2[i] = 0;
            MMin[i] = CCin[i]*fin;
            if (MMsum > cap)
            {
                MMout1[i] = fout1*MM[i]/MMsum;
                MMout2[i] = fout2*MM[i]/MMsum;
            }
            dMM[i] = MMin[i] - MMout1[i] - MMout2[i];
            MM[i] = MM[i] + dMM[i]*h;
            if (MM[i] < cap) MM[i] = 0;
        }
    //выходы
        //датчики
        ll = MMsum/dens;
        for (int i = 0; i < 4 ; i++)
            lvlchk[i] = lvl*lvltst[i];
        if ( ll/sef > lvlchk[2] ) eflg = 2;
        datv[0].val[0] = ll;
        datv[1].val[0] = TT-273;
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
        }
        Signals[0]->Value.doubleVal = datv[0].val[1];
        Signals[1]->Value.doubleVal = datv[1].val[1];
        //индик
        datl[0].val[0] = 0;
        if ( ll/sef > lvlchk[2]) datl[0].val[0] = 1;
        datl[1].val[0] = 0;
        if ( ll/sef > lvlchk[1]) datl[1].val[0] = 1;
        for (int i=0;i<dl;i++)  //дискр
        {
            // копирование
            datl[i].val[1] = datl[i].val[0];
            // аварии
            if (AA[dv+i].trig==1) {}
                //datl[i].val[1] = dlerr(datl[i].val[1], AA[dv+i].opt);
        }
        Signals[2]->Value.boolVal = datl[0].val[1];
        Signals[3]->Value.boolVal = datl[1].val[1];
        //выходы
        //вывод датчки
        outinfosen->setNewOut(0,datv[0].val[1]);
        outinfosen->setNewOut(1,datv[1].val[1]);
        outinfosen->setNewOut(2,datl[0].val[1]);
        outinfosen->setNewOut(3,datl[1].val[1]);
        // отвод 1
        buf = 0;
        for(int i = 0; i < el; i++)
            buf = buf + MMout1[i];
        if (buf > cap)
        {
            outp1->setNewOut(0,fout1);
            outp1->setNewOut(1,TT-273);
            outp1->setNewOut(2,MMout1[1]/buf/kcd);
            outp1->setNewOut(3,MMout1[2]/buf/kcd);
            outp1->setNewOut(4,MMout1[3]/buf/kcd);
            outp1->setNewOut(5,MMout1[4]/buf/kcd);
        }
        else
            for (int i = 0; i < (el+1); i++)
                outp1->setNewOut(i,0);
        // отвод 2
        buf = 0;
        for(int i = 0; i < el; i++)
            buf = buf + MMout2[i];
        if (buf > cap)
        {
            outp2->setNewOut(0,fout2);
            outp2->setNewOut(1,TT-273);
            outp2->setNewOut(2,MMout2[1]/buf/kcd);
            outp2->setNewOut(3,MMout2[2]/buf/kcd);
            outp2->setNewOut(4,MMout2[3]/buf/kcd);
            outp2->setNewOut(5,MMout2[4]/buf/kcd);
        }
        else
            for (int i = 0; i < (el+1); i++)
                outp2->setNewOut(i,0);
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
            error = "Ошибка блока 10)АД-3750(емк.):\n";
            if(eflg==1)
            {
                error = error + "Ошибка в заполнении исходных данных!";
            }
            if(eflg==2)
            {
                error = error + "Переполнение емкости!";
            }
            return false;
        }
    return true;
}


ICalcElement *Create()
{
    return new TR_VAPE_10T_AD3750();
}

void Release(ICalcElement *block)
{
    if(block->type() == "TR_VAPE_10T_AD3750") delete block;
}

std::string Type()
{
    return "TR_VAPE_10T_AD3750";
}
