#include "Transporter.h"

Transporter::Transporter()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
	createParameter("TRANSPORT_TIME", "60");

	// Сигналы блока
    ProgressSig = createSignal("PROGRESS", Signal::ST_DOUBLE);
    ElectroSig  = createSignal("ERR_ELECTRO", Signal::ST_BOOL);
    MechSig     = createSignal("ERR_MECH", Signal::ST_BOOL);

	// Порты блока
    InPort  = createInputPort(0, "Вход", "INFO");
    OutPort = createOutputPort(1, "Выход", "INFO");
    ConPort = createInputPort(2, "Управление", "INFO");

	// Отказы блока
	createSituation("Electro_error");
	createSituation("Mech_error");

}

bool Transporter::process(double t, double h, std::string &error)
{
    // Put your calculations here
    TransportTime = paramToDouble(Parameters[0]);
    if(TransportTime <= 0) {
        error = "Время транспортировки не может быть меньше или равно нулю!";
        return false;
    }

    // Проверка на управление
    bool transportEnable = true;
    if(ConPort->getConnectedPort() != 0) {
        std::vector<double> convect = ConPort->getInput();
        if(!convect.empty()) transportEnable = !(convect[0] == 0);
    }

    std::vector<double> inVect = InPort->getInput();
    if(inVect.empty()) return true;

    // Проверка, что пришел новый объект
    if(NewItem == false && inVect[0] == 1.00) {
        NewItem = true;
        inVect[0] = 0.00;

        if(!TransportDelays.empty()) {
            if(TransportDelays[0] == 0.00) {
                error = "Столкновение объектов при подаче в транспортную систему!";
                return false;
            }
        }

        TransportVect.push_back(inVect);
        TransportDelays.push_back(0.00);
    }
    else if(inVect[0] == 0.00) NewItem = false;

    if(Situations[0].Active) {
        transportEnable = false;
        ElectroSig->Value.boolVal = false;
    }

    if(Situations[1].Active) {
        transportEnable = false;
        MechSig->Value.boolVal = false;
    }

    bool needOut = false;
    if(transportEnable) {
        for(size_t i = 0; i < TransportDelays.size(); i++) {
            TransportDelays[i] += h;
            if(TransportDelays[i] >= TransportTime) needOut = true;
        }
    }

    if(!TransportDelays.empty()) ProgressSig->Value.doubleVal = TransportDelays[0];
    else ProgressSig->Value.doubleVal = 0.00;

    if(needOut) {
        OutVect = TransportVect[0];
        TransportVect.pop_front();
        TransportDelays.pop_front();
        OutVect[0] = 1;
    }
    else OutVect[0] = 0;

    OutPort->setOut(OutVect);
    return true;
}

bool Transporter::init(std::string &error, double h)
{
    // Put your initialization here
    std::vector<std::string> dn;
    if(InPort->getConnectedPort() == 0) dn.push_back("Импульс отправки");
    else dn = InPort->getConnectedPort()->getDataNames();
    OutPort->setDataNames(dn);
    OutVect.resize(dn.size());

    ProgressSig->Value.doubleVal = 0.00;
    ElectroSig->Value.boolVal    = false;
    MechSig->Value.boolVal       = false;

    TransportTime = paramToDouble(Parameters[0]);
    NewItem = false;

    return true;
}

ICalcElement *Create()
{
    return new Transporter();
}

void Release(ICalcElement *block)
{
    if(block->type() == "Transporter") delete block;
}

std::string Type()
{
    return "Transporter";
}
