#include "TR_VAPE_12P_AD37502.h"

TR_VAPE_12P_AD37502::TR_VAPE_12P_AD37502()
{
	// Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
	createParameter("spcont", "0");
	createParameter("pc_ppow", "0");
	createParameter("as_dv_p", "0");
	createParameter("as_dv_i", "0");
	createParameter("as_dv_f", "0");
	createParameter("as_dl_ind", "0");
	createParameter("as_av_ppow", "0");
	createParameter("al_dv_p", "0");
	createParameter("al_dv_i", "0");
	createParameter("al_dv_f", "0");
	createParameter("al_dl_ind", "0");
	createParameter("al_av_ppow", "0");

	// Сигналы блока
	createSignal("sm_p", Signal::ST_DOUBLE);
	createSignal("sm_i", Signal::ST_DOUBLE);
	createSignal("sm_f", Signal::ST_DOUBLE);
	createSignal("sm_ind", Signal::ST_DOUBLE);
	createSignal("sc_ppow", Signal::ST_DOUBLE);

	// Порты блока
    ins = createInputPort(0, "ins", "INFO");
    outs = createOutputPort(1, "out", "INFO");
    outinfo = createOutputPort(2, "outinfo", "INFO");
    outp = createOutputPort(3, "outp", "INFO");
    outinfosen = createOutputPort(4, "outinfosen", "INFO");

	// Отказы блока

}

void TR_VAPE_12P_AD37502::setDataNames()
{
    std::vector<std::string> out1,out2,out3,out4;
    out1.push_back("Расход, кг/с");
    out1.push_back("Температура раствора, оС");
    out1.push_back("Концентрация U, г/л");
    out1.push_back("Концентрация HNO3, г/л");
    out1.push_back("Концентрация ТБФ, г/л");
    out1.push_back("Концентрация CnHn, г/л");
    outs->setDataNames(out1);
    out2.push_back("Ожидаемый расход, кг/с");
    outp->setDataNames(out2);
    out3.push_back("Расход, кг/с");
    out3.push_back("Температура раствора, оС");
    out3.push_back("Концентрация U, г/л");
    out3.push_back("Концентрация HNO3, г/л");
    out3.push_back("Концентрация ТБФ, г/л");
    out3.push_back("Концентрация CnHn, г/л");
    out3.push_back("Мощность насоса, %");
    outinfo->setDataNames(out3);
    out4.push_back("Давление, кгс/см2");
    out4.push_back("Ток, А");
    out4.push_back("Обороты двигателя, об/мин");
    out4.push_back("Ж");
    outinfosen->setDataNames(out4);
}

bool TR_VAPE_12P_AD37502::init(std::string &error, double h)
{
    //инициализации
        for(int i=0;i<el;i++)
            MM[i] = 0;
        TT = 0;
        // датчики
        for (int i=0;i<dv;i++) std::fill(datv[i].val,datv[i].val+2,0);
        // 0 - датчик P
        int cou=0;
        datv[cou].bound[0] = -1;
        datv[cou].bound[1] = 9;
        // 0 - датчик I
        cou++;
        datv[cou].bound[0] = 0;
        datv[cou].bound[1] = 10;
        // 0 - датчик F
        cou++;
        datv[cou].bound[0] = 0;
        datv[cou].bound[1] = 3000;
        for (int i=0;i<av;i++) std::fill(actv[i].val,actv[i].val+2,0);
        //
        cou=0;
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
        fp = 0;
	setDataNames();
    return true;
}

