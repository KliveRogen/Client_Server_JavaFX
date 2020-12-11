#include "TR_VAPE_01T_AD6359.h"

void TR_VAPE_01T_AD6359::getInputs(double h) {
    flowOut[0].mass = inp1->getInput()[0] * h;
    flowOut[1].mass = inp2->getInput()[0] * h;
    flowIn[0].mass = ins->getInput()[0] * h;
    flowIn[0].conc[1] = ins->getInput()[1] * COEFF_MASS;
    flowIn[0].conc[2] = ins->getInput()[2] * COEFF_MASS;
    flowIn[0].conc[0] = 1 - flowIn[0].conc[1] - flowIn[0].conc[2];
    flowIn[0].temp = ins->getInput()[3] + 273;
}

void TR_VAPE_01T_AD6359::getParams() {
    if (paramToInt("ctrlMode") == 0) {
        if (paramToInt("ctrlValveOpen") == 1) {
            ctrlValve = 1;
        } else if (paramToInt("ctrlValveClose") == 1) {
            ctrlValve = -1;
        } else {
            ctrlValve = 0;
        }
    } else {
        if (signAnalActValveOpen->Value.intVal == 1) {
            ctrlValve = 1;
        } else if (signAnalActValveClose->Value.intVal == 1) {
            ctrlValve = -1;
        } else {
            ctrlValve = 0;
        }
    }
    valveRate = paramToDouble("ctrlValveRate");
}

void TR_VAPE_01T_AD6359::getInitValues() {
    // создания
    analAct.push_back(analUnit());
    analSen.push_back(analUnit());
    flowIn.push_back(flow());
    flowOut.push_back(flow());
    flowOut.push_back(flow());
    // зануления
    analAct[0].coeffMul = 1;
    analAct[0].coeffSum = 0;
    analSen[0].coeffMul = 237;
    analSen[0].coeffSum = 0;

    // НУ
    mass = paramToDouble("initMass");
    conc[1] = paramToDouble("initConcU") * COEFF_MASS;
    conc[2] = paramToDouble("initConcHno3") * COEFF_MASS;
    conc[0] = 1 - conc[1] - conc[2];
    temp = paramToDouble("initTemp") + 273;
}

void TR_VAPE_01T_AD6359::setAct(double h) {
    // расчет управления
    valvePos = valvePos + valveRate * ctrlValve * h;
    analAct[0].rawVal = valvePos;
    // рутина управления
    analAct[0].trig = isSituationActive("emerRunAnalActValve");
    analAct[0].opt = paramToDouble("emerSetAnalActValve");
    for (int i = 0; i < analAct.size(); i++) {
        analAct[i].inVal = (analAct[i].rawVal - analAct[i].coeffSum)/analAct[i].coeffMul; // перевод в стандартный формат (0-1)
        // аварии
        if (analAct[i].trig) {

        }
        // ограничения
        if (analAct[i].inVal > 1) {
            analAct[i].inVal = 1;
        }
        if (analAct[i].inVal < 0){
            analAct[i].inVal = 0;
        }
        analAct[i].outVal = analAct[i].inVal * analAct[i].coeffMul + analAct[i].coeffSum; // перевод в нормальный формат
    }
    valvePos = analAct[0].outVal;
    flowIn[0].mass = flowIn[0].mass * valvePos;
}

void TR_VAPE_01T_AD6359::setSen() {
    // расчет управления
    analSen[0].rawVal = mass / DENS;
    // рутина управления
    analSen[0].trig = isSituationActive("emerRunAnalSenLvl");
    analSen[0].opt = paramToDouble("emerSetAnalSenLvl");
    for (int i = 0; i < analSen.size(); i++) {
        analSen[i].inVal = (analSen[i].rawVal - analSen[i].coeffSum)/analSen[i].coeffMul; // перевод в стандартный формат (0-1)
        // аварии
        if (analSen[i].trig) {

        }
        // ограничения
        if (analSen[i].inVal > 1) {
            analSen[i].inVal = 1;
        }
        if (analSen[i].inVal < 0){
            analSen[i].inVal = 0;
        }
        analSen[i].outVal = analSen[i].inVal * analSen[i].coeffMul + analSen[i].coeffSum; // перевод в нормальный формат
    }
}

