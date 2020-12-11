#include "TR_VAPE_MIXER.h"

TR_VAPE_MIXER::TR_VAPE_MIXER()
{
	// Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
    // Управление
    createParameter("ctrlMode", "0");
    createParameter("ctrlDir", "0");
    // Настройка поломок
    createParameter("numInputs", "0");
    createParameter("numOutputs", "0");
    createParameter("mixMode", "0");

    // сигналы
    signDirCtrl = createSignal("signDirCtrl", Signal::ST_DOUBLE);
    signDirView = createSignal("signDirView", Signal::ST_DOUBLE);

    // ситуации

    // порты
    inSol = createInputPort(0, "inSol", "INFO");
    inPump = createInputPort(1, "inPump", "INFO");
    outSol = createOutputPort(2, "outSol", "INFO");
    outPump = createOutputPort(3, "outPump", "INFO");
    outInfo = createOutputPort(4, "outInfo", "INFO");
}

void TR_VAPE_MIXER::setDataNames()
{
    std::vector<std::string> out1;
    for (int i = 0; i < paramToInt("numOutputs"); i++) {
        out1.push_back("Расход раствора, кг/с");
        out1.push_back("Концентрация U, г/л");
        out1.push_back("Концентрация HNO3, г/л");
        out1.push_back("Температура, оС");
    }
    outSol->setDataNames(out1);

    std::vector<std::string> out3;
    for (int i = 0; i < paramToInt("numOutputs"); i++) {
        out3.push_back("Расход раствора, кг/с");
    }
    outPump->setDataNames(out3);

    std::vector<std::string> out2;
    out2.push_back("Уровень, м3");
    out2.push_back("Расход, кг/л");
    out2.push_back("Температура, оС");
    out2.push_back("Давление, кПа");
    out2.push_back("Индикатор П");
    out2.push_back("Индикатор ВУ");
    outInfo->setDataNames(out2);
}

bool TR_VAPE_MIXER::init(std::string &error, double h)
{
    setDataNames();
    return true;
}

