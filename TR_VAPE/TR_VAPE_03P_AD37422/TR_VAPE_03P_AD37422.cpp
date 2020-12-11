#include "TR_VAPE_03P_AD37422.h"

TR_VAPE_03P_AD37422::TR_VAPE_03P_AD37422()
{
	// Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
    createParameter("ctrlSwitch", "0");
    createParameter("ctrlPow", "0");
    createParameter("alarmSetSenPress", "0");
    createParameter("alarmSetActPow", "0");

    // Сигналы блока
    sigMonPress = createSignal("sigMonPress", Signal::ST_DOUBLE);
    sigCtrlPow = createSignal("sigCtrlPow", Signal::ST_DOUBLE);

    // Порты блока
    ins = createInputPort(0, "ins", "INFO");
    outs = createOutputPort(1, "out", "INFO");
    outinfo = createOutputPort(2, "outinfo", "INFO");
    outp = createOutputPort(3, "outp", "INFO");
    outinfosen = createOutputPort(4, "outinfosen", "INFO");

    // Отказы блока
    createSituation("alarmTrigSenPress");
    createSituation("alarmTrigActPow");
}

void TR_VAPE_03P_AD37422::setDataNames()
{
    std::vector<std::string> out1,out2,out3,out4;
    out1.push_back("Расход, кг/с");
    out1.push_back("Температура раствора, оС");
    out1.push_back("Концентрация U, г/л");
    out1.push_back("Концентрация HNO3, г/л");
    outs->setDataNames(out1);
    out2.push_back("Ожидаемый расход, кг/с");
    outp->setDataNames(out2);
    out3.push_back("Расход, кг/с");
    out3.push_back("Температура раствора, оС");
    out3.push_back("Концентрация U, г/л");
    out3.push_back("Концентрация HNO3, г/л");
    out3.push_back("Мощность насоса, %");
    outinfo->setDataNames(out3);
    out4.push_back("Давление в насосе, кгс/см2");
    outinfosen->setDataNames(out4);
}

bool TR_VAPE_03P_AD37422::init(std::string &error, double h)
{
    setDataNames();
    //инициализации
    for (int i = 0; i < NUM_ACT_ANAL; i++) {
        actAnal[i].senOrAct = 0;
        actAnal[i].digOrAnal = 0;
        actAnal[i].analVal[0] = 0;
        actAnal[i].analVal[1] = 0;
        actAnal[i].digVal[0] = 0;
        actAnal[i].digVal[1] = 0;
        actAnal[i].coeffMul = 0;
        actAnal[i].coeffSum = 0;
        actAnal[i].emerTrig = 0;
        actAnal[i].emerOpt = 0;
        actAnal[i].emerFlag = 0;
        actAnal[i].emerCou = 0;
        actAnal[i].emerBuf[0] = 0;
        actAnal[i].emerBuf[1] = 0;
        actAnal[i].emerDigBuf = 0;
    }
    for (int i = 0; i < NUM_SEN_ANAL; i++) {
        senAnal[i].senOrAct = 0;
        senAnal[i].digOrAnal = 0;
        senAnal[i].analVal[0] = 0;
        senAnal[i].analVal[1] = 0;
        senAnal[i].digVal[0] = 0;
        senAnal[i].digVal[1] = 0;
        senAnal[i].coeffMul = 0;
        senAnal[i].coeffSum = 0;
        senAnal[i].emerTrig = 0;
        senAnal[i].emerOpt = 0;
        senAnal[i].emerFlag = 0;
        senAnal[i].emerCou = 0;
        senAnal[i].emerBuf[0] = 0;
        senAnal[i].emerBuf[1] = 0;
        senAnal[i].emerDigBuf = 0;
    }
    powSet = 0;
    errorCode = 0;
    // мощность насоса  0 - 100 %
    actAnal[0].senOrAct = true;
    actAnal[0].digOrAnal = true;
    actAnal[0].coeffMul = 100;
    actAnal[0].coeffSum = 0;
    actAnal[0].emerTrig = isSituationActive("alarmTrigActPow");
    actAnal[0].emerOpt = paramToInt("alarmSetActPow");
    // датчик давления -1 - 9 кгс/см2
    senAnal[0].senOrAct = false;
    senAnal[0].digOrAnal = true;
    senAnal[0].coeffMul = 10;
    senAnal[0].coeffSum = -1;
    senAnal[0].emerTrig = isSituationActive("alarmTrigActPow");
    senAnal[0].emerOpt = paramToInt("alarmSetActPow");
    return true;
}

