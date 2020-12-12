#include <QCoreApplication>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <math.h>

#define NUM_STAGES 3
using namespace std;
/*
struct alarmInterrupt {
    bool isAlarm = true;false;
    double  cycletime = 3600; // брать из средней продолжительности аварий
    double  timeConst = 1;
    double  worktime = 0;
    int prob = 2;
};
*/
struct inConnectionProp {
    int numStage = -1;
    int numPort = -1;
    int typeProd = -1;
    double amountProd = 0;
    bool isLoaded = false;
    bool infSupply = false;
    double storage = 0;
};
struct outConnectionProp {
    int numStage = -1;
    int numPort = -1;
    int typeProd = -1;
    double amountProd = 0;    
};
struct resourseProp {
    int type = -1;
    double value = 0;
};

struct stageProp { //структура узла/участка
    double  worktime = 0; // время работы
    double  cycletime = 0; // время одного цикла
    double  timeConst = 1; // постоянная времени перевод в секунды
    bool    isWorking = false; // флаг работы
    bool    isContinuous = false; // флаг нпр работы
    bool    nonLinkedInputs = false; // флаг несвязных входов
    vector<inConnectionProp> input; // входы
    vector<outConnectionProp> output; // выходы
    double  cycles = 0, // кол-во циклов
    overtime = 0; // время простоя
    vector<resourseProp> res; // ресурсы
};

stageProp stage[NUM_STAGES+1]; //+1 из-за конечной структуры где будет всё копиться
//alarmInterrupt alarm;

/*Функция обхода по всем входам в конкретном узле для проверки на заполнение.
currentStage - номер текущего узла, для которого осуществляется обход*/
void inputLoading(int currentStage){
    for (int k = 0; k < stage[currentStage].input.size(); k++){ // бесконечный саплай?  *Можно ли просто это "вынести" и держать бесконечный саплай ?
        if (stage[currentStage].input[k].infSupply){
            stage[currentStage].input[k].isLoaded = true;
        }else{ // нужно сделать запрос за заполнение порта
            if ((stage[currentStage].input[k].storage >= stage[currentStage].input[k].amountProd) &&
                (!stage[currentStage].input[k].isLoaded)){ // если на складе хватает ресурсов, и вход уже не заряжен
                stage[currentStage].input[k].storage -= stage[currentStage].input[k].amountProd;
                stage[currentStage].input[k].isLoaded = true;
            }
        }
    }
}

/*Функция обхода по всем входам в конкретном узле для проверки на недостаток
currentStage - номер текущего узла, для которого осуществляется обход*/
bool isOutputNeeded(int currentStage){
    for (int k = 0; k < stage[currentStage].input.size(); k++){
        if ((stage[currentStage].input[k].amountProd > stage[currentStage].input[k].storage) &&
            (!stage[currentStage].input[k].isLoaded)){ // если на складе не хватает ресурсов для снабжения хоть одного следующего
            return true; // есть недостаток
        }
    }
    return false; // нет недостатка
}

/*Функция попытки запуска узла
currentStage - номер текущего узла, для которого осуществляется проверка*/
void tryToRunStage(int currentStage){
    if (stage[currentStage].nonLinkedInputs){ //проверка на наличие несвязанных входов в узле
        for (int k = 0; k < stage[currentStage].input.size(); k++){
            if (stage[currentStage].input[k].isLoaded){ //если хоть один порт заряжен, то работать
                stage[currentStage].input[k].isLoaded = false;
                stage[currentStage].isWorking = true;
                break;
            }
        }
    }else{ // если в узле связанные порты
        stage[currentStage].isWorking = true;
        for (int k = 0; k < stage[currentStage].input.size(); k++){
            if (!stage[currentStage].input[k].isLoaded){ //если хоть один порт не заряжен, то не работать
                stage[currentStage].isWorking = false;
                break;
            }
        }
        if ( stage[currentStage].isWorking ){ // если удалось запустить, то отчищаем загрузку входов
            for (int k = 0; k < stage[currentStage].input.size(); k++){
                stage[currentStage].input[k].isLoaded = false;
            }
        }
    }
}

