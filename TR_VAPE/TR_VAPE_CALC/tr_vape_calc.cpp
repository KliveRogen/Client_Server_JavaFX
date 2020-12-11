#include "tr_vape_calc.h"
#include <stdlib.h>

void Tank::tempMix() {
    double buf1,buf2;
    buf1 = mass;
    buf2 = temp * mass;
    for (int i = 0; i < flowIn.size(); i++) {
        buf1 = buf1 + flowIn[i].mass;
        buf2 = buf2 + flowIn[i].mass * flowIn[i].temp;
    }
    if (buf1 > 0) {
        temp = buf2 /buf1;
    } else {
        temp = 0;
    }
}

void Tank::massCalc() {
    // подвод
    for (int i = 0; i < flowIn.size(); i++) {
        mass = mass + flowIn[i].mass;
    }
    for (int i = 0; i < NUM_EL; i++) {
        for (int l = 0; l < flowIn.size(); l++) {
            if ((mass + flowIn[l].mass) > 0) {
                conc[i] = (conc[i] * mass + flowIn[l].conc[i] * flowIn[l].mass) / (mass + flowIn[l].mass);
            } else {
                conc[i] = 0;
            }
        }
    }
    // отвод
    for (int i = 0; i < flowOut.size(); i++) {
        if (mass - flowOut[i].mass > 0) {
            for (int l = 0; l < NUM_EL; l++) {
                flowOut[i].conc[l] = conc[l];
            }
            mass = mass - flowOut[i].mass;
            flowOut[i].temp = temp;
        } else {
            flowOut[i].mass = 0;
        }
    }
}

void Pump::pumping() {
    flowOut = flowIn;
}

void HeatEx::heatExchange() {
    //enerHeat = coeffHeatEx * (flowSolIn.temp - flowHeatIn.temp);

}

void Mixer::mixFlowIn() {

}

void Mixer::splitFlowOut() {

}

void Mixer::setFlowOut(int num) {

}
void digUnit::rout() {
    // аварии
    outVal = rawVal;
    if (trig) {
        switch (opt) {
        case 1:
            outVal = false;
            break;
        case 2:
            outVal = true;
            break;
        default:
            break;
        }
    }
}

void analUnit::rout(bool isAct) {
    inVal = (rawVal - coeffSum)/coeffMul; // перевод в стандартный формат (0-1)
    // аварии
    if (trig) {
        if (isAct) {
            // аварии ИМ
            switch (opt) {
            case 1:
                if (!lagFlag) {
                    inValBuf = inVal;
                    lagFlag = true;
                }
                inVal = inValBuf;
                break;
            default:
                break;
            }
            if (opt != 1) {
                lagFlag = false;
            }
        } else {
            // аварии Дат
            switch (opt) {
            case 1:
                inVal = inVal*1.1;
                break;
            case 2:
                inVal = inVal*0.9;
                break;
            case 3:
                inVal = inVal*(1+0.05*(rand()%101-50)/50);
                break;
            case 4: // завис
                if (!lagFlag) {
                    inValBuf = inVal;
                    lagFlag = true;
                }
                inVal = inValBuf;
                break;
            case 5:
                if ( rand()%10 == 0 ) {
                    inVal = inVal*(1+0.3*((rand()%101)-50)/50);
                }
                break;
            case 6:
                inVal = 1;
                break;
            case 7:
                inVal = 0;
                break;
            case 8:
                inVal = inVal*inVal;
                break;
            default:
                break;
            }
            if (opt != 4) {
                lagFlag = false;
            }
        }
    }
    // ограничения
    if (inVal > 1) {
        inVal = 1;
    }
    if (inVal < 0){
        inVal = 0;
    }
    outVal = inVal * coeffMul + coeffSum; // перевод в нормальный формат
}






