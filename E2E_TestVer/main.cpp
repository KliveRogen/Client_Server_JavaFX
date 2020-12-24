#include <QCoreApplication>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <math.h>

#define NUM_STAGES 10
#define NUM_BLOCKS 3
#define NORM_PROB 10000
using namespace std;

class emer{
public:
    double time;    //время аварии (время на устранение аварии)
    double prob;    //вероятность аварии
    bool isDefect;  //нужно ли продукцию отправлять в брак
    int type;       //тип аварии
};

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
    bool isDefectiveOut = false;
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
   // bool    nonLinkedInputs = false; // флаг несвязных входов
    int    isEmer; //Если не 0, то указывает на вид аварии
    vector<inConnectionProp> input; // входы
    vector<outConnectionProp> output; // выходы
    double  cycles = 0, // кол-во циклов
            overtime = 0, // время простоя
            emerTime = 0; // время простоя
    vector<resourseProp> res; // ресурсы
    vector<int> emerNum;
    void emerRoutine(int i);
    double  Defect = 0; // Пока что отражает величину бракованной продукции
};

stageProp stage[NUM_STAGES+1]; //+1 из-за конечной структуры где будет всё копиться
vector<emer> emers;
//alarmInterrupt alarm;

//Вектор с соотнесением узлов и линий
vector<int> stagesInBlocks[NUM_BLOCKS];

/* Функция для проверки/обработки аварий
 * currentStage - номер текущего узла, для которого осуществляется обработка*/
void emerRoutine(int currentStage)
{   //инициализация аварий
    if(stage[currentStage].isEmer==0){
        for (int k = 0; k < stage[currentStage].emerNum.size(); k++){
            int probabil = rand()%NORM_PROB;
            if(emers[k].prob == probabil){   //авария случилась?
                stage[currentStage].isEmer = emers[k].type;
                stage[currentStage].isWorking = false;
                break;
            }
        }
    }else{ //наличие аварии
        if(stage[currentStage].emerTime==emers[stage[currentStage].isEmer-1].time){
            stage[currentStage].isEmer = 0;
            stage[currentStage].emerTime = 0;
            if(emers[stage[currentStage].isEmer-1].isDefect){ //если есть брак, то отправляем в брак
               for (int i = 0; i < stage[currentStage].output.size(); i++){
                   stage[currentStage].Defect += stage[currentStage].output[i].amountProd;
               }
               stage[currentStage].worktime = 0;
            }else{
                stage[currentStage].isWorking = true;   //Если ничего в брак не надо, то почему бы не продолжить работать?
            }
        }else{
            stage[currentStage].emerTime++;
        }
    }
}

/*Функция обхода по всем входамв конкретном узле для проверки на заполнение.
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

/*Функция обхода по всем выходам в конкретном узле для проверки на недостаток
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
/*    if (stage[currentStage].nonLinkedInputs){ //проверка на наличие несвязанных входов в узле
        for (int k = 0; k < stage[currentStage].input.size(); k++){
            if (stage[currentStage].input[k].isLoaded){ //если хоть один порт заряжен, то работать
                stage[currentStage].input[k].isLoaded = false;
                stage[currentStage].isWorking = true;
                break;
            }
        }
    }else{*/ //в узле связанные порты
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
   // }
}

