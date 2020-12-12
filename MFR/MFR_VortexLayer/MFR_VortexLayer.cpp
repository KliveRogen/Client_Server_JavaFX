#include "MFR_VortexLayer.h"
#include "math.h"

MFR_VortexLayer::MFR_VortexLayer()
{
	// Расчетный тип блока
    BlockCalcType = E_OUTPUTONLY;
    // Параметры блока
	createParameter("needleNum", "255");
	createParameter("Height", "4");
	createParameter("waterTemp", "30");
	createParameter("Diameter", "10.5");
	createParameter("needleNumCrit", "283");
	// Сигналы блока
	// Порты блока
    in = createInputPort(0, "UNKNOWN_NAME", "INFO");
    out = createOutputPort(1, "UNKNOWN_NAME", "INFO");
    contrToDisp = createOutputPort(2, "UNKNOWN_NAME", "INFO");
    OUTInform = createOutputPort(3, "UNKNOWN_NAME", "INFO");
	// Отказы блока
}

void MFR_VortexLayer::setDataNames()
{
    std::vector<std::string> outname, contrToDispFlag;
    outname.push_back("Масса порошка в 1ой ячейке");
    outname.push_back("Масса порошка в 2ой ячейке");
    outname.push_back("Масса порошка в 3ей ячейке");
    outname.push_back("Масса порошка в 4ой ячейке");
    contrToDispFlag.push_back("contrToDispFlag");
    out->setDataNames(outname);
    OUTInform->setDataNames(outname);
    contrToDisp->setDataNames(contrToDispFlag);
}

bool MFR_VortexLayer::init(std::string &error, double h)
{
    // Put your initialization here
    setDataNames();
    containerPar.needleNum = paramToInt("needleNum");   //Получение значения числа игл
    containerPar.Height = paramToInt("Height");         //Получение значения высоты контейнера
    containerPar.needleNumCrit = paramToInt("needleNumCrit");//Получение значения критического числа игл
    containerPar.Square = PI*paramToDouble("Diameter")*paramToDouble("Diameter")/4; // площадь основания контейнера
    containerPar.waterTemperature=paramToDouble("waterTemp"); //Получение значения температуры охлаждающей воды
    containerVolume = containerPar.Square*containerPar.Height; //Вычисление объема контейнера
    cellSurfaceArea = containerPar.Square*0.001; //%Площадь поверхности одной ячейки
    NumCell = round(containerPar.Height/pow(containerVolume/containerPar.needleNumCrit,1/3));//Вычисление числа ячеек на основе объема контейнера и критической загрузки
    HeightOneCell = containerPar.Height/NumCell;    //Вычисление высоты одной ячейки
    HeightSubmergedPart = containerPar.HeightSubmergedPart; //запоминнаие высоты рабочей зоны АВС (т.к. в последующем данный параметр уменьшаться будет)
    Efficiency = new double[NumCell];   //Создание массивы эффективности каждой ячейки
    for(int a=0;a<NumCell;a++)          //Вычисление эффективности каждой ячейки. Суть в том, что контейнер может колебаться и не весь порошок всегда находиться в ЭМ поле
    {
        if(HeightSubmergedPart >= (HeightOneCell + containerPar.Amplitude))
        {
            Efficiency[NumCell-a-1] = 1;
            HeightSubmergedPart = HeightSubmergedPart - 1;
        }
        else if(HeightSubmergedPart < (HeightOneCell + containerPar.Amplitude) && HeightSubmergedPart > 0)
        {
            Efficiency[NumCell-a-1] = 1-0.636*((HeightOneCell + containerPar.Amplitude-HeightSubmergedPart)/HeightSubmergedPart);
            HeightSubmergedPart = HeightSubmergedPart - 1;
        }
        else
        {
            Efficiency[NumCell-a-1] = 0;
        }
    }
    //%%%% СМЕШИВАНИЕ,ИЗМЕЛЬЧЕНИЕ, ДВИЖЕНИЕ ИГЛ, НАГРЕВ И ОХЛАЖДЕНИЕ%%%% <-Создание исходных массивов
    needleMovement = new double*[containerPar.needleNum];
    for(int i = 0; i < containerPar.needleNum; ++i)
    {
        needleMovement[i] = new double[3]; //3 т.к. мы отслеживаем 3 точки в цилиндрической системе координат для описания движения (радиус, угол поворота и высоту)
    }

    powderWeightContainer = new double*[NumComp+1]; //Двумерная матрица для ослеживания перемешивания порошка. В первой строке содержится суммарная масса оставшихся ниже строк для данного столбца
    for(int i = 0; i < NumComp+1; ++i) //Здесь и далее где NumComp+1, это значит, что 0 строка нужна для суммирования по столбцам всех оставшихся ниже строк. Это принципиальное условие работы модели
    {
        powderWeightContainer[i] = new double[NumCell];
        for(int j = 0; j < NumCell; ++j)
        {
            powderWeightContainer[i][j] = 0;
        }
    }
    changePowderWeight = new double*[NumComp+1]; //Изменение приращения массы для каждого контейнера
    for(int i = 0; i < NumComp+1; ++i)
    {
        changePowderWeight[i] = new double[NumCell];
    }
    particleCrushing = new double[NumCompCrash]; // Доля крупности каждого компонента
    for(int i = 0; i < NumCompCrash; ++i)
    {
        particleCrushing[i] = containerPar.particleCrushing[i]; //!!!!!!!!
    }
   temperature = new double[NumCell];   //Значение температуры в кажой ячейке
    for(int i=0;i<NumCell;i++)
    {
        temperature[i] = containerPar.temperatureStart;
    }
    temperatureChange = new double[NumCell]; //Значение изменения температуры в кажой ячейк
    meanCapacity = 0;//% Вспомогательная переменная для расчета среднего значения теплоёмкости смеси
    for(int i=0;i<5;i++)
    {
        meanCapacity = meanCapacity + containerPar.heatCapacity[i]*Concentration[i];
    }
    numberNeedlesOneLayer = new double[NumCell]; //Число игл в одном слое. Используется при расчете нагрева
    currenNodesMix=0;
    startCalc = 0;
    nodesMix = 0;
    return true;
}

