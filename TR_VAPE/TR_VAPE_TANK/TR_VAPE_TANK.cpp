#include "TR_VAPE_TANK.h"

TR_VAPE_TANK::TR_VAPE_TANK()
{
	// Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
    // Управление
    createParameter("ctrlMode", "0");  //used
    createParameter("ctrlValveInOpen", "0");  //used
    createParameter("ctrlValveInClose", "0");  //used
    createParameter("ctrlPumpOutOn", "0");  //used
    createParameter("ctrlPumpOutOff", "0");  //used
    // Настройка поломок
    createParameter("emerAnalSenLvl", "0"); //used
    createParameter("emerAnalSenFlow", "0"); //used
    createParameter("emerAnalSenTemp", "0"); //used
    createParameter("emerAnalSenPress", "0"); //used
    createParameter("emerDigSenOverLvl", "0"); //used
    createParameter("emerDigSenTopLvl", "0"); //used
    createParameter("emerAnalActValveIn", "0"); //used
    createParameter("emerAnalActPumpOut", "0"); //used
    // Начальные условия
    createParameter("initMass", "0"); //used
    createParameter("initConcU", "0"); //used
    createParameter("initConcHno3", "0"); //used
    createParameter("initTemp", "0"); //used
    // Входной поток
    createParameter("flowInMass", "0"); //used
    createParameter("flowInConcU", "0"); //used
    createParameter("flowInConcHno3", "0"); //used
    createParameter("flowInTemp", "0"); //used
    // Конфигурация емкости
    createParameter("propVol", "0");
    createParameter("propHeight", "0");
    createParameter("propOverLvl", "0");
    createParameter("propTopLvl", "0");
    createParameter("propCtrlFlowIn", "0"); //used
    createParameter("propCtrlPumpOut", "0"); //used
    createParameter("propValveInRate", "0"); //used
    createParameter("propPumpOutRate", "0"); //used
    createParameter("propPumpOutFlow", "0"); //used
    createParameter("propSensToShow", "0");

    // сигналы
    signValveInOpen = createSignal("signValveInOpen", Signal::ST_DOUBLE);
    signValveInClose = createSignal("signValveInClose", Signal::ST_DOUBLE);
    signPumpOutOn = createSignal("signPumpOutOn", Signal::ST_DOUBLE);
    signPumpOutOff = createSignal("signPumpOutOff", Signal::ST_DOUBLE);
    signAnalSenLvl = createSignal("signAnalSenLvl", Signal::ST_DOUBLE);
    signAnalSenFlow = createSignal("signAnalSenFlow", Signal::ST_DOUBLE);
    signAnalSenTemp = createSignal("signAnalSenTemp", Signal::ST_DOUBLE);
    signAnalSenPress = createSignal("signAnalSenPress", Signal::ST_DOUBLE);
    signDigSenOverLvl = createSignal("signDigSenOverLvl", Signal::ST_DOUBLE);
    signDigSenTopLvl = createSignal("signDigSenTopLvl", Signal::ST_DOUBLE);

    // ситуации
    createSituation("analSenLvl");
    createSituation("analSenFlow");
    createSituation("analSenTemp");
    createSituation("analSenPress");
    createSituation("digSenOverLvl");
    createSituation("digSenTopLvl");
    createSituation("analActValveIn");
    createSituation("analActPumpOut");

    // порты
    inPump = createInputPort(0, "inPump", "INFO");
    inSol = createInputPort(1, "inSol", "INFO");
    outSol = createOutputPort(2, "outSol", "INFO");
    outInfo = createOutputPort(3, "outInfo", "INFO");
}

void TR_VAPE_TANK::setDataNames()
{
    std::vector<std::string> out1;
    out1.push_back("Расход раствора, кг/с");
    out1.push_back("Концентрация U, г/л");
    out1.push_back("Концентрация HNO3, г/л");
    out1.push_back("Температура, оС");
    outSol->setDataNames(out1);

    std::vector<std::string> out2;
    out2.push_back("Уровень, м3");
    out2.push_back("Расход, кг/л");
    out2.push_back("Температура, оС");
    out2.push_back("Давление, кПа");
    out2.push_back("Индикатор П");
    out2.push_back("Индикатор ВУ");
    outInfo->setDataNames(out2);
}

bool TR_VAPE_TANK::init(std::string &error, double h)
{
    setDataNames();

    // создания
    analAct.push_back(analUnit());
    analAct.push_back(analUnit());
    analSen.push_back(analUnit());
    analSen.push_back(analUnit());
    analSen.push_back(analUnit());
    analSen.push_back(analUnit());
    digSen.push_back(digUnit());
    digSen.push_back(digUnit());
    flowIn.push_back(flow());
    flowOut.push_back(flow());
    // зануления
    int cou = 0;
    // входной клапан
    analAct[cou].coeffMul = 0;
    analAct[cou].coeffSum = 0; cou++;
    // выходной виртуальный насос
    analAct[cou].coeffMul = 1;
    analAct[cou].coeffSum = 0; cou++;
    cou = 0;
    // датчик уровня
    analSen[cou].coeffMul = paramToDouble("propVol");
    analSen[cou].coeffSum = 0; cou++;
    // датчик расхода
    analSen[cou].coeffMul = 10;
    analSen[cou].coeffSum = 0; cou++;
    // датчик температуры
    analSen[cou].coeffMul = 250;
    analSen[cou].coeffSum = 0; cou++;
    // датчик давления
    analSen[cou].coeffMul = 1e5;
    analSen[cou].coeffSum = 0; cou++;

    // НУ
    mass = paramToDouble("initMass");
    conc[1] = paramToDouble("initConcU") * COEFF_MASS;
    conc[2] = paramToDouble("initConcHno3") * COEFF_MASS;
    conc[0] = 1 - conc[1] - conc[2];
    temp = paramToDouble("initTemp") + 273;

    // init
    valvePos = 0;
    pumpPow = 0;

    return true;
}

bool TR_VAPE_TANK::process(double t, double h, std::string &error)
{
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

    return true;
}

ICalcElement *Create()
{
    return new TR_VAPE_TANK();
}

void Release(ICalcElement *block)
{
    if(block->type() == "TR_VAPE_TANK") delete block;
}

std::string Type()
{
    return "TR_VAPE_TANK";
}