/*Функция подготовки запуска узла.
currentStage - номер текущего узла, который пытаемся запустить*/
void prepRoutine(int currentStage){
    if (!stage[currentStage].isWorking && !stage[currentStage].isEmer){ // если узел не работает и нет аварии, то пробуем запустить //
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

    emerRoutine(currentStage); //реализация аварий
    if (stage[currentStage].isWorking){ // если работает, то прибавляем шаг времени работы
        stage[currentStage].worktime++;
    }else{  // если не работает, то прибавляем шаг времени простоя
        stage[currentStage].overtime++;
    }
}

/*Функция выдачи продукта в текущем узле (если есть, что выдавать).
currentStage - номер текущего узла, который выдает продукт*/
void productDispensingStage(int currentStage){
    if (stage[currentStage].worktime >= stage[currentStage].cycletime){ // если отработал один цикл, то выдает продукт
        stage[currentStage].cycles++;
        stage[currentStage].isWorking = false;
        stage[currentStage].worktime = 0;
        for (int k = 0; k < stage[currentStage].output.size(); k++)
        {
            if(stage[currentStage].output[k].numStage != -1){
                stage[stage[currentStage].output[k].numStage].input[stage[currentStage].output[k].numPort].storage += stage[currentStage].output[k].amountProd;
            }
        }
    }
}

/*Функция проверки выходов (переменной amountProd) на корректность введенного значения
(оно должно быть больше 0, если это не выход брака).
Также проверяется, что при одинаковом типе продукта на входе и выходе, их разность была равна 0.
Должна использоваться сразу после инициализации структур*/
int outputsCorrectCheck(){
    //Проверка на материальный баланс
    for (int k = 0; k < NUM_STAGES; k++){
        for (int i = 0; i < stage[k].output.size(); i++){
            //проверка на равенство кол. прод. у выходов и входов одного типа
            for (int j = 0; j < stage[k].input.size();j++){
                if(stage[k].input[j].typeProd == stage[k].output[i].typeProd &&
                        stage[k].input[j].amountProd != stage[k].output[i].amountProd){
                    printf("Output %d and Input %d in Stage %d is NOT EQUAL!\n",i+1,j+1,k+1);
                    return 11;
                }
            }
            //если колич. продукта <=0 и это не брак или колич. продукта <0 и это брак, то ошибка
            if(stage[k].output[i].amountProd<=0 &&  stage[k].output[i].isDefectiveOut==false ||
                    stage[k].output[i].amountProd<0 &&  stage[k].output[i].isDefectiveOut==true){
                 printf("Output %d in Stage %d is NOT CORRECT!\n",i+1, k+1);
                 return 11;

            }else{
                printf("Output %d in Stage %d is correct\n",i+1, k+1);
            }
        }
    }
}

/*Функция индикации состояния узла и линий*/
void stageIndication(){
    int couEmersBlock=0; //счетчик аварий на линии
    int couProblemBlock=0; //счетчик неполадок на линии

    for (int numBlock = 0; numBlock < NUM_BLOCKS; numBlock++){
        for (int currentStage = 0; currentStage < stagesInBlocks[numBlock].size(); currentStage++){
            if(stage[stagesInBlocks[numBlock][currentStage]].isEmer==true && emers[(stage[stagesInBlocks[numBlock][currentStage]].isEmer)-1].isDefect==true){
                couEmersBlock++;
                //printf("\nThere was the Ermergency at the stage%d in line %d!!!", currentStage+1,numBlock+1);
            }else if (stage[stagesInBlocks[numBlock][currentStage]].isEmer==true && emers[stage[stagesInBlocks[numBlock][currentStage]].isEmer-1].isDefect==false){
                //printf("\nThere was the Problem at the stage %d in line %d", currentStage+1,numBlock+1);
                couProblemBlock++;
			}
		}
        if(couEmersBlock >= 2){
            printf("\nLine %d emergency!!!", numBlock+1);
        }else if(couEmersBlock==1){
            printf("\nLine %d problem", numBlock+1);
        }

        if(couProblemBlock > 3){
            printf("\nLine %d problem", numBlock+1);
        }
        couEmersBlock=0;
        couProblemBlock=0;
    }
}

int main()
{
    int cou = 0,
            couin = 0,
            step=0; //количество итераций в цикле //ПОТОМ УБРАТЬ
    int couNumEmer = 0;
    // CTS - карботермический
    // 1 - Участок подготовки исходных материалов
    if(1)
    {
        couin = 0;
        stage[cou].isContinuous = true; couin++;
        stage[cou].cycletime = 10; couin++;
        stage[cou].timeConst = 60*60;
        stage[cou].emerNum.push_back(int());
        stage[cou].emerNum[couNumEmer]=1; couNumEmer++;
        stage[cou].emerNum.push_back(int());
        stage[cou].emerNum[couNumEmer]=2; couNumEmer++;

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

		// 2 - Участок дозирования, смешения и грануляции
        if(1)
        {
            couin = 0;
            couNumEmer = 0;
            stage[cou].isContinuous = true; couin++;
            stage[cou].cycletime = 10; couin++;
            stage[cou].timeConst = 60*60;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=4; couNumEmer++;

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
            stage[cou].output[0].amountProd = 25; couin++;
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
		// 3 - Участок подготовки и хранения пресс-порошка
        if(1)
        {
            couin = 0;
            couNumEmer = 0;
            stage[cou].isContinuous = (0 == 0); couin++;
            stage[cou].cycletime = 10; couin++;
            stage[cou].timeConst = 60*60;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=3; couNumEmer++;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=4; couNumEmer++;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=5; couNumEmer++;

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
        // FPP - таблетки
		 // 1 -  Участок дозирования, смешения и грануляции
		if(1)
        {
			//4
            couin = 0;
            couNumEmer = 0;
            stage[cou].isContinuous = (0 == 0); couin++;
            stage[cou].cycletime = 10; couin++;
            stage[cou].timeConst = 60*60;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=3; couNumEmer++;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=4; couNumEmer++;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=5; couNumEmer++;

            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[0].numStage = cou - 1;
            stage[cou].input[0].numPort = 0;
            stage[cou].input[0].amountProd = 5; couin++;
            stage[cou].input[0].typeProd = 5;
            stage[cou].input[0].storage = 80;

            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[1].numStage = cou - 1;
            stage[cou].input[1].numPort = 1;
            stage[cou].input[1].amountProd = 6; couin++;
            stage[cou].input[1].typeProd = 6;
            stage[cou].input[1].storage = 90;

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
        // 2 - Участок подготовки и хранения пресс-порошка
		if(1)
        {
            // 5 
            couin = 0;
            couNumEmer = 0;
            stage[cou].isContinuous = (0 == 0); couin++;
            stage[cou].cycletime = 10; couin++;
            stage[cou].timeConst = 60*60;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=3; couNumEmer++;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=4; couNumEmer++;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=5; couNumEmer++;

            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[0].numStage = cou - 1;
            stage[cou].input[0].numPort = 0;
            stage[cou].input[0].amountProd = 5; couin++;
            stage[cou].input[0].typeProd = 5;
            stage[cou].input[0].storage = 80;

            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[1].numStage = cou - 1;
            stage[cou].input[1].numPort = 1;
            stage[cou].input[1].amountProd = 6; couin++;
            stage[cou].input[1].typeProd = 6;
            stage[cou].input[1].storage = 90;

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
        // 3 - Участок прессования таблеток
		if(1)
        {
            // 6
            couin = 0;
            couNumEmer = 0;
            stage[cou].isContinuous = (0 == 0); couin++;
            stage[cou].cycletime = 10; couin++;
            stage[cou].timeConst = 60*60;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=3; couNumEmer++;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=4; couNumEmer++;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=5; couNumEmer++;

            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[0].numStage = cou - 1;
            stage[cou].input[0].numPort = 0;
            stage[cou].input[0].amountProd = 5; couin++;
            stage[cou].input[0].typeProd = 5;
            stage[cou].input[0].storage = 80;

            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[1].numStage = cou - 1;
            stage[cou].input[1].numPort = 1;
            stage[cou].input[1].amountProd = 6; couin++;
            stage[cou].input[1].typeProd = 6;
            stage[cou].input[1].storage = 90;

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
        // NFEA - твэлы
		// 1 - Участок входного контроля
		if(1)
        {
            // 7
            couin = 0;
            couNumEmer = 0;
            stage[cou].isContinuous = (0 == 0); couin++;
            stage[cou].cycletime = 10; couin++;
            stage[cou].timeConst = 60*60;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=3; couNumEmer++;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=4; couNumEmer++;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=5; couNumEmer++;

            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[0].numStage = cou - 1;
            stage[cou].input[0].numPort = 0;
            stage[cou].input[0].amountProd = 5; couin++;
            stage[cou].input[0].typeProd = 5;
            stage[cou].input[0].storage = 80;

            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[1].numStage = cou - 1;
            stage[cou].input[1].numPort = 1;
            stage[cou].input[1].amountProd = 6; couin++;
            stage[cou].input[1].typeProd = 6;
            stage[cou].input[1].storage = 90;

            stage[cou].output.push_back(outConnectionProp());
            stage[cou].output[0].numStage = cou + 1;
            stage[cou].output[0].numPort = 0;
            stage[cou].output[0].amountProd = 7; couin++;
            stage[cou].output[0].typeProd = 7;
            stage[cou].output.push_back(outConnectionProp());
            stage[cou].output[1].numStage = -1;
            stage[cou].output[1].numPort = -1;
            stage[cou].output[1].isDefectiveOut = true; //этот выход с браком
            stage[cou].output[1].amountProd = 8; couin++;
            stage[cou].output[1].typeProd = 101; //БРАК

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
        // 2 - Участок сборки и герметизации ТВЭЛов
		if(1)
        {
            // 8 
            couin = 0;
            couNumEmer = 0;
            stage[cou].isContinuous = (0 == 0); couin++;
            stage[cou].cycletime = 10; couin++;
            stage[cou].timeConst = 60*60;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=3; couNumEmer++;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=4; couNumEmer++;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=5; couNumEmer++;

            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[0].numStage = cou - 1;
            stage[cou].input[0].numPort = 0;
            stage[cou].input[0].amountProd = 5; couin++;
            stage[cou].input[0].typeProd = 5;
            stage[cou].input[0].storage = 80;

            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[1].numStage = -1;
            stage[cou].input[1].numPort = -1;
            stage[cou].input[1].amountProd = 6; couin++;
            stage[cou].input[1].infSupply = true;
            stage[cou].input[1].typeProd = 6;
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
        // 3 - Участок дезактивации
		if(1)
        {
            // 9
            couin = 0;
            couNumEmer = 0;
            stage[cou].isContinuous = (0 == 0); couin++;
            stage[cou].cycletime = 10; couin++;
            stage[cou].timeConst = 60*60;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=3; couNumEmer++;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=4; couNumEmer++;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=5; couNumEmer++;

            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[0].numStage = cou - 1;
            stage[cou].input[0].numPort = 0;
            stage[cou].input[0].amountProd = 5; couin++;
            stage[cou].input[0].typeProd = 5;
            stage[cou].input[0].storage = 80;

            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[1].numStage = cou - 1;
            stage[cou].input[1].numPort = 1;
            stage[cou].input[1].amountProd = 6; couin++;
            stage[cou].input[1].typeProd = 6;
            stage[cou].input[1].storage = 90;

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
        // 4 - Участок термообработки
		if(1)
        {
            // 10
            couin = 0;
            couNumEmer = 0;
            stage[cou].isContinuous = (0 == 0); couin++;
            stage[cou].cycletime = 10; couin++;
            stage[cou].timeConst = 60*60;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=3; couNumEmer++;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=4; couNumEmer++;
            stage[cou].emerNum.push_back(int());
            stage[cou].emerNum[couNumEmer]=5; couNumEmer++;

            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[0].numStage = cou - 1;
            stage[cou].input[0].numPort = 0;
            stage[cou].input[0].amountProd = 5; couin++;
            stage[cou].input[0].typeProd = 5;
            stage[cou].input[0].storage = 80;

            stage[cou].input.push_back(inConnectionProp());
            stage[cou].input[1].numStage = cou - 1;
            stage[cou].input[1].numPort = 1;
            stage[cou].input[1].amountProd = 6; couin++;
            stage[cou].input[1].typeProd = 6;
            stage[cou].input[1].storage = 90;

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
        // end - последний участок, который содержит в себе конечный продукт
		if(1)
        {
			// 10 (+1)
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


    //Количество узлов в каждой линии
    stagesInBlocks[0].push_back(0);
    stagesInBlocks[0].push_back(1);
    stagesInBlocks[0].push_back(2);
    //
    stagesInBlocks[1].push_back(3);
    stagesInBlocks[1].push_back(4);
    stagesInBlocks[1].push_back(5);
    //
    stagesInBlocks[2].push_back(6);
    stagesInBlocks[2].push_back(7);
    stagesInBlocks[2].push_back(8);
    stagesInBlocks[2].push_back(9);
    //printf("\n eeee = %d", stagesInBlocks[1][2]);


    /* Типы аварий
     * 1) разгерметизация чего-либо
     * 2) отключение электричества
     * 3) недостаток ресурсов? (в расчете ресурсы бесконечны, ну а вдруг?)
     * 4) поломка манипулятора/конвейера (заклинивание сюда)
     * 5) остальные аварии
     */
    couNumEmer = 0;
    emers.push_back(emer()); //1
    emers[couNumEmer].time = 100;
    emers[couNumEmer].prob = 10;
    emers[couNumEmer].isDefect = 0;
    emers[couNumEmer].type = 1;
    emers.push_back(emer()); couNumEmer++; //2
    emers[couNumEmer].time = 200;
    emers[couNumEmer].prob = 9;
    emers[couNumEmer].isDefect = 1;
    emers[couNumEmer].type = 2;
    emers.push_back(emer()); couNumEmer++; //3
    emers[couNumEmer].time = 300;
    emers[couNumEmer].prob = 8;
    emers[couNumEmer].isDefect = 0;
    emers[couNumEmer].type = 3;
    emers.push_back(emer()); couNumEmer++; //4
    emers[couNumEmer].time = 400;
    emers[couNumEmer].prob = 7;
    emers[couNumEmer].isDefect = 1;
    emers[couNumEmer].type = 4;
    emers.push_back(emer()); couNumEmer++; //5
    emers[couNumEmer].time = 500;
    emers[couNumEmer].prob = 6;
    emers[couNumEmer].isDefect = 0;
    emers[couNumEmer].type = 5;

/* //проверка на сохранение массы 

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
*/

    //Проверка материального баланса
    outputsCorrectCheck();

    //основной цикл программы
    while(step!=360000)
    {
        for (int num = 0; num < NUM_STAGES; num++){ // обход по всем узлам
            prepRoutine(num);//Если узел NUM не работает, то пробуем его запустить
            runOrDownTimeStage(num); //Определение времени работы или простоя узла NUM
            productDispensingStage(num);//Выдача продукта в узле NUM (если есть, что выдывать)
        }
        stageIndication(); //функция индикакции состояний узлов и линий


        step +=1;
    }
    //printf("\nStep = %d", step);
    return 0;
}