bool TR_VAPE_03P_AD37422::process(double t, double h, std::string &error)
{
    double  flowSet,powCtrl,flowIn,flowOut,pressOut;
    // ВВОД
    // получение управляющих воздействий
    if (paramToInt("ctrlSwitch") == 0) {
        actAnal[0].analVal[0] = paramToDouble("ctrlPow");
    } else {
        actAnal[0].analVal[0] = sigCtrlPow->Value.doubleVal;
    }
    if ( actAnal[0].analVal[0] < 0 || actAnal[0].analVal[0] > 100 ) {
        errorCode = 2;
    }
    actAnal[0].analVal[0] = (actAnal[0].analVal[0] - actAnal[0].coeffSum) /actAnal[0].coeffMul;
    actAnal[0].emerTrig = isSituationActive("alarmTrigActPow");
    actAnal[0].emerOpt = paramToInt("alarmSetActPow");
    for (int i = 0; i < NUM_ACT_ANAL; i++) {
        actOrSenRoutine(&actAnal[i]);
    }
    powCtrl = actAnal[0].analVal[1];
    // получение входов
    flowIn = ins->getInput()[0];
    if ( ins->getInput()[0] < 0 ||
         ins->getInput()[1] < 0 ||
         ins->getInput()[2] < 0 ||
         ins->getInput()[3] < 0 ||
         ins->getInput()[0] < (ins->getInput()[2] + ins->getInput()[3])/kcd ) {
        errorCode = 4;
    }
    // РАСЧЕТ
    // расчет расхода
    powSet = powSet + h*(powCtrl - powSet)/timePowInert;
    flowSet = coeffPowFlow * powSet;
    if ( flowSet <= flowIn ) {
        flowOut = flowSet;
    } else {
        flowOut = flowIn;
    }
    // расчет давления
    pressOut = powSet * coeffPowPress;
    // расчет датчиков
    senAnal[0].analVal[0] = pressOut;
    senAnal[0].analVal[0] = (senAnal[0].analVal[0] - senAnal[0].coeffSum) /senAnal[0].coeffMul;
    senAnal[0].emerTrig = isSituationActive("alarmTrigSenPress");
    senAnal[0].emerOpt = paramToInt("alarmSetSenPress");
    for (int i = 0; i < NUM_SEN_ANAL; i++) {
        actOrSenRoutine(&senAnal[i]);
    }
    // ВЫВОД
    // визуальные
    // давление
    sigMonPress->Value.doubleVal = senAnal[0].analVal[1];
    // выходы
    // запрос
    outp->setNewOut(0,flowSet);
    // выход
    outs->setNewOut(0,flowOut);
    outs->setNewOut(1,ins->getInput()[1]);
    outs->setNewOut(2,ins->getInput()[2]);
    outs->setNewOut(3,ins->getInput()[3]);
    // информационный
    outinfo->setNewOut(0,flowOut);
    outinfo->setNewOut(1,ins->getInput()[1]);
    outinfo->setNewOut(2,ins->getInput()[2]);
    outinfo->setNewOut(3,ins->getInput()[3]);
    outinfo->setNewOut(4,flowSet/coeffPowFlow);
    // датчики
    outinfosen->setNewOut(0,senAnal[0].analVal[1]);
    // проверка критичных ошибок блока
    if (errorCode != 0) {
        error = errorMsgCreator(errorCode,blockName);
        return false;
    }
    return true;
}


ICalcElement *Create()
{
    return new TR_VAPE_03P_AD37422();
}

void Release(ICalcElement *block)
{
    if(block->type() == "TR_VAPE_03P_AD37422") delete block;
}

std::string Type()
{
    return "TR_VAPE_03P_AD37422";
}
