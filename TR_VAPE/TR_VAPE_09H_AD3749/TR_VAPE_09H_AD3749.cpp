#include "TR_VAPE_09H_AD3749.h"

TR_VAPE_09H_AD3749::TR_VAPE_09H_AD3749()
{
	// Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
	createParameter("as_dv_t", "0");
	createParameter("al_dv_t", "0");

	// Сигналы блока
	createSignal("sm_t", Signal::ST_DOUBLE);

	// Порты блока
    ins = createInputPort(0, "ins", "INFO");
    inh = createInputPort(1, "inh", "INFO");
    outs = createOutputPort(2, "outs", "INFO");
    outinfo = createOutputPort(3, "outinfo", "INFO");
    outinfosen = createOutputPort(4, "outinfosen", "INFO");

	// Отказы блока

}

void TR_VAPE_09H_AD3749::setDataNames()
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
    out3.push_back("Температура, оС");
    outinfosen->setDataNames(out3);
}

bool TR_VAPE_09H_AD3749::init(std::string &error, double h)
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
        // 0 - датчик T
        int cou=0;
        datv[cou].bound[0] = 0;
        datv[cou].bound[1] = 100;
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
	setDataNames();
    return true;
}

bool TR_VAPE_09H_AD3749::process(double t, double h, std::string &error)
{
    //инициализации
        //перем
        double  MMout[el],CCin[el],
                buf,qheat,fin,tin,thout,tout;
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
        // параметры
        fhin = inh->getInput()[0];
        thin = inh->getInput()[1]+273;
        if (fhin < 0) eflg = 1;
        //аварии
        for (int i=0;i<alpar;i++)
        {
            AA[i].trig = paramToInt(Parameters[cpar+ivpar+alpar+i]);
            AA[i].opt = paramToInt(Parameters[cpar+ivpar+i]);
        }
    //основной расчет
        //изменение температуры
        qheat = 0;
        if (fin > cap && fhin > cap)
            qheat = ksheat * (thin - tin);
        tout = tin;
        if (fin > cap)
            tout = tin + qheat/(hcap * fin);
        thout = thin;
        if (fhin > cap)
            thout = thin - qheat/(hcap * fhin);
        if (thin/thout>1 && tout>thout)
            tout = thout;
        if (thin/thout<1 && tout<thout)
            tout = thout;

    //выходы
        //датчики
        datv[0].val[0] = tout-273;
        for (int i=0;i<dv;i++)
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
        //выходы
        // вывод датчкик
        outinfosen->setNewOut(0,datv[0].val[1]);
        // выход
        outs->setNewOut(0,fin);
        outs->setNewOut(1,tout-273);
        outs->setNewOut(2,CCin[1]/kcd);
        outs->setNewOut(3,CCin[2]/kcd);
        outs->setNewOut(4,CCin[3]/kcd);
        outs->setNewOut(5,CCin[4]/kcd);
        // информационный
        outinfo->setNewOut(0,fin);
        outinfo->setNewOut(1,tin - 273);
        outinfo->setNewOut(2,tout - 273);
        outinfo->setNewOut(3,fhin);
        outinfo->setNewOut(4,thin - 273);
        outinfo->setNewOut(5,thout - 273);
        //Отказы
        if(eflg!=0)
        {
            error = "Ошибка блока 9)АД-3749(тепл.):\n";
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
    return new TR_VAPE_09H_AD3749();
}

void Release(ICalcElement *block)
{
    if(block->type() == "TR_VAPE_09H_AD3749") delete block;
}

std::string Type()
{
    return "TR_VAPE_09H_AD3749";
}