void actOrSenRoutine(actOrSen *unit) {
    if ( unit->emerTrig ) {
        if ( unit->senOrAct ) { //аварии ИМ
            if ( unit->digOrAnal ) { //аналог
                switch ( unit->emerOpt ) {
                case 1: //работает в крайних точках
                    if (unit->analVal[0] >= 0.5) {
                        unit->analVal[0] = 1;
                    } else {
                        unit->analVal[0] = 0;
                    }
                    break;
                case 2: //не работает
                    if ( unit->emerFlag ) {
                        unit->analVal[0] = unit->emerBuf[0];
                    } else {
                        unit->emerFlag = true;
                        unit->emerBuf[0] = unit->analVal[0];
                    }
                    break;
                case 3: //работает не с первого раза
                    if (unit->emerBuf[0] != unit->analVal[0]) {
                        unit->emerCou++;
                        unit->emerBuf[0] = unit->analVal[0];
                    }
                    if (unit->emerCou > 1) {
                        unit->emerCou = 0;
                        unit->emerBuf[1] = unit->analVal[0];
                    }
                    unit->analVal[0] = unit->emerBuf[1];
                    break;
                case 4: //заклинивает в крайних полож
                    if ( unit->emerFlag ) {
                        unit->analVal[0] = unit->emerBuf[0];
                    } else {
                        if (unit->analVal[0] > 0.9 || unit->analVal[0] < 0.1) {
                            unit->emerFlag = true;
                            unit->emerBuf[0] = unit->analVal[0];
                        }
                    }
                    break;
                case 5: //пропуск
                    unit->analVal[0] = unit->analVal[0] + 0.01;
                    break;
                default:
                    break;
                }
                if (unit->emerOpt != 4 && unit->emerOpt != 2) {
                    unit->emerFlag = false;
                }
            } else { //цифровой
                switch (unit->emerOpt) {
                case 1: //не работает
                    if ( unit->emerFlag ) {
                        unit->digVal[0] = unit->emerDigBuf;
                    } else {
                        unit->emerFlag = true;
                        unit->emerDigBuf = unit->digVal[0];
                    }
                    break;
                default:
                    break;
                }
            }
        } else { //аварии
            if ( unit->digOrAnal ) { //аналог
                switch (unit->emerOpt) {
                case 1: //завышение
                    unit->analVal[0] = unit->analVal[0]*1.1;
                    break;
                case 2: //занижение
                    unit->analVal[0] = unit->analVal[0]*0.9;
                    break;
                case 3: //нестабильность
                    unit->analVal[0] = unit->analVal[0]*(1+0.05*(rand()%101-50)/50);
                    break;
                case 4: //зависание
                    if ( unit->emerFlag == false ) {
                        unit->emerFlag = true;
                        unit->emerBuf[0] = unit->analVal[0];
                    }
                    unit->analVal[0] = unit->emerBuf[0];
                    break;
                case 5: //пики
                    if ( rand()%10 == 0 ) {
                        unit->analVal[0] = unit->analVal[0]*(1+0.3*((rand()%101)-50)/50);
                    }
                    break;
                case 6: //верхнее значение
                    unit->analVal[0] = 1;
                    break;
                case 7: //нижнее значение
                    unit->analVal[0] = 0;
                    break;
                case 8: //ошибка
                    unit->analVal[0] = unit->analVal[0]/2;
                    break;
                case 9: //задержка
                    if ( unit->emerCou > 10 ) {
                        unit->emerCou = 0;
                        unit->emerBuf[0] = unit->analVal[0];
                    } else {
                        unit->analVal[0] = unit->emerBuf[0];
                        unit->emerCou++;
                    }
                    break;
                default:
                    break;
                }
                if ( unit->emerOpt != 4 ) {
                    unit->emerFlag = false;
                }
            } else { //цифровой
                switch (unit->emerOpt) {
                case 1: //ложно не горит
                    unit->digVal[0] = false;
                    break;
                case 2: //ложно горит
                    unit->digVal[0] = true;
                    break;
                default:
                    break;
                }
            }
        }
    }
    if (unit->analVal[0] > 1) {
        unit->analVal[0] = 1;
    }
    if (unit->analVal[0] < 0) {
        unit->analVal[0] = 0;
    }
    unit->analVal[1] = unit->coeffMul * unit->analVal[0] + unit->coeffSum;
}

