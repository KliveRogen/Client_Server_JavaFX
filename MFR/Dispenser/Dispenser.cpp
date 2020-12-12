#include "Dispenser.h"
#include "math.h"

Dispenser::Dispenser()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
	createParameter("maxCapac", "80");
	createParameter("FillRate", "2");
	createParameter("FillInert", "5");
	createParameter("delayFiling", "5");

	// Сигналы блока
    UO2 = createSignal("UO2", Signal::ST_DOUBLE);
    PuO2 = createSignal("PuO2", Signal::ST_DOUBLE);
    C = createSignal("C", Signal::ST_DOUBLE);
    UN = createSignal("UN", Signal::ST_DOUBLE);
    PuN = createSignal("PuN", Signal::ST_DOUBLE);

	// Порты блока
    in = createInputPort(0, "UNKNOWN_NAME", "INFO"); //выходное значение концентраций дозатора
    out = createOutputPort(1, "UNKNOWN_NAME", "INFO"); //выход дозатора
    contr = createInputPort(2, "UNKNOWN_NAME", "INFO");//Порт управления дозатором (заполнение + работа)
    contrFromVortex = createInputPort(3, "UNKNOWN_NAME", "INFO");//подтверждение получения данных с АВС
    OUTInform = createOutputPort(4, "UNKNOWN_NAME", "INFO"); //вывод на показывающий прибор
	// Отказы блока

}

void Dispenser::setDataNames()
{
    std::vector<std::string> outname;
    outname.push_back("PowderMass");
    outname.push_back("C1m");
    outname.push_back("C2m");
    outname.push_back("C3m");
    outname.push_back("C4m");
    outname.push_back("C5m");
    out->setDataNames(outname);
    OUTInform->setDataNames(outname);
}

bool Dispenser::init(std::string &error, double h)
{
    // Put your initialization here
    dispen.containerFillingTime = 0;// зануление, чтобы избежать появления ненужных значений
    dispen.FillInert = paramToDouble(Parameters[2]); //Постоянная времени дозаторов
    dispen.delayFiling = paramToDouble(Parameters[3]); // Время задержки для перемещения между дозаторами
    for(int i =0;i<NumComp;i++) //
    {
        dispen.maxCapac[i] = paramToDouble(Parameters[0]); // Максимальная вместимость каждого дозатора. В частном случае можно принять их равными
        dispen.FillRate[i] = paramToDouble(Parameters[1]); //Массовый расход при заполеннии дозаторов кг/с
        dispen.FlagCalcTime = 0; // зануление, чтобы избежать появления ненужных значений
        dispen.ConcentrIn[i] = 0; // зануление, чтобы избежать появления ненужных значений
        dispen.ControlChanges = 1; // разрешение на подготовку новой партии порошка (для того, чтобы аждый раз программа не заходила в цикл обнуления первый)
    }
    dispen.NumNotZero = 0; // зануление, чтобы избежать появления ненужных значений
    dispen.numCalcNodes = 0; // зануление, чтобы избежать появления ненужных значений
	setDataNames();
    return true;
}

