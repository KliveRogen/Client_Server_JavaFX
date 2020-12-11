#include "TR_VAPE_08T_AN3702.h"

TR_VAPE_08T_AN3702::TR_VAPE_08T_AN3702()
{
	// Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
    createParameter("spcont", "0");
    createParameter("pc_num", "1");

    createParameter("as_dv_l1", "0");
    createParameter("as_dv_q1", "0");
    createParameter("as_dv_l2", "0");
    createParameter("as_dv_q2", "0");
    createParameter("as_dv_l3", "0");
    createParameter("as_dv_q3", "0");
    createParameter("as_dv_l4", "0");
    createParameter("as_dv_q4", "0");
    createParameter("as_dv_l5", "0");
    createParameter("as_dv_q5", "0");
    createParameter("as_dv_l6", "0");
    createParameter("as_dv_q6", "0");
    createParameter("as_dv_l7", "0");
    createParameter("as_dv_q7", "0");
    createParameter("as_dv_l8", "0");
    createParameter("as_dv_q8", "0");

    createParameter("al_dv_l1", "0");
    createParameter("al_dv_q1", "0");
    createParameter("al_dv_l2", "0");
    createParameter("al_dv_q2", "0");
    createParameter("al_dv_l3", "0");
    createParameter("al_dv_q3", "0");
    createParameter("al_dv_l4", "0");
    createParameter("al_dv_q4", "0");
    createParameter("al_dv_l5", "0");
    createParameter("al_dv_q5", "0");
    createParameter("al_dv_l6", "0");
    createParameter("al_dv_q6", "0");
    createParameter("al_dv_l7", "0");
    createParameter("al_dv_q7", "0");
    createParameter("al_dv_l8", "0");
    createParameter("al_dv_q8", "0");

	// Сигналы блока
    createSignal("sm_l", Signal::ST_DOUBLE);
    createSignal("sm_q", Signal::ST_DOUBLE);
    createSignal("sc_num", Signal::ST_DOUBLE);
    createSignal("sm_l1", Signal::ST_DOUBLE);
    createSignal("sm_q1", Signal::ST_DOUBLE);
    createSignal("sm_l2", Signal::ST_DOUBLE);
    createSignal("sm_q2", Signal::ST_DOUBLE);
    createSignal("sm_l3", Signal::ST_DOUBLE);
    createSignal("sm_q3", Signal::ST_DOUBLE);
    createSignal("sm_l4", Signal::ST_DOUBLE);
    createSignal("sm_q4", Signal::ST_DOUBLE);
    createSignal("sm_l5", Signal::ST_DOUBLE);
    createSignal("sm_q5", Signal::ST_DOUBLE);
    createSignal("sm_l6", Signal::ST_DOUBLE);
    createSignal("sm_q6", Signal::ST_DOUBLE);
    createSignal("sm_l7", Signal::ST_DOUBLE);
    createSignal("sm_q7", Signal::ST_DOUBLE);
    createSignal("sm_l8", Signal::ST_DOUBLE);
    createSignal("sm_q8", Signal::ST_DOUBLE);


	// Порты блока
    ins = createInputPort(0, "UNKNOWN_NAME", "INFO");
    outinfo = createOutputPort(1, "UNKNOWN_NAME", "INFO");
    outinfosen = createOutputPort(2, "UNKNOWN_NAME", "INFO");


	// Отказы блока

}

void TR_VAPE_08T_AN3702::setDataNames()
{
    std::vector<std::string> out,out1;
    out.push_back("Количество раствора в емкости, кг");
    out.push_back("Температура раствора, оС");
    out.push_back("Концентрация U, г/л");
    out.push_back("Концентрация HNO3, г/л");
    out.push_back("Концентрация ТБФ, г/л");
    out.push_back("Концентрация CnHn, г/л");
    out.push_back("Выбранная емкость");
    outinfo->setDataNames(out);
    out1.push_back("Количество раствора в емкости 1, м3");
    out1.push_back("Количество раствора в емкости 2, м3");
    out1.push_back("Количество раствора в емкости 3, м3");
    out1.push_back("Количество раствора в емкости 4, м3");
    out1.push_back("Количество раствора в емкости 5, м3");
    out1.push_back("Количество раствора в емкости 6, м3");
    out1.push_back("Количество раствора в емкости 7, м3");
    out1.push_back("Количество раствора в емкости 8, м3");
    out1.push_back("Расход в емкость 1, м3/ч");
    out1.push_back("Расход в емкость 2, м3/ч");
    out1.push_back("Расход в емкость 3, м3/ч");
    out1.push_back("Расход в емкость 4, м3/ч");
    out1.push_back("Расход в емкость 5, м3/ч");
    out1.push_back("Расход в емкость 6, м3/ч");
    out1.push_back("Расход в емкость 7, м3/ч");
    out1.push_back("Расход в емкость 8, м3/ч");
    outinfosen->setDataNames(out1);
}