void TR_VAPE_01T_AD6359::setOutputs() {
    // на насос 1
    outp1->setOut(0,flowOut[0].mass);
    outp1->setOut(1,flowOut[0].conc[1] / COEFF_MASS);
    outp1->setOut(2,flowOut[0].conc[2] / COEFF_MASS);
    outp1->setOut(3,flowOut[0].temp - 273);
    // на насос 2
    outp2->setOut(0,flowOut[1].mass);
    outp2->setOut(1,flowOut[1].conc[1] / COEFF_MASS);
    outp2->setOut(2,flowOut[1].conc[2] / COEFF_MASS);
    outp2->setOut(3,flowOut[1].temp - 273);
    // на датчики
    outsen->setOut(0,analSen[0].outVal);
}

TR_VAPE_01T_AD6359::TR_VAPE_01T_AD6359()
{
	// Расчетный тип блока
    BlockCalcType = E_INITVALUES;

    // Параметры блока
    // управление
    createParameter("ctrlMode", "0");
    createParameter("ctrlValveOpen", "0");
    createParameter("ctrlValveClose", "0");
    createParameter("ctrlValveRate", "0");
    // ну
    createParameter("initMass", "0");
    createParameter("initConcU", "0");
    createParameter("initConcHno3", "0");
    createParameter("initTemp", "0");
    //аварии
    createParameter("emerSetAnalSenLvl", "0");
    createParameter("emerSetAnalActValve", "0");

	// Сигналы блока
    signAnalSenLvl = createSignal("signAnalSenLvl", Signal::ST_DOUBLE);
    signAnalActValveOpen = createSignal("signAnalActValveOpen", Signal::ST_DOUBLE);
    signAnalActValveClose = createSignal("signAnalActValveClose", Signal::ST_DOUBLE);

	// Порты блока
    inp1 = createInputPort(0, "inp1", "INFO");
    inp2 = createInputPort(1, "inp2", "INFO");
    ins = createInputPort(2, "ins", "INFO");
    outp1 = createOutputPort(3, "outp1", "INFO");
    outp2 = createOutputPort(4, "outp2", "INFO");
    outsen = createOutputPort(5, "outinfo", "INFO");

	// Отказы блока
    createSituation("emerRunAnalSenLvl");
    createSituation("emerRunAnalActValve");

}

void TR_VAPE_01T_AD6359::setDataNames()
{
    std::vector<std::string> out1,out2,out3,out4;
    out1.push_back("Расход раствора, кг/с");
    out1.push_back("Концентрация U, г/л");
    out1.push_back("Концентрация HNO3, г/л");
    out1.push_back("Температура раствора, оС");
    outp1->setDataNames(out1);
    out2.push_back("Расход раствора, кг/с");
    out2.push_back("Концентрация U, г/л");
    out2.push_back("Концентрация HNO3, г/л");
    out2.push_back("Температура раствора, оС");
    outp2->setDataNames(out2);
    out4.push_back("Уровень раствора в емкости, м3");
    outsen->setDataNames(out4);
}

bool TR_VAPE_01T_AD6359::init(std::string &error, double h)
{
    setDataNames();
    getInitValues();
    return true;
}

bool TR_VAPE_01T_AD6359::process(double t, double h, std::string &error)
{
    getInputs(h);
    getParams();
    setAct(h);
    tempMix();
    massCalc();
    setSen();
    setOutputs();
    return true;
}

ICalcElement *Create()
{
    return new TR_VAPE_01T_AD6359();
}

void Release(ICalcElement *block)
{
    if(block->type() == "TR_VAPE_01T_AD6359") delete block;
}

std::string Type()
{
    return "TR_VAPE_01T_AD6359";
}
