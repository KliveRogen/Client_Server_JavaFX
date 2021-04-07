#include "GCS_Ejector.h"
#include "math.h"
#include "../constants_list.h"
#define GAS_CONSTANT 8.314
#define AVOGADRO_CONSTANT 6.022e23


GCS_Ejector::GCS_Ejector()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
    createParameter("passiveGasDensity", "1.2"); //Плотность эжектируемого газа, кг/м^3
    createParameter("activeGasDensity", "1.2"); // Плотность эжектирующего газа, кг/м^3
    createParameter("passiveGasNozzleArea", "10207"); //Площадь выходного сечения сопла эжектируемого газа, мм^2
    createParameter("activeGasNozzleArea", "2290"); // Площадь выходного сечения сопла эжектирующего газа, мм^2
    createParameter("mixingChamberArea", "10207"); //Площадь сечения камеры смешения, мм^2
    createParameter("passiveGasMolarMass", "0.04"); //Молярная масса эжектируемого газа, кг/моль
    createParameter("activeGasMolarMass", "0.04"); //Молярная масса эжектирующего газа, кг/моль
    createParameter("passiveGasSpecificHeat", "1000"); //Удельная теплоемкость эжектируемого газа, Дж/(кг*К)
    createParameter("activeGasSpecificHeat", "1000"); //Удельная теплоемкость эжектирующего газа, Дж/(кг*К)
    createParameter("passiveGasHeatCapacityRatio", "1.4"); //Показатель адиабады для эжектируемого газа
    createParameter("activeGasHeatCapacityRatio", "1.4"); //Показатель адиабады для эжектирующего газа

	// Сигналы блока

	// Порты блока
    inPassiveGas=createInputPort(0, "UNKNOWN_NAME", "INFO");
    inActiveGas=createInputPort(1, "UNKNOWN_NAME", "INFO");
    outMixingGas=createOutputPort(2, "UNKNOWN_NAME", "INFO");
    outEjectorParameters=createOutputPort(3, "UNKNOWN_NAME", "INFO");

	// Отказы блока
	createSituation("calcDist");

}
void GCS_Ejector::setDataNames()
{
    //выходные параметры газа
    std::vector<std::string> outMixingGasName;
    outMixingGasName.push_back("Объемный расход, м^3/с");
    outMixingGasName.push_back("Давление, Па");
    outMixingGasName.push_back("Температура, °C");
    outMixingGasName.push_back("Объемная активность газа, Бк/м^3");
    outMixingGasName.push_back("Объемная доля частиц в газе, отн. ед.");
    outMixingGas->setDataNames(outMixingGasName);
    //выходные параметры эжектора
    std::vector<std::string> outEjectorParametersName;
    outEjectorParametersName.push_back("Коэффициент эжекции");
    outEjectorParameters->setDataNames(outEjectorParametersName);
}
bool GCS_Ejector::init(std::string &error, double h)
{
    setDataNames();
    //проверка корректности параметров
    for (int i = 0; i < (int)Parameters.size(); i++){
        if (paramToDouble(Parameters[i])<0){
            error = "Ошибка в заполнении исходных данных!";
            return false;
        }
    }
    return true;
}
bool GCS_Ejector::process(double t, double h, std::string &error)
{
    // Put your calculations here

    //переменные для параметров эжектора и газа
    double passiveGasDensity, activeGasDensity, passiveGasNozzleArea,activeGasNozzleArea,mixingChamberArea,
            passiveGasMolarMass, activeGasMolarMass, passiveGasSpecificHeat, activeGasSpecificHeat,
            passiveGasHeatCapacityRatio,activeGasHeatCapacityRatio;
    //переменные для входов-выходов
    double passiveGasVolumeFlowRate, passiveGasInputPressure, passiveGasInputTemperature, passiveGasInputActivity,
            passiveGasInputParticleFraction, activeGasVolumeFlowRate, activeGasInputPressure, activeGasInputTemperature, activeGasInputActivity,
            activeGasInputParticleFraction, mixingGasVolumeFlowRateCurrent, mixingGasOutputPressureCurrent, mixingGasTemperatureCurrent, mixingGasActivityCurrent,
            mixingGasParticleFractionCurrent;
    //промежуточные перменные
    double passiveGasSpeed, passiveGasTotalTemperature, passiveGasSpecificSpeed, passiveGasSpecificImpulse, ejectionCoef, temperatureRatio,
            activeGasSpeed,activeGasTotalTemperature, activeGasSpecificSpeed, activeGasSpecificImpulse, activeGasSpecificFlowRate,
            activeGasTotalPressure, mixingGasSpecificSpeed, mixingGasSpecificFlowRate, mixingGasTotalPressure, mixingGasTotalTemperature,
            mixingGasSpeed, mixingGasSpecificImpulse, mixingGasHeatCapacityRatio, mixingGasMolarMass;
    //параметры эжектора и газов
    passiveGasDensity = paramToDouble("passiveGasDensity");
    activeGasDensity = paramToDouble("activeGasDensity");
    passiveGasNozzleArea = paramToDouble("passiveGasNozzleArea");
    activeGasNozzleArea = paramToDouble("activeGasNozzleArea");
    mixingChamberArea = paramToDouble("mixingChamberArea");
    passiveGasMolarMass = paramToDouble("passiveGasMolarMass");
    activeGasMolarMass = paramToDouble("activeGasMolarMass");
    passiveGasSpecificHeat = paramToDouble("passiveGasSpecificHeat");
    activeGasSpecificHeat = paramToDouble("activeGasSpecificHeat");
    passiveGasHeatCapacityRatio = paramToDouble("passiveGasHeatCapacityRatio");
    activeGasHeatCapacityRatio = paramToDouble("activeGasHeatCapacityRatio");
    //считывание входных значений эжектируемого газа
    passiveGasVolumeFlowRate = inPassiveGas->getInput()[0];//объемный расход эжектируемого газа, м^3/с
    passiveGasInputPressure = inPassiveGas->getInput()[1];//давление эжектируемого газа, Па
    passiveGasInputTemperature = inPassiveGas->getInput()[2];//температура эжектируемого газа, град. Цел.
    passiveGasInputActivity = inPassiveGas->getInput()[3];//активность эжектируемого газа, Бк
    passiveGasInputParticleFraction = inPassiveGas->getInput()[4];//объемная доля частиц в эжектируемом газе, отн. ед.
    //считывание входных значений эжектирующего газа
    activeGasVolumeFlowRate = inActiveGas->getInput()[0];//объемный расход эжектирующего газа, м^3/с
    activeGasInputPressure = inActiveGas->getInput()[1];//давление эжектирующего газа, Па
    activeGasInputTemperature = inActiveGas->getInput()[2];//температура эжектирующего газа, град. Цел.
    activeGasInputActivity = inActiveGas->getInput()[3];//активность эжектирующего газа, Бк
    activeGasInputParticleFraction = inActiveGas->getInput()[4];//объемная доля частиц в эжектирующем газе, отн. ед.
    //проверка соотношения давлений перед соплами
    if (activeGasInputPressure < passiveGasInputPressure){
        error = "Ошибка в значении давления активного газа!";
    }

    //расчет эжектора
    //расчет доп параметров для входных газов
    //скорости потока
    passiveGasSpeed = passiveGasVolumeFlowRate/(passiveGasNozzleArea / 1000000); //скорость потока эжектируемого газа, м/с
    activeGasSpeed = activeGasVolumeFlowRate/(activeGasNozzleArea / 1000000); //скорость потока эжектирующего газа, м/с
    //температуры торможения
    passiveGasTotalTemperature = (passiveGasInputTemperature + 273) + pow(passiveGasSpeed,2) / (2 * passiveGasSpecificHeat);   //температура торможения эжектируемого газа, К
    activeGasTotalTemperature = (activeGasInputTemperature + 273) + pow(activeGasSpeed,2) / (2 * activeGasSpecificHeat);   //температура торможения эжектирующего газа, К
    //приведенные скорости
    passiveGasSpecificSpeed = passiveGasSpeed / sqrt(2 * passiveGasHeatCapacityRatio * GAS_CONSTANT *
                               passiveGasTotalTemperature / ((passiveGasHeatCapacityRatio + 1) * passiveGasMolarMass)); //приведенная скорость эжектируемого газа
    activeGasSpecificSpeed = activeGasSpeed / sqrt(2 * activeGasHeatCapacityRatio * GAS_CONSTANT *
                               activeGasTotalTemperature / ((activeGasHeatCapacityRatio + 1) * activeGasMolarMass)); //приведенная скорость эжектирующего газа
    //отношение температур торможения эжектируемого и эжектирующего потоков
    temperatureRatio = passiveGasTotalTemperature / activeGasTotalTemperature;
    //коэффициент эжекции
    ejectionCoef = (passiveGasVolumeFlowRate * passiveGasDensity)/(activeGasVolumeFlowRate * activeGasDensity);
    //приведенные импульсы
    passiveGasSpecificImpulse = passiveGasSpecificSpeed + 1 / passiveGasSpecificSpeed; //приведенный импульс эжектируемого газа
    activeGasSpecificImpulse = activeGasSpecificSpeed + 1 / activeGasSpecificSpeed; //приведенный импульс эжектирующего газа
    mixingGasSpecificImpulse = (activeGasSpecificImpulse + ejectionCoef * sqrt(temperatureRatio) * passiveGasSpecificImpulse) /
            sqrt((ejectionCoef + 1) * (1 + ejectionCoef * temperatureRatio)); //приведенный импульс смеси газов
    //приведенная скорость смеси газов
    mixingGasSpecificSpeed = (mixingGasSpecificImpulse - sqrt(pow(mixingGasSpecificImpulse,2) - 4)) / 2;
    //усреднение показателя адиабаты для смеси газов
    mixingGasHeatCapacityRatio = (passiveGasVolumeFlowRate * passiveGasHeatCapacityRatio + activeGasVolumeFlowRate * activeGasHeatCapacityRatio) /
            (passiveGasVolumeFlowRate + activeGasVolumeFlowRate);
    //усреднение молярной массы для смеси газов, кг/моль
    mixingGasMolarMass = (passiveGasVolumeFlowRate * passiveGasMolarMass + activeGasVolumeFlowRate * activeGasMolarMass) /
            (passiveGasVolumeFlowRate + activeGasVolumeFlowRate);
    //приведенные расходы
    activeGasSpecificFlowRate = pow((activeGasHeatCapacityRatio + 1) / 2, (1 / (activeGasHeatCapacityRatio - 1))) *
            activeGasSpecificSpeed * pow((1 - (activeGasHeatCapacityRatio - 1) / (activeGasHeatCapacityRatio + 1) * pow(activeGasSpecificSpeed, 2)), (1/(activeGasHeatCapacityRatio-1))); //приведенный расход для эжектирующего газа
    mixingGasSpecificFlowRate = pow((mixingGasHeatCapacityRatio + 1) / 2, (1 / (mixingGasHeatCapacityRatio - 1))) *
            mixingGasSpecificSpeed * pow((1 - (mixingGasHeatCapacityRatio - 1) / (mixingGasHeatCapacityRatio + 1) * pow(mixingGasSpecificSpeed, 2)), (1/(mixingGasHeatCapacityRatio-1))); //приведенный расход для смеси газов
    //полное давление для эжектирующего газа, Па
    activeGasTotalPressure = activeGasInputPressure / pow(1 - (activeGasHeatCapacityRatio - 1) / (activeGasHeatCapacityRatio + 1) * pow(activeGasSpecificSpeed, 2), (activeGasHeatCapacityRatio/(activeGasHeatCapacityRatio-1)));
    //полное давление для смеси газов, Па
    mixingGasTotalPressure = activeGasTotalPressure * (sqrt((ejectionCoef + 1) * (ejectionCoef * temperatureRatio + 1)) / (1 + passiveGasNozzleArea/activeGasNozzleArea)) * activeGasSpecificFlowRate / mixingGasSpecificFlowRate;
    //статическое давление для смести газов, Па
    mixingGasOutputPressureCurrent = mixingGasTotalPressure * pow(1 - (mixingGasHeatCapacityRatio - 1) / (mixingGasHeatCapacityRatio + 1) * pow(mixingGasSpecificSpeed, 2), (mixingGasHeatCapacityRatio/(mixingGasHeatCapacityRatio-1)));
    //температура торможения смеси газов, К
    mixingGasTotalTemperature = (activeGasInputTemperature + 273) * (ejectionCoef * temperatureRatio + 1) / (ejectionCoef + 1);
    //текущая температура смеси газов, град Цельсия
    mixingGasTemperatureCurrent = - 273 + mixingGasTotalTemperature * (1 - (mixingGasHeatCapacityRatio - 1) / (mixingGasHeatCapacityRatio + 1) * pow(mixingGasSpecificSpeed, 2));
    //скорость смеси газов
    mixingGasSpeed = mixingGasSpecificSpeed * sqrt(2 * mixingGasHeatCapacityRatio / (mixingGasHeatCapacityRatio + 1) * GAS_CONSTANT / mixingGasMolarMass * mixingGasTotalTemperature);
    //объемный расход смеси газов, м^3/с
    mixingGasVolumeFlowRateCurrent = mixingGasSpeed * (mixingChamberArea / 1000000);
    //активность смеси газов, Бк
    mixingGasActivityCurrent = (passiveGasVolumeFlowRate * passiveGasInputActivity + activeGasVolumeFlowRate * activeGasInputActivity) /
            (passiveGasVolumeFlowRate + activeGasVolumeFlowRate);
    //объемная доля частиц в смеси газов, отн. ед.
    mixingGasParticleFractionCurrent = (passiveGasVolumeFlowRate * passiveGasInputParticleFraction + activeGasVolumeFlowRate * activeGasInputParticleFraction) /
            (passiveGasVolumeFlowRate + activeGasVolumeFlowRate);
    //передача значений на выходные порты
    outMixingGas->setOut(0, mixingGasVolumeFlowRateCurrent);//объемный расход смеси газов, м^3/с
    outMixingGas->setOut(1, mixingGasOutputPressureCurrent);//давление смеси газов, Па
    outMixingGas->setOut(2, mixingGasTemperatureCurrent);//температура смеси газов, град. Цел.
    outMixingGas->setOut(3, mixingGasActivityCurrent);//активность смеси газов, Бк
    outMixingGas->setOut(4, mixingGasParticleFractionCurrent);//объемная доля частиц в смеси газов, отн. ед.
    outEjectorParameters->setOut(0, ejectionCoef);//коэффициент эжекции
    outEjectorParameters->setOut(1, passiveGasSpeed);//коэффициент эжекции
    outEjectorParameters->setOut(2, activeGasSpeed);//коэффициент эжекции
    outEjectorParameters->setOut(3, passiveGasTotalTemperature);//коэффициент эжекции
    outEjectorParameters->setOut(4, activeGasTotalTemperature);//коэффициент эжекции
    return true;
}
ICalcElement *Create()
{
    return new GCS_Ejector();
}
void Release(ICalcElement *block)
{
    if(block->type() == "GCS_Ejector") delete block;
}
std::string Type()
{
    return "GCS_Ejector";
}