/*Функция подготовки запуска узла.
currentStage - номер текущего узла, который пытаемся запустить*/
void prepRoutine(int currentStage){
    if (!stage[currentStage].isWorking){ // если узел не работает, то пробуем запустить //
        // проверка на заполнение входов узла currentStage
        inputLoading(currentStage);
        //попытка запуска узла
        if (stage[currentStage].isContinuous || isOutputNeeded(currentStage)){ // если установлена непрерывная работа
            tryToRunStage(currentStage);
        }
    }
}

/*Функция для расчета рабочего времени или времени простоя узла.
currentStage - номер текущего узла, для которого осущесвляется расчет*/
void runOrDownTimeStage(int currentStage){
    if (stage[currentStage].isWorking){ // если работает, то прибавляем шаг времени работы
        stage[currentStage].worktime++;
    }else{  // если не работает, то прибавляем шаг времени простоя
        stage[currentStage].overtime++;
    }
}

/*Функция выдачи продукта в следующий узел из текущего (если есть, что выдавать).
currentStage - номер текущего узла, который выдает продукт*/
void productDispensingStage(int currentStage){
    if (stage[currentStage].worktime >= stage[currentStage].cycletime){ // если отработал один цикл, то выдает продукт
        stage[currentStage].cycles++;
        stage[currentStage].isWorking = false;
        stage[currentStage].worktime = 0;
        for (int k = 0; k < stage[currentStage].output.size(); k++)
        {
            stage[stage[currentStage].output[k].numStage].input[stage[currentStage].output[k].numPort].storage += stage[currentStage].output[k].amountProd;
        }
    }
}