std::string errorMsgCreator(char code, std::string name) {
    std::string msg;
    msg = "Ошибка блока " + name + "\n";
    if (code == 1) {
        msg = msg + "Ошибка в заполнении исходных данных!";
    }
    if (code == 2) {
        msg = msg + "Ошибка в заполнении управляющих сигналов!";
    }
    if (code == 3) {
        msg = msg + "Переполнение емкости!";
    }
    if (code == 4) {
        msg = msg + "Ошибка в полученых через порт данных!";
    }
    if (code == 5) {
    }
    return msg;
}




/*

double dverr(double vol, int nerr, double h, alarm *AA)
{
    //a4dve ans;
    double out = vol;
    switch(nerr)
    {
        case 1: out = vol*1.1;  //завыш
                break;
        case 2: out = vol*0.9;  //заниж
                break;
        case 3: out = vol*(1+0.1*((rand()%101)-50)/50); //ошибка
                break;

        case 4: if(AA->eflag!=1) //завис
                {
                    AA->eflag = 1;
                    AA->eval_i[0] = vol;
                }
                out = AA->eval_i[0];
                break;
        case 5: if (rand()%10==0) //пики
                {
                    out = vol*(1+0.3*((rand()%101)-50)/50);
                }
                break;
        case 6: out = 1e100; //верх границ
                break;
        case 7: out = -1e100; //ниж границ
                break;
        case 8: out = vol*vol; //ошибка
                break;
        case 9: if (AA->ecou>10*h) //задержка
                {
                    AA->ecou = 0;
                    AA->eval_i[0] = vol;
                    out = vol;
                }
                else
                {
                    out = AA->eval_i[0];
                    AA->ecou = AA->ecou + h;
                }
                break;
        default:
                break;
    }
    if (nerr!=4)
        AA->eflag = 0;
    return out;
}
*/
/*
int dlerr(int vol, int nerr)
{
    int out = vol;
    switch(nerr)
    {
        case 1: out = 0;  //не гор
                break;
        case 2: out = 1;  //ложно горит
                break;
        default:
                break;
    }
    return out;
}
*/
/*
double averr(double vol, int nerr, alarm *AA)
{
    double out = vol;
    switch(nerr)
    {
        case 1: if (vol>=0.5) out = 1;  else out = 0;  //край
                break;
        case 2: if (AA->eflag == 0) //2 - не раб
                {
                    AA->eflag = 1;
                    AA->eval_i[0] = vol;
                }
                else out = AA->eval_i[0];
                break;
        case 3: if (AA->eval_i[0] != vol) //раб не с первого раза
                {
                    AA->ecou++;
                    AA->eval_i[0] = vol;
                }
                if (AA->ecou > 1)
                {
                    AA->ecou = 0;
                    AA->eval_i[1] = vol;
                }
                out = AA->eval_i[1];
                break;
        case 4: if ((vol > 0.9 || vol < 0.1) && AA->eflag==0)//заклин в крайних полож
                {
                    AA->eflag = 1;
                    AA->eval_i[0] = vol;
                }
                if (AA->eflag == 1)
                {
                    out = AA->eval_i[0];
                }
                break;
        case 5: out = vol+0.01; //пропуск вентиля
                break;
        default:
                break;
    }
    if (nerr != 4 && nerr != 2) AA->eflag = 0;
    return out;
}
*/
/*
int alerr(int vol, int nerr, alarm *AA)
{
    double out = vol;
    switch(nerr)
    {
        case 1: if (AA->eflag == 0) //1 - не раб
                {
                    AA->eflag = 1;
                    AA->eval_i[0] = vol;
                }
                else out = AA->eval_i[0];
                break;
        default:
                break;
    }
//    if (nerr!=1)
//        AA[nel].eflag = 0;
    return out;
}
*/