bool TR_VAPE_MIXER::process(double t, double h, std::string &error)
{

    std::vector<flow> flowIn;
    std::vector<flow> flowOut;
    std::vector<double> pumpReq;
    flow flowSum;
    double reqSum = 0;

    flowSum.mass = 0;
    flowSum.conc[0] = 0;
    flowSum.conc[1] = 0;
    flowSum.conc[2] = 0;
    flowSum.temp = 0;


    // just mix
    for (int i = 0; i < paramToInt("numInputs"); i++) {
        // get in
        flowIn.push_back(flow());
        flowIn[i].mass = inSol->getInput()[4*i] * h;
        flowIn[i].conc[1] = inSol->getInput()[4*i+1] * COEFF_MASS;
        flowIn[i].conc[2] = inSol->getInput()[4*i+2] * COEFF_MASS;
        flowIn[i].conc[0] = 1 - flowIn[i].conc[1] - flowIn[i].conc[2];
        flowIn[i].temp = inSol->getInput()[4*i+3] + 273;
    }
    for (int i = 0; i < paramToInt("numInputs"); i++) {
        // mix
        flowSum.mass += flowIn[i].mass;
        flowSum.conc[0] += flowIn[i].mass * flowIn[i].conc[0];
        flowSum.conc[1] += flowIn[i].mass * flowIn[i].conc[1];
        flowSum.conc[2] += flowIn[i].mass * flowIn[i].conc[2];
        flowSum.temp += flowIn[i].mass * flowIn[i].temp;
    }

    flowSum.conc[0] /= flowSum.mass;
    flowSum.conc[1] /= flowSum.mass;
    flowSum.conc[2] /= flowSum.mass;
    flowSum.temp /= flowSum.mass;
    // flow out calc
    if (paramToInt("mixMode") == 0) {
        for (int i = 0; i < paramToInt("numOutputs"); i++) {
            flowOut.push_back(flow());
            flowOut[i].mass = flowSum.mass /paramToInt("numOutputs") /h;
            flowOut[i].conc[1] = flowSum.conc[1] / COEFF_MASS;
            flowOut[i].conc[2] = flowSum.conc[2] / COEFF_MASS;
            flowOut[i].temp = flowSum.temp - 273;
        }
    }
    if (paramToInt("mixMode") == 1) {
        for (int i = 0; i < paramToInt("numOutputs"); i++) {
            flowOut.push_back(flow());
            flowOut[paramToInt("ctrlDir")].mass = 0;
            flowOut[i].conc[1] = 0;
            flowOut[i].conc[2] = 0;
            flowOut[i].temp = 0;
        }
        flowOut[paramToInt("ctrlDir")].mass = flowSum.mass /h;
        flowOut[paramToInt("ctrlDir")].conc[1] = flowSum.conc[1] / COEFF_MASS;
        flowOut[paramToInt("ctrlDir")].conc[2] = flowSum.conc[2] / COEFF_MASS;
        flowOut[paramToInt("ctrlDir")].temp = flowSum.temp - 273;
    }
    if (paramToInt("mixMode") == 2) {
        for (int i = 0; i < paramToInt("numOutputs"); i++) {
            pumpReq.push_back(inPump->getInput()[i]);
            reqSum += inPump->getInput()[i];
        }
        outPump->setNewOut(0,reqSum);
        if (reqSum > 0) {
            for (int i = 0; i < paramToInt("numOutputs"); i++) {
                flowOut.push_back(flow());
                flowOut[i].mass = flowSum.mass /h * pumpReq[i] /reqSum;
                if (flowOut[i].mass > 0) {
                    flowOut[i].conc[1] = flowSum.conc[1] / COEFF_MASS;
                    flowOut[i].conc[2] = flowSum.conc[2] / COEFF_MASS;
                    flowOut[i].temp = flowSum.temp - 273;
                } else {
                    flowOut[i].conc[1] = 0;
                    flowOut[i].conc[2] = 0;
                    flowOut[i].temp = 0;
                }
            }
        } else {
            for (int i = 0; i < paramToInt("numOutputs"); i++) {
                flowOut.push_back(flow());
                flowOut[i].mass = 0;
                flowOut[i].conc[1] = 0;
                flowOut[i].conc[2] = 0;
                flowOut[i].temp = 0;
            }
        }
    }

    for (int i = 0; i < paramToInt("numOutputs"); i++) {
        outSol->setNewOut(4*i,flowOut[i].mass);
        outSol->setNewOut(4*i+1,flowOut[i].conc[1]);
        outSol->setNewOut(4*i+2,flowOut[i].conc[2]);
        outSol->setNewOut(4*i+3,flowOut[i].temp);
    }




    //int cou = 0;
    //outSol->setNewOut(cou,cou); cou++;


    /*
    // получение входов
    if ( paramToInt("propCtrlPumpOut") == 1 ) {
        flowOut[0].mass = paramToDouble("propPumpOutFlow") * h;
    } else {
        flowOut[0].mass = inPump->getInput()[0] * h;
    }
    if ( paramToInt("propCtrlFlowIn") == 1 ) {
        flowIn[0].mass = paramToDouble("flowInMass") * h;
        flowIn[0].conc[1] = paramToDouble("flowInConcU") * COEFF_MASS;
        flowIn[0].conc[2] = paramToDouble("flowInConcHno3") * COEFF_MASS;
        flowIn[0].conc[0] = 1 - flowIn[0].conc[1] - flowIn[0].conc[2];
        flowIn[0].temp = paramToDouble("flowInTemp") + 273;
    } else {
        flowIn[0].mass = inSol->getInput()[0] * h;
        flowIn[0].conc[1] = inSol->getInput()[1] * COEFF_MASS;
        flowIn[0].conc[2] = inSol->getInput()[2] * COEFF_MASS;
        flowIn[0].conc[0] = 1 - flowIn[0].conc[1] - flowIn[0].conc[2];
        flowIn[0].temp = inSol->getInput()[3] + 273;
    }

    // установка ИМ
    int valveCtrl,pumpCtrl;
    if (paramToInt("ctrlMode") == 0) {
        valveCtrl = paramToInt("ctrlValveInOpen") - paramToInt("ctrlValveInClose");
        pumpCtrl = paramToInt("ctrlPumpOutOn") - paramToInt("ctrlPumpOutOff");
    } else {
        valveCtrl = signValveInOpen->Value.intVal - signValveInClose->Value.intVal;
        pumpCtrl = signPumpOutOn->Value.intVal - signPumpOutOff->Value.intVal;
    }
    valvePos = valvePos + valveCtrl * paramToDouble("propValveInRate") * h;
    pumpPow = pumpPow + pumpCtrl * paramToDouble("propPumpOutRate") * h;
    //
    analAct[0].rawVal = valvePos;
    analAct[0].trig = isSituationActive("analActValveIn");
    analAct[0].opt = paramToInt("emerAnalActValveIn");
    analAct[1].rawVal = pumpPow;
    analAct[1].trig = isSituationActive("analActPumpOut");
    analAct[1].opt = paramToInt("emerAnalActPumpOut");
    //
    for (int i = 0; i < analAct.size(); i++) {
        analAct[i].rout(true);
    }
    valvePos = analAct[0].outVal;
    pumpPow = analAct[1].outVal;

    // действия
    if (paramToInt("propCtrlFlowIn") == 1) {
        flowIn[0].mass = flowIn[0].mass * valvePos;
    } else {
        flowIn[0].mass = flowIn[0].mass;
    }
    if (paramToInt("propCtrlPumpOut") == 1) {
        flowOut[0].mass = flowOut[0].mass * pumpPow;
    } else {
        flowOut[0].mass = flowOut[0].mass;
    }

    // расчеты
    tempMix();
    massCalc();

    // установка датчиков
    analSen[0].rawVal = mass /DENS;
    analSen[0].trig = isSituationActive("analSenLvl");
    analSen[0].opt = paramToInt("emerAnalSenLvl");
    analSen[1].rawVal = flowOut[0].mass /h;
    analSen[1].trig = isSituationActive("analSenFlow");
    analSen[1].opt = paramToInt("emerAnalSenFlow");
    analSen[2].rawVal = temp-273;
    analSen[2].trig = isSituationActive("analSenTemp");
    analSen[2].opt = paramToInt("emerAnalSenTemp");
    analSen[3].rawVal = 1e5;
    analSen[3].trig = isSituationActive("analSenPress");
    analSen[3].opt = paramToInt("emerAnalSenPress");
    for (int i = 0; i < analSen.size(); i++) {
        analSen[i].rout(false);
    }
    digSen[0].rawVal = false;
    if (mass /DENS >= paramToDouble("propVol") * paramToDouble("propOverLvl")) {
        digSen[0].rawVal = true;
    }
    digSen[0].trig = isSituationActive("digSenOverLvl");
    digSen[0].opt = paramToInt("emerDigSenOverLvl");
    digSen[1].rawVal = false;
    if (mass /DENS >= paramToDouble("propVol") * paramToDouble("propTopLvl")) {
        digSen[1].rawVal = true;
    }
    digSen[1].trig = isSituationActive("digSenTopLvl");
    digSen[1].opt = paramToInt("emerDigSenTopLvl");
    for (int i = 0; i < digSen.size(); i++) {
        digSen[i].rout();
    }

    // установка выходов
    int couout = 0;
    outSol->setNewOut(couout,flowOut[0].mass /h); couout++;
    outSol->setNewOut(couout,flowOut[0].conc[1] / COEFF_MASS); couout++;
    outSol->setNewOut(couout,flowOut[0].conc[2] / COEFF_MASS); couout++;
    outSol->setNewOut(couout,flowOut[0].temp - 273); couout++;
    couout = 0;
    outInfo->setNewOut(couout,analSen[couout].outVal); couout++;
    outInfo->setNewOut(couout,analSen[couout].outVal); couout++;
    outInfo->setNewOut(couout,analSen[couout].outVal); couout++;
    outInfo->setNewOut(couout,analSen[couout].outVal); couout++;
    outInfo->setNewOut(couout,digSen[0].outVal); couout++;
    outInfo->setNewOut(couout,digSen[1].outVal); couout++;
    */

    return true;
}

ICalcElement *Create()
{
    return new TR_VAPE_MIXER();
}

void Release(ICalcElement *block)
{
    if(block->type() == "TR_VAPE_MIXER") delete block;
}

std::string Type()
{
    return "TR_VAPE_MIXER";
}