int main()
{
    int cou = 0,
            couin = 0,
            step=0; //количество итераций в цикле //ПОТОМ УБРАТЬ
    // CTS - карботермический
    // 1 - Участок подготовки исходных материалов
    if(1)
    {
        couin = 0;
        stage[cou].isContinuous = true; couin++;
        stage[cou].cycletime = 10; couin++;
        stage[cou].timeConst = 60*60;

        stage[cou].input.push_back(inConnectionProp());
        stage[cou].input[0].numStage = -1;
        stage[cou].input[0].numPort = -1;
        stage[cou].input[0].amountProd = 1; couin++;
        stage[cou].input[0].typeProd = 1;
        stage[cou].input[0].infSupply = true;
        stage[cou].input[0].storage = -1;

        stage[cou].input.push_back(inConnectionProp());
        stage[cou].input[1].numStage = -1;
        stage[cou].input[1].numPort = -1;
        stage[cou].input[1].amountProd = 2; couin++;
        stage[cou].input[1].typeProd = 2;
        stage[cou].input[1].infSupply = true;
        stage[cou].input[1].storage = -1;

        stage[cou].input.push_back(inConnectionProp());
        stage[cou].input[2].numStage = -1;
        stage[cou].input[2].numPort = -1;
        stage[cou].input[2].amountProd = 3; couin++;
        stage[cou].input[2].typeProd = 3;
        stage[cou].input[2].infSupply = true;
        stage[cou].input[2].storage = -1;

        stage[cou].output.push_back(outConnectionProp());
        stage[cou].output[0].numStage = cou + 1;
        stage[cou].output[0].numPort = 0;
        stage[cou].output[0].amountProd = 1; couin++;
        stage[cou].output[0].typeProd = 1;

        stage[cou].output.push_back(outConnectionProp());
        stage[cou].output[1].numStage = cou + 1;
        stage[cou].output[1].numPort = 1;
        stage[cou].output[1].amountProd = 2; couin++;
        stage[cou].output[1].typeProd = 2;

        stage[cou].output.push_back(outConnectionProp());
        stage[cou].output[2].numStage = cou + 1;
        stage[cou].output[2].numPort = 2;
        stage[cou].output[2].amountProd = 3; couin++;
        stage[cou].output[2].typeProd = 3;

        stage[cou].res.push_back(resourseProp());
        stage[cou].res[0].value =10; couin++;
        stage[cou].res[0].type = 1;
        stage[cou].res.push_back(resourseProp());
        stage[cou].res[1].value = 10; couin++;
        stage[cou].res[1].type = 2;
        stage[cou].res.push_back(resourseProp());
        stage[cou].res[2].value = 10; couin++;
        stage[cou].res[2].type = 6;
        stage[cou].res.push_back(resourseProp());
        stage[cou].res[3].value = 10; couin++;
        stage[cou].res[3].type = 4;
        cou++;
        if(1)
        {
            // 2 - Участок дозирования, смешения и грануляции
            couin = 0;
            stage[cou].isContinuous = true; couin++;
            stage[cou].cycletime = 10; couin++;
            stage[cou].timeConst = 60*60;
            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[0].numStage = cou - 1;
            stage[cou].input[0].numPort = 0;
            stage[cou].input[0].amountProd = 1; couin++;
            stage[cou].input[0].typeProd = 1;
            stage[cou].input[0].storage = 50;

            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[1].numStage = cou - 1;
            stage[cou].input[1].numPort = 1;
            stage[cou].input[1].amountProd = 2; couin++;
            stage[cou].input[1].typeProd = 2;
            stage[cou].input[1].storage = 60;

            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[2].numStage = cou - 1;
            stage[cou].input[2].numPort = 2;
            stage[cou].input[2].amountProd = 3; couin++;
            stage[cou].input[2].typeProd = 3;
            stage[cou].input[2].storage = 70;

            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[3].numStage = -1;
            stage[cou].input[3].numPort = - 1;
            stage[cou].input[3].amountProd = 4; couin++;
            stage[cou].input[3].typeProd = 4;
            stage[cou].input[3].infSupply = true;
            stage[cou].input[3].storage = -1;

            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[4].numStage = -1;
            stage[cou].input[4].numPort = - 1;
            stage[cou].input[4].amountProd = 4; couin++;
            stage[cou].input[4].typeProd = 4;
            stage[cou].input[4].infSupply = true;
            stage[cou].input[4].storage = -1;

            stage[cou].output.push_back(outConnectionProp());
            stage[cou].output[0].numStage = cou + 1;
            stage[cou].output[0].numPort = 0;
            stage[cou].output[0].amountProd = 10; couin++;
            stage[cou].output[0].typeProd = 5;

            stage[cou].res.push_back(resourseProp());
            stage[cou].res[0].value = 10; couin++;
            stage[cou].res[0].type = 1;
            stage[cou].res.push_back(resourseProp());
            stage[cou].res[1].value = 10; couin++;
            stage[cou].res[1].type = 6;
            stage[cou].res.push_back(resourseProp());
            stage[cou].res[2].value = 10; couin++;
            stage[cou].res[2].type = 8;
            stage[cou].res.push_back(resourseProp());
            stage[cou].res[3].value = 10; couin++;
            stage[cou].res[3].type = 4;
            stage[cou].res.push_back(resourseProp());
            stage[cou].res[4].value = 10; couin++;
            stage[cou].res[4].type = 4;
            cou++;
        }
        if(1)
        {
            // 3 - Участок подготовки и хранения пресс-порошка
            couin = 0;
            stage[cou].isContinuous = (0 == 0); couin++;
            stage[cou].cycletime = 10; couin++;
            stage[cou].timeConst = 60*60;
            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[0].numStage = cou - 1;
            stage[cou].input[0].numPort = 0;
            stage[cou].input[0].amountProd = 5; couin++;
            stage[cou].input[0].typeProd = 5;
            stage[cou].input[0].storage = 80;

            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[1].numStage = -1;
            stage[cou].input[1].numPort = - 1;
            stage[cou].input[1].amountProd = 6; couin++;
            stage[cou].input[1].typeProd = 6;
            stage[cou].input[1].infSupply = true;
            stage[cou].input[1].storage = -1;

            stage[cou].output.push_back(outConnectionProp());
            stage[cou].output[0].numStage = cou + 1;
            stage[cou].output[0].numPort = 0;
            stage[cou].output[0].amountProd = 7; couin++;
            stage[cou].output[0].typeProd = 7;
            stage[cou].output.push_back(outConnectionProp());
            stage[cou].output[1].numStage = cou + 1;
            stage[cou].output[1].numPort = 1;
            stage[cou].output[1].amountProd = 8; couin++;
            stage[cou].output[1].typeProd = 8;

            stage[cou].res.push_back(resourseProp());
            stage[cou].res[0].value = 10; couin++;
            stage[cou].res[0].type = 1;
            stage[cou].res.push_back(resourseProp());
            stage[cou].res[1].value = 10; couin++;
            stage[cou].res[1].type = 4;
            stage[cou].res.push_back(resourseProp());
            stage[cou].res[2].value = 10; couin++;
            stage[cou].res[2].type = 4;
            cou++;
        }
        if(1)
        {
            // 3 - Участок подготовки и хранения пресс-порошка
            couin = 0;
            stage[cou].isContinuous = (0 == 0); couin++;
            stage[cou].cycletime = 0; couin++;
            stage[cou].timeConst = 0;
            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[0].numStage = cou - 1;
            stage[cou].input[0].numPort = 0;
            stage[cou].input[0].typeProd = 7;
            stage[cou].input[0].storage = 0;

            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[1].numStage = cou - 1;
            stage[cou].input[1].numPort = 1;
            stage[cou].input[1].typeProd = 8;
            stage[cou].input[1].storage = 0;
        }
    }

    //провекра на сохранение массы (потом через векторы будет сделано)
    //для примера, обозначение типов базовых компонентов: 1 - U02, 2 - PuO2, 3 - стеарат Zn, 4 - C
    const unsigned int numTypes = 8;
    const unsigned int baseTypes = 4;
    double massBalanceMatrix[numTypes][baseTypes] = {
        {1, 0, 0, 0},   //тип 1
        {0, 1, 0, 0},   //2
        {0, 0, 1, 0},   //3
        {0, 0, 0, 1},   //4
        {0.3, 0.44, 0.1, 0},  //5
        {0, 0, 0, 0.7},   //6
        {0.1, 0.3, 0, 0},   //7
        {0, 0, 0.1, 0.2}   //8
    };


    for (int k = 0; k < NUM_STAGES; k++){
        double sumIn=0, sumOut=0;
        for (int i = 0; i < stage[k].input.size(); i++){
            for (int j = 0; j < baseTypes; j++){
                sumIn+= stage[k].input[i].amountProd * massBalanceMatrix[stage[k].input[i].typeProd-1][j];
            }
        }
        for (int i = 0; i < stage[k].output.size(); i++){
            for (int j = 0; j < baseTypes; j++){
                sumOut+= stage[k].output[i].amountProd * massBalanceMatrix[stage[k].output[i].typeProd-1][j];
            }
        }
        if(sumIn < sumOut){
            printf("ALARM stage");
            printf(" %d\n", k+1);
            return 11;
        }else{
            printf("NEalarm stage");
            printf(" %d\n", k+1);
        }

    }

    //основной цикл программы
    while(step!=360000)
    {
        /*ДОРАБОТАТЬ аварии после выполнения других задач
    int Xex;
    //int chance;
    chance = rand();
    Xex = chance%100;
    if((Xex <= alarm.prob) && (!alarm.isAlarm)){
        alarm.isAlarm = true;
    }
    if(alarm.isAlarm){
        switch ((int)alarm.worktime){ // стоит ли затраченного времени?
            case 0:
                //грамотное обнуление + часть в брак или нет?
                alarm.worktime++;
                break;
            case (3600):
                alarm.isAlarm = false;
                alarm.worktime = 0;
                printf("%d \n",step);
               // stage[l].overtime++;

                break;
            default:
                alarm.worktime++;
                break;
        }
    }else{
//сюда основной расчет//
    }*/
        for (int num = 0; num < NUM_STAGES; num++){ // обход по всем узлам
            prepRoutine(num);//Если узел NUM не работает, то пробуем его запустить
            runOrDownTimeStage(num); //Определение времени работы или простоя узла NUM
            productDispensingStage(num);//Выдача продукта в узле NUM (если есть, что выдывать)
        }
        step +=1;
    }
    //printf("\nStep = %d", step);
    return 0;
}