bool Dispenser::process(double t, double h, std::string &error)
{
    // Put your calculations here
    double dm, mCont, flagPrior=1; //локальные переменные для: 1) для определения приращения массы, 2) для суммирования массы, 3) определения очередности заполнения дозаторов ниже
    if((contr->getInput()[0]) && (contrFromVortex->getInput()[0]) && dispen.ControlChanges) //Должны совпасть 3 дейтсвия: 1)должно быть разренеие работы дозутору (константа "разрешение на работу блока" равна 1)
    {                                                                                       //2) чтобы аппарат вихревого слоя подтвердил готовность принять новый контейнер 3) чтобы новый контейнер уже не формировался
        dispen.FlagCalcTime=1;  //разрешение на формирование нового контерйа (следующий цикл)
        dispen.NumNotZero = 0;  // занулениче числа компонентов с ненулевой концентрацией
        dispen.numCalcNodes = 0;// зануление числа расчетных узлов
        dispen.PowderMass = 0;  // зануление массы порошка
        dispen.ControlChanges = 0;// запрет на формирование нового контейнера (т.к этот уже в процессе)
        for(int i=0;i<NumComp+1;i++) //при формировании нового контейнера выходные значения устанавливаются в 0 (режим ожидания)
        {
            out->setOut(i,0);
            OUTInform->setOut(i,0);
        }
    }
    for(int i =0;i<NumComp && dispen.FlagCalcTime==1;i++) //непосредственное формирование переменных для заполнения нового контейнера
    {
        dispen.ConcentrIn[i]=in->getInput()[i]; //получение входных концентраций
        if(dispen.ConcentrIn[i] > 0) //если значение концентрации больше 0, то формируем очередность заполнения в форме очереди и подсчет числа ненулевых элементов
        {
            dispen.NumNotZero = dispen.NumNotZero+1; //подсчет числа ненулевых элементов
            dispen.PriorFill[i] = flagPrior;        //формируем очередность заполнения в форме очереди
            flagPrior = flagPrior + 1;              //формирование новой свободной позиции в очереди
        }
        if(i==NumComp-1)                            //когда получили значение концентрации последнего элемента формируем число расчетных узлов
        {
            dispen.FlagCalcTime = 0;
            dispen.containerFillingTime = dispen.FillInert*5 + dispen.delayFiling; //вычисление времени заполнения одного дозатора (само заполнение за 5 постоянных времени + время на перемещение между дозаторами)
            dispen.numCalcNodes = ceil(dispen.NumNotZero*dispen.containerFillingTime/h); //вычисление числа узлов расчета для одного контейнера (время заполнения одного дозатор на их число)
            dispen.currenCalcNodes = 0; // утсановка первого расчетного узла
        }
    }
    //%%% БЛОК РАБОТЫ С ЗАПОЛЕНИЕМ КОНТЕЙНЕРА %%%
    if((dispen.currenCalcNodes < dispen.numCalcNodes) && contr->getInput()[0]) //проверяем заполненность контейнера (если нет, то продолжаем) и разрешение на заполнение
    {
        for(int j = 0; j < NumComp; j++)
        {
            if(dispen.PriorFill[j] == (ceil(dispen.NumNotZero*dispen.currenCalcNodes/(dispen.numCalcNodes)+1))) //Выбор номера текущего дозатора. Идея проста: Равномерно распределяяем кол-во расчетных узлов на все дозаторы
            {                                                                                                   // дальше с помощью округления в бОльшую сторону сверяем с приоритетом заполнения (1 т.к. в начальный момент число узлов 0, а 0 приоритет у 0 концентрации)
                if((dispen.PriorFill[j]*(dispen.FillInert*5/h)+(dispen.PriorFill[j]-1)*(dispen.delayFiling/h)) >= dispen.currenCalcNodes) //определяем заполнение или перемещение контейнера осуществляется. Идея в том, что заполняется подряд, но вот время для преемещения будет на 1 интервал меньше.
                {                                                                                                                          // это связано с тем, что текущее перемещение контейра мы учесть не можем т.к оно происходит сейчас. Учитывается в формуле после совершения
                    mCont = 0;  //определение максимальной ТЕКУЩЕЙ масса порошка для ТЕКУЩИХ компонентов, который в сумме в конце дадут 1.5кг
                    for(int i=0;i<=j;i++)
                    {
                        mCont = mCont + dispen.MaxMass * dispen.ConcentrIn[i];
                    }
                    dm = (mCont - dispen.PowderMass)/dispen.FillInert; //вычисление изменение массы дозатора
                }
                else
                {
                    dm = 0; //если контейнер на стадии перемещения, то масса не изменяется
                }
                dispen.currentDispenCapac[j] = dispen.currentDispenCapac[j] - dm*h; //учитываем тот факт, что масса порошка в дозаторах уменьшается
            }
        }
        dispen.PowderMass = dispen.PowderMass + dm*h; //вычисление текущей массы дозатора
        dispen.currenCalcNodes = dispen.currenCalcNodes + 1;
    }
    // %%% БЛОК РАБОТЫ С ДОЗАТОРОМ %%%
    if(contr->getInput()[1]) //%Проверяем на наличие сигнала заполнения какого-либо дозатора. Если возвращается не 0, то осуществляем поиск
    {
        for(int j=0;j<NumComp;j++)
        {
            dispen.currentDispenCapac[j]=  dispen.currentDispenCapac[j]+ dispen.FillRate[j]*h; //заполнения дозатора с постоянной скоростью
            if(dispen.currentDispenCapac[j] > dispen.maxCapac[j]*TopValue) //% Если дозатор заполнился до заданного уровня, то прекращение заполнения
            {
                //для данного случая установленный предел равен 95% от максимального значения вместимости дозатора в кг (переменная прописана в .h файле)
                dispen.currentDispenCapac[j]=  dispen.currentDispenCapac[j] - dispen.FillRate[j]*h; // Костыль. "защита от пеереполнения". Если пересыпали, то забираем сколько до этого добавили
            }
        }

    }
    if((dispen.currenCalcNodes == dispen.numCalcNodes) && contr->getInput()[0]) //Если управляющее воздействие всё еще сохранилось и прошло время необходимое для заполнения, то выводятся данные
    {
        out->setOut(0,dispen.PowderMass); //вывод массы порошка (всего) в АВС
        OUTInform->setOut(0,dispen.PowderMass); //вывод массы порошка (всего) на показывающий прибор
        for(int i=1;i<NumComp+1;i++) //Аналогичный вывод концентрации
        {
            out->setOut(i,dispen.ConcentrIn[i-1]);
            OUTInform->setOut(i,dispen.ConcentrIn[i-1]);
        }
        dispen.currenCalcNodes = dispen.currenCalcNodes+1;  //увеличения числа узлов на 1, чтобы в этот цикл больше не заходить для данного контейнера
        dispen.ControlChanges = 1; //Сигнал на разрешение заполнения нового контейнера
    }
    if(contr->getInput()[0] == 0)   //Если разрешение на работу блока 0, то и выхода все в 0
    {
        for(int i=0;i<NumComp+1;i++)
        {
            out->setOut(i,0);
            OUTInform->setOut(i,0);
        }
    }
    UO2->Value.doubleVal=dispen.currentDispenCapac[0]; //вывод текущего значения порошка в дозаторах
    PuO2->Value.doubleVal=dispen.currentDispenCapac[1];
    C->Value.doubleVal=dispen.currentDispenCapac[2];
    UN->Value.doubleVal=dispen.currentDispenCapac[3];
    PuN->Value.doubleVal=dispen.currentDispenCapac[4];
    return true;
}


ICalcElement *Create()
{
    return new Dispenser();
}

void Release(ICalcElement *block)
{
    if(block->type() == "Dispenser") delete block;
}

std::string Type()
{
    return "Dispenser";
}