bool TR_VAPE_12P_AD37502::process(double t, double h, std::string &error)
{
    //инициализации
        //перем
        double  fin,CCin[el],tin,buf,
                ppow,dfp,fout,P;
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
        //аварии
        for (int i=0;i<alpar;i++)
        {
            AA[i].trig = paramToInt(Parameters[cpar+alpar+i]);
            AA[i].opt = paramToInt(Parameters[cpar+i]);
        }
        // параметры
        if (paramToInt(Parameters[0])==0)
        {
            actv[0].val[0] = paramToDouble(Parameters[1]);
        }
        else
        {
            actv[0].val[0] = Signals[4]->Value.doubleVal;
        }
        actv[0].val[0] = actv[0].val[0]/100;
        for (int i=0;i<av;i++)  //дискр
        {
            // копирование
            actv[i].val[1] = actv[i].val[0];
            // аварии
            if (AA[dvsit+dlsit+i].trig==1)
                actv[i].val[1] = averr(actv[i].val[1], AA[dvsit+dlsit+i].opt, &AA[dvsit+dlsit+i]);
            // ограничение
            if (actv[i].val[1]>actv[i].bound[1]) actv[i].val[1] = actv[i].bound[1];
                else if(actv[i].val[1]<actv[i].bound[0]) actv[i].val[1]=actv[i].bound[0];
        }
        ppow = actv[0].val[1];
    // Основной расчет
        dfp = (kpow*ppow-fp)/tiner;
        fp = fp + dfp*h;
        if(fp<=fin) fout = fp; else fout = fin;
    //выходы
        //датчики
        //P = 1e5;
        datv[0].val[0] = fp/kpow*kp; //p
        datv[1].val[0] = fp/kpow*ki; //i
        datv[2].val[0] = fp/kpow*kf; //f
        for (int i=0;i<dv;i++)
        {
            // копирование
            datv[i].val[1] = datv[i].val[0];
            // аварии
            if (AA[i].trig==1)
                datv[i].val[1] = dverr(datv[i].val[1], AA[i].opt, h, &AA[i]);
            // ограничение
            if (datv[i].val[1]>datv[i].bound[1]) datv[i].val[1] = datv[i].bound[1];
                else if(datv[i].val[1]<datv[i].bound[0]) datv[i].val[1]=datv[i].bound[0];
        }
        Signals[0]->Value.doubleVal = datv[0].val[1];
        Signals[1]->Value.doubleVal = datv[1].val[1];
        Signals[2]->Value.doubleVal = datv[2].val[1];
        //инд
        datl[0].val[0] = 0;
        if (fout>cap) datl[0].val[0] = 1;
        for (int i=0;i<dl;i++)
        {
            // копирование
            datl[i].val[1] = datl[i].val[0];
            // аварии
            if (AA[dvsit+i].trig==1)
                datl[i].val[1] = dlerr(datv[i].val[1], AA[dvsit+i].opt);
        }
        Signals[3]->Value.boolVal = datl[0].val[1];
        //выходы
        outinfosen->setNewOut(0,datv[0].val[1]);
        outinfosen->setNewOut(1,datv[1].val[1]);
        outinfosen->setNewOut(2,datv[2].val[1]);
        outinfosen->setNewOut(3,datl[0].val[1]);
        // запрос
        outp->setNewOut(0,fp);
        // выход
        outs->setNewOut(0,fout);
        outs->setNewOut(1,tin-273);
        outs->setNewOut(2,CCin[1]/kcd);
        outs->setNewOut(3,CCin[2]/kcd);
        outs->setNewOut(4,CCin[3]/kcd);
        outs->setNewOut(5,CCin[4]/kcd);
        // информационный
        outinfo->setNewOut(0,fout);
        outinfo->setNewOut(1,tin-273);
        outinfo->setNewOut(2,CCin[1]/kcd);
        outinfo->setNewOut(3,CCin[2]/kcd);
        outinfo->setNewOut(4,CCin[3]/kcd);
        outinfo->setNewOut(5,CCin[4]/kcd);
        outinfo->setNewOut(6,fp/kpow*100);
        //Отказы
        if(eflg!=0)
        {
            error = "Ошибка блока 12)АД-3750/2(нас.):\n";
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
    return new TR_VAPE_12P_AD37502();
}

void Release(ICalcElement *block)
{
    if(block->type() == "TR_VAPE_12P_AD37502") delete block;
}

std::string Type()
{
    return "TR_VAPE_12P_AD37502";
}