bool TR_VAPE_08T_AN3702::init(std::string &error, double h)
{
    //инициализации
        for(int l=0; l<tnum; l++)
        {
            for(int i=0;i<el;i++)
                MM[i][l] = 0;
            TT[l] = 0;
        }
        // датчики
        //for (int i=0;i<tnum*2;i++) std::fill(datv[i].val,datv[i].val+2,0);
        for (int i = 0; i < tnum; i++) {
            datv[i].val[0] = 0;
            datv[i].val[1] = 0;
            datv[i+tnum].val[0] = 0;
            datv[i+tnum].val[1] = 0;
            datv[i].bound[0] = 0;
            datv[i].bound[1] = 290;
            datv[i+tnum].bound[0] = 0;
            datv[i+tnum].bound[1] = 15;
        }
        //аварии
        for (int i=0;i<tnum*2;i++)
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

bool TR_VAPE_08T_AN3702::process(double t, double h, std::string &error)
{
    //инициализации
        //перем
        int     notk;
        double  dMM[el],MMin[el],
                MMsum[tnum],fin,CCin[el],tin,buf,
                ll[tnum],lvlchk[4];
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
        AA[0].trig = paramToInt("al_dv_l1");
        AA[0].opt = paramToInt("as_dv_l1");
        AA[1].trig = paramToInt("al_dv_l2");
        AA[1].opt = paramToInt("as_dv_l2");
        AA[2].trig = paramToInt("al_dv_l3");
        AA[2].opt = paramToInt("as_dv_l3");
        AA[3].trig = paramToInt("al_dv_l4");
        AA[3].opt = paramToInt("as_dv_l4");
        AA[4].trig = paramToInt("al_dv_l5");
        AA[4].opt = paramToInt("as_dv_l5");
        AA[5].trig = paramToInt("al_dv_l6");
        AA[5].opt = paramToInt("as_dv_l6");
        AA[6].trig = paramToInt("al_dv_l7");
        AA[6].opt = paramToInt("as_dv_l7");
        AA[7].trig = paramToInt("al_dv_l8");
        AA[7].opt = paramToInt("as_dv_l8");
        AA[8].trig = paramToInt("al_dv_q1");
        AA[8].opt = paramToInt("as_dv_q1");
        AA[9].trig = paramToInt("al_dv_q2");
        AA[9].opt = paramToInt("as_dv_q2");
        AA[10].trig = paramToInt("al_dv_q3");
        AA[10].opt = paramToInt("as_dv_q3");
        AA[11].trig = paramToInt("al_dv_q4");
        AA[11].opt = paramToInt("as_dv_q4");
        AA[12].trig = paramToInt("al_dv_q5");
        AA[12].opt = paramToInt("as_dv_q5");
        AA[13].trig = paramToInt("al_dv_q6");
        AA[13].opt = paramToInt("as_dv_q6");
        AA[14].trig = paramToInt("al_dv_q7");
        AA[14].opt = paramToInt("as_dv_q7");
        AA[15].trig = paramToInt("al_dv_q8");
        AA[15].opt = paramToInt("as_dv_q8");

        // параметры
        if (paramToInt(Parameters[0])==0)
        {
            actl[0].val[0] = paramToInt(Parameters[1]);
        }
        else
        {
            actl[0].val[0] = Signals[2]->Value.doubleVal;
        }
        for (int i=0;i<al;i++)  //дискр
        {
            // копирование
            actl[i].val[1] = actl[i].val[0];
        }
        notk = actl[0].val[1]-1;
        if ((notk > 7) || (notk < 0)) eflg = 3;
    //вспомогательный расчет
        //масса
        for(int l = 0; l < tnum; l++)
        {
            MMsum[l] = 0;
            for(int i=0;i<el;i++)
                MMsum[l] = MMsum[l] + MM[i][l];
        }
    //основной расчет
        //температура
        for (int i = 0; i < tnum; i++)
            if (MMsum[i] < cap)
                TT[i] = 20 + 273;
            else
                TT[i] = (TT[i]*MMsum[i]+tin*fin*h)/(MMsum[i]+fin*h);
        //масса
        for(int i=0;i<el;i++)
        {
            MMin[i] = CCin[i]*fin;
            dMM[i] = MMin[i];
            MM[i][notk] = MM[i][notk] + dMM[i]*h;
            if (MM[i][notk] < cap) MM[i][notk] = 0;
        }
    //выходы
        //датчики
        for (int i = 0; i < 4 ; i++)
            lvlchk[i] = lvl*lvltst[i];
        for (int l = 0; l < tnum; l++)
        {
            ll[l] = MMsum[l]/dens;
            if ( ll[l]/sef > lvlchk[2] ) eflg = 2;
        }
        for (int i = 0; i < tnum; i++) {
            datv[i].val[0] = ll[i];
            datv[i+tnum].val[0] = 0;
        }
        datv[notk+tnum].val[0] = fin*3600/dens;
        for (int i = 0; i < tnum*2; i++) {
            //копирование
            datv[i].val[1] = datv[i].val[0];
            //аварии
            if (AA[i].trig==1) {}
                //datv[i].val[1] = dverr(datv[i].val[1], AA[i].opt, h, &AA[i]);

            //ограничение
            if ( datv[i].val[1] > datv[i].bound[1] ) {
                datv[i].val[1] = datv[i].bound[1];
            }
            if( datv[i].val[1] < datv[i].bound[0] ) {
                datv[i].val[1] = datv[i].bound[0];
            }
        }
        Signals[0]->Value.doubleVal = datv[notk].val[1];
        Signals[1]->Value.doubleVal = datv[notk+tnum].val[1];
        for (int i = 0; i < tnum; i++) {
            Signals[i*2+3]->Value.doubleVal = datv[i].val[1];
            Signals[i+1+3]->Value.doubleVal = datv[i+tnum].val[1];;
        }
        //выходы
        // вывод датчики
        outinfosen->setNewOut(0,datv[0].val[1]);
        outinfosen->setNewOut(1,datv[1].val[1]);
        outinfosen->setNewOut(2,datv[2].val[1]);
        outinfosen->setNewOut(3,datv[3].val[1]);
        outinfosen->setNewOut(4,datv[4].val[1]);
        outinfosen->setNewOut(5,datv[5].val[1]);
        outinfosen->setNewOut(6,datv[6].val[1]);
        outinfosen->setNewOut(7,datv[7].val[1]);
        outinfosen->setNewOut(8,datv[8].val[1]);
        outinfosen->setNewOut(9,datv[9].val[1]);
        outinfosen->setNewOut(10,datv[10].val[1]);
        outinfosen->setNewOut(11,datv[11].val[1]);
        outinfosen->setNewOut(12,datv[12].val[1]);
        outinfosen->setNewOut(13,datv[13].val[1]);
        outinfosen->setNewOut(14,datv[14].val[1]);
        outinfosen->setNewOut(15,datv[15].val[1]);
        // информационный
        buf = 0;
        for(int i = 0; i < el; i++)
            buf = buf + MM[i][notk];
        if (buf > cap)
        {
            outinfo->setNewOut(0,buf);
            outinfo->setNewOut(1,TT[notk]-273);
            outinfo->setNewOut(2,MM[1][notk]/buf/kcd);
            outinfo->setNewOut(3,MM[2][notk]/buf/kcd);
            outinfo->setNewOut(4,MM[3][notk]/buf/kcd);
            outinfo->setNewOut(5,MM[4][notk]/buf/kcd);
            outinfo->setNewOut(6,notk+1);
        }
        else
            for (int i = 0; i < (el+1); i++)
                outinfo->setNewOut(i,0);
        //Отказы
        if(eflg!=0)
        {
            error = "Ошибка блока 08)АН-3702(емк.):\n";

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
    return new TR_VAPE_08T_AN3702();
}

void Release(ICalcElement *block)
{
    if(block->type() == "TR_VAPE_08T_AN3702") delete block;
}

std::string Type()
{
    return "TR_VAPE_08T_AN3702";
}
