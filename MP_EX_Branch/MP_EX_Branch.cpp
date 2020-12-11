#include "MP_EX_Branch.h"

MP_EX_Branch::MP_EX_Branch()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    InPort1 = createInputPort(0, "Вход1", "INFO");
    InPort2 = createInputPort(1, "Вход2", "INFO");
    OutPort = createOutputPort(2, "Выход", "INFO");
}

bool MP_EX_Branch::init(std::string &error, double h)
{
    std::vector<std::string> dn;
    dn.resize(6);

    dn[0] = "Расход";
    dn[1] = "Температура";
    dn[2] = "Концентрация U";
    dn[3] = "Концентрация Pu";
    dn[4] = "Концентрация HNO3";
    dn[5] = "Концентрация Np";

    OutPort->setDataNames(dn);

    return true;
}

bool MP_EX_Branch::process(double t, double h, std::string &error)
{
    double Q1 = 0, Q2 = 0, T1 = 0, T2 = 0, CU1 = 0, CU2 = 0, CPU1 = 0, CPU2 = 0, CHNO31 = 0, CHNO32 = 0, CNP1 = 0, CNP2 = 0;

    std::vector<double> in1, in2;
    in1 = InPort1->getInput();
    in2 = InPort2->getInput();

    if(!in1.empty()) {
        if(in1.size() != 6) {
            error = "Некорректная подача раствора во вход №1 тройника";
        }
        else {
            Q1     = in1[0];
            T1     = in1[1];
            CU1    = in1[2];
            CPU1   = in1[3];
            CHNO31 = in1[4];
            CNP1   = in1[5];
        }
    }

    if(!in2.empty()) {
        if(in2.size() != 6) {
            error = "Некорректная подача раствора во вход №2 тройника";
        }
        else {
            Q2     = in2[0];
            T2     = in2[1];
            CU2    = in2[2];
            CPU2   = in2[3];
            CHNO32 = in2[4];
            CNP2   = in2[5];
        }
    }

    std::vector<double> outVect;
    outVect.resize(6);

    if(Q1 + Q2 != 0.00) {
        outVect[0] = Q1 + Q2;
        outVect[1] = (Q1*T1 + Q2*T2)/(Q1 + Q2);
        outVect[2] = (Q1*CU1 + Q2*CU2)/(Q1 + Q2);
        outVect[3] = (Q1*CPU1 + Q2*CPU2)/(Q1 + Q2);
        outVect[4] = (Q1*CHNO31 + Q2*CHNO32)/(Q1 + Q2);
        outVect[5] = (Q1*CNP1 + Q2*CNP2)/(Q1 + Q2);
    }

    OutPort->setOut(outVect);

    return true;
}

ICalcElement *Create()
{
    return new MP_EX_Branch();
}

void Release(ICalcElement *block)
{
    if(block->type() == "MP_EX_Branch") delete block;
}

std::string Type()
{
    return "MP_EX_Branch";
}
