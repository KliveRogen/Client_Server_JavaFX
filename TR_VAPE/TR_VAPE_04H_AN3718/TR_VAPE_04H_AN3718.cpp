#include "TR_VAPE_04H_AN3718.h"

TR_VAPE_04H_AN3718::TR_VAPE_04H_AN3718()
{
	// Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
    createParameter("iv_t", "100");         //0
    createParameter("iv_f", "5");           //1
    createParameter("as_dv_f", "0");        //2
    createParameter("al_dv_f", "0");        //3

	// Сигналы блока
	createSignal("sm_f", Signal::ST_DOUBLE);

	// Порты блока
    ins = createInputPort(0, "ins", "INFO");
    outs = createOutputPort(1, "outs", "INFO");
    outinfo = createOutputPort(2, "outinfo", "INFO");
    outinfosen = createOutputPort(3, "outinfosen", "INFO");

	// Отказы блока

}

void TR_VAPE_04H_AN3718::setDataNames()
{
    std::vector<std::string> out1,out2,out3;
    out1.push_back("Расход раствора, кг/с");
    out1.push_back("Температура раствора, оС");
    out1.push_back("Концентрация U, г/л");
    out1.push_back("Концентрация HNO3, г/л");
    out1.push_back("Концентрация ТБФ, г/л");
    out1.push_back("Концентрация CnHn, г/л");
    outs->setDataNames(out1);
    out2.push_back("Расход раствора, кг/с");
    out2.push_back("Температура раствора на входе, оС");
    out2.push_back("Температура раствора на выходе, оС");
    out2.push_back("Расход подогревателя, кг/с");
    out2.push_back("Температура подогревателя на входе, оС");
    out2.push_back("Температура подогревателя на выходе, оС");
    outinfo->setDataNames(out2);
    out3.push_back("Расход раствора, м3/ч");
    outinfosen->setDataNames(out3);
}

bool TR_VAPE_04H_AN3718::init(std::string &error, double h)
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
        datv[cou].bound[1] = 15;
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
        fhin = paramToDouble(Parameters[0]);
        thin = paramToDouble(Parameters[1])+273;
	setDataNames();
    return true;
}

bool TR_VAPE_04H_AN3718::process(double t, double h, std::string &error)
{
    //инициализации
        //перем
        double  MMout[el],MMin1[el],MMin2[el],
                MMsum,fin1,fin2,CCin1[el],CCin2[el],
                tin1,tin2,buf,qheat,fin,tin,thout,tout;
        for (int i = 0; i < el; i++) {
            MMout[i] = 0;
            MMin1[i] = 0;
            MMin2[i] = 0;
            CCin1[i] = 0;
            CCin2[i] = 0;
        }
        //входы
        fin1 = ins->getInput()[0];
        tin1 = ins->getInput()[1]+273;
        buf = 0;
        for(int i=0;i<el-1;i++)
        {
            CCin1[1+i] = ins->getInput()[2+i]*kcd;
            buf = buf + CCin1[1+i];
        }
        CCin1[0] = 1 - buf;
        fin2 = ins->getInput()[0+el+1];
        tin2 = ins->getInput()[1+el+1]+273;
        buf = 0;
        for(int i=0;i<el-1;i++)
        {
            CCin2[1+i] = ins->getInput()[2+i+el+1]*kcd;
            buf = buf + CCin2[1+i];
        }
        CCin2[0] = 1 - buf;
        // параметры
        fhin = paramToDouble(Parameters[1]);
        thin = paramToDouble(Parameters[0])+273;
        if (fhin < 0) eflg = 1;
        //аварии
        for (int i=0;i<alpar;i++)
        {
            AA[i].trig = paramToInt(Parameters[cpar+ivpar+alpar+i]);
            AA[i].opt = paramToInt(Parameters[cpar+ivpar+i]);
        }
    // доп расчет
        fin = fin1 + fin2;
    //основной расчет
        //температура
        tin = 0 + 273;
        if (fin > cap)
            tin = (tin1 * fin1 + tin2 * fin2) / (fin1 + fin2);
        //масса
        MMsum = 0;
        for (int i = 0; i < el; i++)
        {
            MMin1[i] = CCin1[i] * fin1;
            MMin2[i] = CCin2[i] * fin2;
            MMout[i] = (MMin1[i] + MMin2[i]);
            MMsum = MMsum + MMout[i];
        }
        //изменение температуры
        qheat = ksheat * (thin - tin);
        thout = thin;
        if (fhin > cap) {
            thout = thin - qheat/(hcap*fhin);
        }
        tout = tin;
        if (fin > cap) {
            tout = tin + qheat/(hcap*fin);
        }
        if (thin/thout>1 && tout>thout) {
            tout = thout;
        }
        if (thin/thout<1 && tout<thout) {
            tout = thout;
        }
    //выходы
        //датчики
        datv[0].val[0] = fin*3600/dens;
        for (int i=0;i<dv;i++)
        {
            // копирование
            datv[i].val[1] = datv[i].val[0];
            // аварии
            if (AA[i].trig==1){}
                //datv[i].val[1] = dverr(datv[i].val[1], AA[i].opt, h, &AA[i]);
            // ограничение
            if (datv[i].val[1] > datv[i].bound[1]) {
                datv[i].val[1] = datv[i].bound[1];
            }
            if (datv[i].val[1] < datv[i].bound[0]) {
                datv[i].val[1]=datv[i].bound[0];
            }
        }
        Signals[0]->Value.doubleVal = datv[0].val[1];
        //выходы
        // выход
        buf = 0;
        for(int i = 0; i < el; i++)
            buf = buf + MMout[i];
        if (buf > cap)
        {
            outs->setNewOut(0,buf);
            outs->setNewOut(1,tout-273);
            outs->setNewOut(2,MMout[1]/buf/kcd);
            outs->setNewOut(3,MMout[2]/buf/kcd);
            outs->setNewOut(4,MMout[3]/buf/kcd);
            outs->setNewOut(5,MMout[4]/buf/kcd);
        }
        else
            for (int i = 0; i < (el+1); i++)
                outinfo->setNewOut(i,0);
        // информационный
        outinfo->setNewOut(0,buf);
        outinfo->setNewOut(1,tin - 273);
        outinfo->setNewOut(2,tout - 273);
        outinfo->setNewOut(3,fhin);
        outinfo->setNewOut(4,thin - 273);
        outinfo->setNewOut(5,thout - 273);
        // вывод датчики
        outinfosen->setNewOut(0,datv[0].val[1]);
        //Отказы
        if(eflg!=0)
        {
            error = "Ошибка блока 4)АН-3718(тепл.):\n";

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
    return new TR_VAPE_04H_AN3718();
}

void Release(ICalcElement *block)
{
    if(block->type() == "TR_VAPE_04H_AN3718") delete block;
}

std::string Type()
{
    return "TR_VAPE_04H_AN3718";
}