bool MFR_VortexLayer::process(double t, double h, std::string &error)
{
    double radiusChange,angleChange,height,inputPowder;
    int heightChange;
    if((currenNodesMix == 0) && (in->getInput()[0] > 0))            //если никакие значения на расчитываются и есть новый подготовленный контейнер, то начинаем подготовку расчета
    {
        contrToDisp->setOut(0,0);                                   //уставновка выхода синхронизации в 0. Информация о том, что новый контейнер не нужен. Начата/идёт работа с последним
        containerPar.powderMaxMass = in->getInput()[0];             // Получение значения массы контейнера
        powderMassOneCell = containerPar.powderMaxMass/(NumCell-1); //Масса порошка в одной ячейке
        NumNotZero = 0;
        for(int a=0;a<NumComp;a++)                                  //Получения концентраций и определенеие ненулевых значений компонентов
        {
            Concentration[a] = in->getInput()[a+1];
            if(Concentration[a] > 0)
            {
                NumNotZero++;           //подсчет числа ненулевых элементов
            }
            nodesMix = ceil(containerPar.timeMix/h); // количество расчетных узлов для смешения

        }
        for(int j=0; j<containerPar.needleNum;j++) //начальное положение игл по слою, радиусу и углу в цилиндрических координатах
        {
            needleMovement[j][0] = sqrt((containerPar.Square/PI))*(rand()%10)*0.1;// %распределение по радиусу
            needleMovement[j][1] = (rand()%101)*Norm;// %распределение по углу фи
            needleMovement[j][2] = NumCell-(j%NumCell);// %распределение по слоям
        }
        b=NumCell-1; //%номер текущей ячейки (заполнение произодилось снизу вверх)
        remainingPowderMass = 0; //%Остаточная масса материала для разбиения по ячейкам
        for(int j=1;j<NumComp+1;j++)        //вычисление начального заполнения по ячейкам
        {
            if((powderMassOneCell - (powderWeightContainer[0][b]+ containerPar.powderMaxMass*Concentration[j-1])) <= 0)  //% Проверка на вместимость в b-ую ячейку массы порошка j-ого компонента
            {
                remainingPowderMass = containerPar.powderMaxMass*Concentration[j-1]-(powderMassOneCell-powderWeightContainer[0][b]); //% Если непомещается, то дополняем массу порошка в рассматриваемой ячейки до максимума (m0)
                powderWeightContainer[j][b] = powderMassOneCell-powderWeightContainer[0][b];    // записываем в последующую ячейку оставщуюся массу
                powderWeightContainer[0][b] = powderMassOneCell;    // дополняем до максимума вот здесь
                b = b-1;    //уменьшаем номер ячейки на 1
                while(remainingPowderMass != 0) //Далее продолжаем цикл пока оставшаяся масса не станет равна 0
                {
                    if((powderMassOneCell - remainingPowderMass) > 0)   // если оставшаяся масса  j-ого компонента выходит в одну ячейку, то заполняем её на данную величину
                    {
                        powderWeightContainer[j][b] = remainingPowderMass;
                        remainingPowderMass = 0;
                    }
                    else    // если остается еще масса j-ого компонента сверх заполнения одной ячейки, то продолжаем цикл
                    {
                        powderWeightContainer[j][b] = powderMassOneCell;
                        remainingPowderMass = remainingPowderMass - powderMassOneCell;
                        powderWeightContainer[0][b] = powderMassOneCell;
                        b = b-1;
                    }
                }
            }
            else    //если с первого раза вся масса j-ого компонента вошла, то записываем и переходим дальше
            {
                powderWeightContainer[j][b]= containerPar.powderMaxMass*Concentration[j-1];   //% если вмещается, то запоминаем
            }
            powderWeightContainer[0][b]=0;  //%Находим общую массу всех компонентов в рассматриваемоя ячейке (учитываем, что ранее заполненные мы уже "просуммировали" - учли, что они заполнены полностью
            for(int i=1;i<NumComp+1;i++)
            {
                powderWeightContainer[0][b] = powderWeightContainer[0][b] + powderWeightContainer[i][b]; //%Находим общую массу всех компонентов (в основном для проверки работы алгоритма)
            }
        }
        startCalc = 1; //разрешение на начало расчета т.к. основные приготовления закончены
    }
    if((currenNodesMix < nodesMix) && startCalc)
    {
        for(int j=1;j<NumComp+1;j++) //расчёт смешивания порошка
        {
            changePowderWeight[j][0] = (containerPar.reverseFlow[j-1]*powderWeightContainer[j][1]-(containerPar.reverseFlow[j-1])*powderWeightContainer[j][0]); //Учитываем, что часть массы уходит в нижнюю ячейку (с минусом) учитываем обратный поток из нижней ячейки (с +)
            powderWeightContainer[j][0] = powderWeightContainer[j][0] + changePowderWeight[j][0]*h;
            for(int i=1;i<NumCell-1;i++)
            {
                changePowderWeight[j][i] = containerPar.reverseFlow[j-1]*powderWeightContainer[j][i-1]+ containerPar.reverseFlow[j-1]*powderWeightContainer[j][i+1]-(2*containerPar.reverseFlow[j-1])*powderWeightContainer[j][i];//Учитываем, что часть массы уходит в нижнюю и верхнюю ячейку (с минусом) учитываем обратный поток из нижней и верхней ячейки (с +)
                powderWeightContainer[j][i] = powderWeightContainer[j][i] + changePowderWeight[j][i]*h;
            }
            changePowderWeight[j][NumCell-1] = (containerPar.reverseFlow[j-1]*powderWeightContainer[j][NumCell-2]-(containerPar.reverseFlow[j-1])*powderWeightContainer[j][NumCell-1]); //Учитываем, что часть массы уходит в верхнюю ячейку (с минусом) учитываем обратный поток из верхней ячейки (с +)
            powderWeightContainer[j][NumCell-1] = powderWeightContainer[j][NumCell-1] + changePowderWeight[j][NumCell-1]*h;
        }

        for(int j=0;j<NumCell;j++) //Суммирное изменение массы компонентов смеси в одной ячейки (а значит и вычисления перемешивания и распределения компонентов)
        {
            powderWeightContainer[0][j] = 0;
            for(int i=1;i<NumComp+1;i++)
            {
                powderWeightContainer[0][j] = powderWeightContainer[0][j] +(powderWeightContainer[i][j]);
            }
        }

        //  %%% БЛОК ДВИЖЕНИЯ ИГЛ %%%
        for(int j=0;j<containerPar.needleNum;j++)
        {
            //%%%% РАДИУС %%%%%
            //% На иглу действует множество сил. В данном случае вероятностная составляющая отражает возможность столкнуться иглам между друг другом, с стенкой
            radiusChange = containerPar.sumForcesRadius*(1-containerPar.needleNum/(containerPar.needleNumCrit*NumCell))*((rand()%11)*0.1)/containerPar.mNeedles;
            if(rand()%2) //%в данном случае имеется равная вероятность столкнуться и избежать столкновение. Если было столкновение (остаток от деленя 1), то игла движется в противоположном направлении
            {
                radiusChange = radiusChange * -1;
            }
            needleMovement[j][0] = needleMovement[j][0]+radiusChange*h*h*Efficiency[int(needleMovement[j][2])-1]; // определяем движение иглы с учетом эффективности ячейки
            if(needleMovement[j][0] > (sqrt((containerPar.Square/PI))*TopValue)) //%Исключается возможность выхода за стенки камеры/ границу в 0.95 взял при рассматрении нескольких случае на практике. При величине 0,95 выше вероятность оказаться "за стенкой"
            {
                needleMovement[j][0] = sqrt((containerPar.Square/PI))*TopValue;
            }
            if(needleMovement[j][0] < (sqrt((containerPar.Square/PI))*TopValue)) //%Радиус на может быть в цилиндрических кординатах отрицательной ввеличиной
            {
                needleMovement[j][0] = sqrt((containerPar.Square/PI))*TopValue;
            }
            // %%%% УГОЛ %%%%%
            angleChange = containerPar.sumForcesAngle*(1-containerPar.needleNum/(containerPar.needleNumCrit*NumCell))*((rand()%11)*0.1)*needleMovement[j][0]/containerPar.mNeedles; //%Аналогично с радиусом.
            if(rand()%2) //%Аналогично с радиусом.
            {
                angleChange = angleChange * -1;
            }
            needleMovement[j][1] = needleMovement[j][1]+angleChange*h*h*Efficiency[int(needleMovement[j][2])-1]; //аналогично с радиусом
            //%%%% ПЕРЕХОД МЕЖДУ СЛОЯМИ %%%%%
            height = (rand()%NormDistrib); //%Если функция описывает нормальное распределение, то разбиваем на 3 одинаковые вероятности: больше int((NormDistrib-1)*2/3 подняться вверх, int((NormDistrib-1)/3 опуститься вниз. Что между - ничего не происходит
            heightChange=0;
            if((height >= int((NormDistrib-1)*2/3)) && (((currenNodesMix+1)%int(TimeStep/h))==0)) //% при больше установленном значении игла поднимается в слой выше при прошестии TimeStep секунды
            {
                heightChange = -1; // не может выйти выше 1ой ячейки
                if(needleMovement[j][2] == 1)
                {
                    heightChange = 0;
                }
            }
            if((height <= int((NormDistrib-1)/3)) && (((currenNodesMix+1)%int(TimeStep/h))==0))    //% при меньшем установленном значении игла опускается в слой ниже
            {
                heightChange = 1; // не можем выйти ниже нижней ячейки
                if(needleMovement[j][2] == NumCell)
                {
                    heightChange = 0;
                }
            }
            needleMovement[j][2] = needleMovement[j][2] + heightChange;
        }
        //%%%БЛОК ИЗМЕЛЬЧЕНИЯ%%%
        particleCrushing[0]= particleCrushing[0] + (-containerPar.selectionFunction[0]*particleCrushing[0])*h; //% В первую категорию ничего не приходит, только убывает
        inputPowder = 0; //промежутоная переменная для расчета
        for(int j=1;j<NumCompCrash-1;j++) //вычисляем убывание материала
        {
            for(int i=0;i<NumCompCrash;i++)
            {
                inputPowder = inputPowder + containerPar.selectionFunction[i]*containerPar.destructionFunction[j][i]* particleCrushing[i];
            }
            particleCrushing[j] = particleCrushing[j] + (inputPowder-containerPar.selectionFunction[j]*particleCrushing[j])*h;
            inputPowder = 0;
        }
        for(int i=0;i<NumCompCrash;i++)//вычисляем прибавление материала
        {
            inputPowder = inputPowder + containerPar.selectionFunction[i]*containerPar.destructionFunction[3][i]*particleCrushing[i];
        }
        particleCrushing[NumCompCrash]= particleCrushing[NumCompCrash] + inputPowder*h;
        inputPowder = 0;
        //%%%БЛОК НАГРЕВА И ОХЛАЖДЕНИЯ%%%
        // % От количества игл в слое вычисляется нагревпорошка
        for(int i=0;i<NumCell;i++)
        {
            numberNeedlesOneLayer[i] = 0; // зануляем предыдущие значения
        }
        // %Промежутоная переменная для вычисления текущего числа игл в каждом слое
        for(int j=0;j<containerPar.needleNum;j++)
        {
            for(int i=0;i<NumCell;i++)
            {
                if(needleMovement[j][2] == i+1) // Если игла находится в указанном слое,то увеличиваем на 1 число. В данном случае записывается номер ячейки с 1, поэтом прибавляем 1
                {
                    numberNeedlesOneLayer[i] = numberNeedlesOneLayer[i]+1;
                }
            }
        }
        //вычисление нагрева порошка с учетом: нагрева при прохождении тока, охлаждение водой и теплообмене между слоями. Для первой ячейки только теплообмен с нижним слоем
        temperatureChange[0] = ((numberNeedlesOneLayer[0]/(containerPar.needleNum/NumCell))*containerPar.contourIntegrityFactor*containerPar.resistanceNeedle*containerPar.needleCurrent*containerPar.needleCurrent
        -containerPar.convectiveHeatTransferCoefficient*cellSurfaceArea*(temperature[0]-containerPar.waterTemperature)
        -containerPar.powderHeatTransferCoefficient*containerPar.Square*(temperature[0]-temperature[0+1]));
        if(powderWeightContainer[0][0] > powderMassOneCell*LimBound )  //Если наша масса меньше пороговой, то заменяем её максимальной на долю. Нужно для компенсации нелинейности
        {
            temperatureChange[0] = temperatureChange[0]/(powderWeightContainer[0][0]*meanCapacity);
        }
        else
        {
            temperatureChange[0] = temperatureChange[0]/(powderMassOneCell*LimBound*meanCapacity);
        }
        temperature[0] = temperature[0] + temperatureChange[0]*h*Efficiency[0];
        for(int j=1;j<NumCell-1;j++) //вычисление нагрева порошка с учетом: нагрева при прохождении тока, охлаждение водой и теплообмене между слоями (верхним и нижним)
        {
            temperatureChange[j] = ((numberNeedlesOneLayer[j]/(containerPar.needleNum/NumCell))*containerPar.contourIntegrityFactor*containerPar.resistanceNeedle*containerPar.needleCurrent*containerPar.needleCurrent
            -containerPar.convectiveHeatTransferCoefficient*cellSurfaceArea*(temperature[j]-containerPar.waterTemperature)
            -containerPar.powderHeatTransferCoefficient*containerPar.Square*(temperature[j]-temperature[j+1])
            +containerPar.powderHeatTransferCoefficient*containerPar.Square*(temperature[j-1]-temperature[j]));
            if(powderWeightContainer[0][j] > powderMassOneCell*LimBound )  //Если наша масса меньше пороговой, то заменяем её максимальной на долю. Нужно для компенсации нелинейности
            {
                temperatureChange[j] = temperatureChange[j]/(powderWeightContainer[0][j]*meanCapacity);
            }
            else
            {
                temperatureChange[j] = temperatureChange[j]/(powderMassOneCell*LimBound*meanCapacity);
            }
            temperature[j] = temperature[j] + temperatureChange[j]*h*Efficiency[j];
        }
        //вычисление нагрева порошка с учетом: нагрева при прохождении тока, охлаждение водой и теплообмене между слоями. Для последней ячейки только теплообмен с верхним слоем
        temperatureChange[NumCell-1]= ((numberNeedlesOneLayer[NumCell-1]/(containerPar.needleNum/NumCell))*containerPar.contourIntegrityFactor*containerPar.resistanceNeedle*containerPar.needleCurrent*containerPar.needleCurrent
        -containerPar.convectiveHeatTransferCoefficient*cellSurfaceArea*(temperature[NumCell-1]-containerPar.waterTemperature)
        +containerPar.powderHeatTransferCoefficient*containerPar.Square*(temperature[NumCell-1-1]-temperature[NumCell-1]));
        if(powderWeightContainer[0][NumCell-1] > powderMassOneCell*LimBound )  //Если наша масса меньше пороговой, то заменяем её максимальной на долю. Нужно для компенсации нелинейности
        {
            temperatureChange[NumCell-1] = temperatureChange[NumCell-1]/(powderWeightContainer[0][NumCell-1]*meanCapacity);
        }
        else
        {
            temperatureChange[NumCell-1] = temperatureChange[NumCell-1]/(powderMassOneCell*LimBound*meanCapacity);
        }
        temperature[NumCell-1] = temperature[NumCell-1]+temperatureChange[NumCell-1]*h*Efficiency[NumCell-1];

        /*out->setOut(0,particleCrushing[0]);
        out->setOut(1,particleCrushing[1]);
        out->setOut(2,particleCrushing[2]);
        out->setOut(3,particleCrushing[3]);*/
        out->setOut(0,temperature[0]); //вывод значений массы на самописец
        out->setOut(1,temperature[1]);
        out->setOut(2,temperature[2]);
        out->setOut(3,temperature[3]);
        OUTInform->setOut(0,powderWeightContainer[0][0]);   // вывод значений массы на показывающий прибор
        OUTInform->setOut(1,powderWeightContainer[0][1]);
        OUTInform->setOut(2,powderWeightContainer[0][2]);
        OUTInform->setOut(3,powderWeightContainer[0][3]);
        // out->setOut(4,sqrt((containerPar.Square/PI))*0.1);
        currenNodesMix = currenNodesMix+1;  //увеличение на 1 узла расчета
    }
    if(currenNodesMix == nodesMix) //если произведено смешивание, то зануление переменных и подготовка к новому расчёту.
    {
        currenNodesMix = 0;
        contrToDisp->setOut(0,1);
        startCalc = 0;
        for(int i = 0; i < NumComp+1; ++i)
        {
            for(int j = 0; j < NumCell; ++j)
            {
                powderWeightContainer[i][j] = 0;
            }
        }
        for(int i = 0; i < NumCompCrash; ++i)
        {
            particleCrushing[i] = containerPar.particleCrushing[i];
        }
        for(int i=0;i<NumCell;i++)
        {
            temperature[i] = containerPar.temperatureStart;
        }
    }

    return true;
}


ICalcElement *Create()
{
    return new MFR_VortexLayer();
}

void Release(ICalcElement *block)
{
    if(block->type() == "MFR_VortexLayer") delete block;
}

std::string Type()
{
    return "MFR_VortexLayer";
}
