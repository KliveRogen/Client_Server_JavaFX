#include "MP_VOL_VOLOX.h"
#include <math.h>

MP_VOL_VOLOX::MP_VOL_VOLOX()
{
	// Расчетный тип блока
    BlockCalcType = E_NEEDINPUT;

    // Параметры блока
    // Реторта
	createParameter("L", "3");
	createParameter("D", "0.18");
	createParameter("theta", "2");
	createParameter("mu", "0.0088");
	createParameter("Tret", "650");
    // ТВЭЛ
	createParameter("l", "36");
	createParameter("Num_tabl", "3");
	createParameter("h_tabl", "12");
	createParameter("d_tabl", "8.7");
    // Неокисл. ОЯТ
	createParameter("rho_INF", "11.5");
	createParameter("M_INF", "252");
	createParameter("delta_UN", "0.8");
	createParameter("M_UN", "252");
	createParameter("rho_MUPNF", "12.3");
    // Газ
    createParameter("rho_G", "0.45");
	createParameter("M_G", "30");
	createParameter("Cair", "1.081");
	createParameter("Tw", "723");
	createParameter("delta_O2", "0.2");
	createParameter("rho_G_st", "0.5");
    // Кинетич., т/динамич. параметры
	createParameter("k", "85");
	createParameter("Q1", "2695");
	createParameter("Q2", "730");
	createParameter("beta", "0.94");

	// Сигналы блока

	// Порты блока
    InPortQs = createInputPort(0, "UNKNOWN_NAME", "INFO");
    InPortOmega = createInputPort(1, "UNKNOWN_NAME", "INFO");
    InPortQG = createInputPort(2, "UNKNOWN_NAME", "INFO");
    OutPortDO2 = createOutputPort(3, "UNKNOWN_NAME", "INFO");
    OutPortV = createOutputPort(4, "UNKNOWN_NAME", "INFO");
    OutPortT = createOutputPort(5, "UNKNOWN_NAME", "INFO");

	// Отказы блока

}

void MP_VOL_VOLOX::setDataNames()
{
    std::vector<std::string> dnDO2;
    dnDO2.push_back("Массовая доля O2 в газе (т.1)");
    dnDO2.push_back("Массовая доля O2 в газе (т.2)");
    dnDO2.push_back("Массовая доля O2 в газе (т.3)");
    dnDO2.push_back("Массовая доля O2 в газе (т.4)");
    dnDO2.push_back("Массовая доля O2 в газе (т.5)");
    dnDO2.push_back("Массовая доля O2 в газе (т.6)");
    OutPortDO2->setDataNames(dnDO2);

    std::vector<std::string> dnV;
    dnV.push_back("Объем неокисленного ОЯТ, см3 (т.1)");
    dnV.push_back("Объем неокисленного ОЯТ, см3 (т.2)");
    dnV.push_back("Объем неокисленного ОЯТ, см3 (т.3)");
    dnV.push_back("Объем неокисленного ОЯТ, см3 (т.4)");
    dnV.push_back("Объем неокисленного ОЯТ, см3 (т.5)");
    dnV.push_back("Объем неокисленного ОЯТ, см3 (т.6)");
    OutPortV->setDataNames(dnV);

    std::vector<std::string> dnT;
    dnT.push_back("Температура, К (т.1)");
    dnT.push_back("Температура, К (т.2)");
    dnT.push_back("Температура, К (т.3)");
    dnT.push_back("Температура, К (т.4)");
    dnT.push_back("Температура, К (т.5)");
    dnT.push_back("Температура, К (т.6)");
    OutPortT->setDataNames(dnT);
}

bool checkParamDiapason(double val, double min, double max)
{
    if ( (val < min) || (val > max) )
    {
        return false;
    }
    return true;
}

bool MP_VOL_VOLOX::init(std::string &error, double h)
{
    setDataNames();

    pi = M_PI;
    // Загрузка значений из параметров.
    // TODO: Сделать проверку параметров на принадлежность диапозону
    // Диапазон зашить в параметры невидимые пользователю??

    // %%%%%%%%%%%%%%   Параметры реторты   %%%%%%%%%%%%%%%%%%%%%%
    L = paramToDouble("L"); // Длина реторты, м
    if (!checkParamDiapason(L, 2.5, 3.5))
    {
        error = "Ошибка!\n Значение параметра \"Длина реторты\" вне диапазона!";
        return false;
    }

    D = paramToDouble("D"); // Диаметр реторты, м
    if (!checkParamDiapason(D, 0.15, 0.2))
    {
        error = "Ошибка!\n Значение параметра \"Диаметр реторты\" вне диапазона!";
        return false;
    }

    theta = paramToDouble("theta"); // Угол наклона реторты, градусы
    if (!checkParamDiapason(theta, 1, 3))
    {
        error = "Ошибка!\n Значение параметра \"Угол наклона реторты\" вне диапазона!";
        return false;
    }
    theta = theta*pi/180;

    mu = paramToDouble("mu"); // коэффициент трения ОЯТ о реторту
    if (!checkParamDiapason(mu, 0.0079, 0.0097))
    {
        error = "Ошибка!\n Значение параметра \"К-т трения фрагментов ТВЭЛов о стенку реторты\" вне диапазона!";
        return false;
    }

    Tret = paramToDouble("Tret"); // Начальная температура стенки реторты, К
    if (!checkParamDiapason(Tret, 550, 680))
    {
        error = "Ошибка!\n Значение параметра \"Начальная температура стенки\" вне диапазона!";
        return false;
    }

    // %%%%%%%%%%%%%   Параметры модели   %%%%%%%%%%%%%%%%%%%%%%%%
    Nx = 5; // Кол-во шагов по координате
    dx = round(L/Nx); // Шаг моделирования по координате, м

    // %%%%%%%%%%%%%   Параметры ТВЭЛа   %%%%%%%%%%%%%%%%%%%%%%%%
    l = paramToDouble("l"); // Длина фрагмента ТВЭЛа, мм
    if (!checkParamDiapason(l, 25, 45))
    {
        error = "Ошибка!\n Значение параметра \"Длина фрагмента ТВЭЛа\" вне диапазона!";
        return false;
    }
    l = l*0.001;

    Num_tabl = paramToDouble("Num_tabl"); // Среднее число таблеток во фрагменте ТВЭЛа
    if (!checkParamDiapason(Num_tabl, 1, 4))
    {
        error = "Ошибка!\n Значение параметра \"Среднее число таблеток в ТВЭЛе\" вне диапазона!";
        return false;
    }

    h_tabl = paramToDouble("h_tabl"); // Высота таблетки, мм;
    if (!checkParamDiapason(h_tabl, 10, 14))
    {
        error = "Ошибка!\n Значение параметра \"Высота таблетки\" вне диапазона!";
        return false;
    }
    h_tabl=h_tabl*0.001;

    d_tabl = paramToDouble("d_tabl"); // Диаметр таблетки, мм
    if (!checkParamDiapason(d_tabl, 8, 9))
    {
        error = "Ошибка!\n Значение параметра \"Диаметр таблетки\" вне диапазона!";
        return false;
    }
    d_tabl=d_tabl*0.001;

    // %%%%%%%%%%%%%   Параметры ОЯТ   %%%%%%%%%%%%%%%%%%%%%%%%
    rho_INF = paramToDouble("rho_INF"); // плотность (г/см3) неокисленного ОЯТ (Irradiated Nuclear Fuel)
    if (!checkParamDiapason(rho_INF, 11.2, 11.8))
    {
        error = "Ошибка!\n Значение параметра \"Плотность неокисленного ОЯТ\" вне диапазона!";
        return false;
    }
    rho_INF=rho_INF*1000;

    M_INF = paramToDouble("M_INF"); // Молярная масса неокисленного ОЯТ, г/моль
    if (!checkParamDiapason(M_INF, 240, 265))
    {
        error = "Ошибка!\n Значение параметра \"Молярная масса неокисленного ОЯТ\" вне диапазона!";
        return false;
    }
    M_INF=M_INF*0.001;

    delta_UN = paramToDouble("delta_UN"); // массовая доля молекул UN в неокисленном ОЯТ
    if (!checkParamDiapason(delta_UN, 0.79, 0.82))
    {
        error = "Ошибка!\n Значение параметра \"Массовая доля молекул UN в неокисленном ОЯТ\" вне диапазона!";
        return false;
    }

    M_UN = paramToDouble("M_UN"); // Молярная масса UN, г/моль
    if (!checkParamDiapason(M_UN, 251, 253))
    {
        error = "Ошибка!\n Значение параметра \"Молярная масса UN\" вне диапазона!";
        return false;
    }
    M_UN=M_UN*0.001;

    rho_MUPNF = paramToDouble("rho_MUPNF"); // плотность (г/см3) эталонного СНУП-топлива (Mixed Uranium-Plutonium Nitride Fuel)
    if (!checkParamDiapason(rho_MUPNF, 12.1, 12.8))
    {
        error = "Ошибка!\n Значение параметра \"Плотность эталонного СНУП-топлива\" вне диапазона!";
        return false;
    }
    rho_MUPNF=rho_MUPNF*1000;

    // %%%%%%%%%%%%%   Параметры газа   %%%%%%%%%%%%%%%%%%%%%%%%
    rho_G = paramToDouble("rho_G"); // плотность (кг/м3) газа, подаваемого в волоксидатор
    if (!checkParamDiapason(rho_G, 0.43, 0.47))
    {
        error = "Ошибка!\n Значение параметра \"Плотность технологического газа\" вне диапазона!";
        return false;
    }

    M_G = paramToDouble("M_G"); // Молярная масса газа, г/моль
    if (!checkParamDiapason(M_G, 25, 35))
    {
        error = "Ошибка!\n Значение параметра \"Молярная масса технологического газа\" вне диапазона!";
        return false;
    }
    M_G=M_G*0.001;

    Cair = paramToDouble("Cair"); // Средняя удельная теплоемкость О2, кДж/(кг К)
    if (!checkParamDiapason(Cair, 1, 1.1))
    {
        error = "Ошибка!\n Значение параметра \"Средняя удельная теплоемкость технологического газа\" вне диапазона!";
        return false;
    }
    Cair=Cair*1000;

    Tw = paramToDouble("Tw"); // Нач. темп-ра газа, К
    if (!checkParamDiapason(Tw, 700, 780))
    {
        error = "Ошибка!\n Значение параметра \"Начальная температура технологического газа\" вне диапазона!";
        return false;
    }
    delta_O2 = paramToDouble("delta_O2"); // массовая доля О2 в газовом потоке;
    if (!checkParamDiapason(delta_O2, 0.19, 0.22))
    {
        error = "Ошибка!\n Значение параметра \"Массовая доля O2 в технологическом газе\" вне диапазона!";
        return false;
    }

    rho_G_st = paramToDouble("rho_G_st"); // плотность (кг/м3) эталонного газа, подаваемого в волоксидатор
    if (!checkParamDiapason(rho_G_st, 0.49, 0.52))
    {
        error = "Ошибка!\n Значение параметра \"Плотность эталонного газа\" вне диапазона!";
        return false;
    }

    // %%%%%%%%%%%%%   Кинетич., т/динамич. параметры   %%%%%%%%%%%%%%%%%%%%%%%%

    k = paramToDouble("k"); // Константа скорости волоксидации, см/ч
    if (!checkParamDiapason(k, 60, 110))
    {
        error = "Ошибка!\n Значение параметра \"Константа скорости реакции волоксидации\" вне диапазона!";
        return false;
    }
    k=k*0.01/3600;

    Q1 = paramToDouble("Q1"); // 2677 %Тепловой эффект реакции UN+O2, кДж
    if (!checkParamDiapason(Q1, 2677, 2713))
    {
        error = "Ошибка!\n Значение параметра \"Тепловой эффект реакции UN c О2\" вне диапазона!";
        return false;
    }
    Q1=Q1*1000;

    Q2 = paramToDouble("Q2"); // Тепловой эффект реакции PuN+O2, кДж
    if (!checkParamDiapason(Q2, 710.4, 740))
    {
        error = "Ошибка!\n Значение параметра \"Тепловой эффект реакции PuN c О2\" вне диапазона!";
        return false;
    }
    Q2=Q2*1000;

    beta = paramToDouble("beta"); // Коэффициент теплоотдачи, Вт/(м2 К)
    if (!checkParamDiapason(beta, 0.93, 0.95))
    {
        error = "Ошибка!\n Значение параметра \"Коэффициент теплоотдачи технологического газа\" вне диапазона!";
        return false;
    }

    if (l < Num_tabl * h_tabl)
    {
        error = "Общий размер таблеток превышает размер фрагмента ТВЭЛа";
        return false;
    }

    return true;
}

bool MP_VOL_VOLOX::process(double t, double h, std::string &error)
{
    dt = h;
    // TODO: Проверка, изменял ли кто параметры, изменять которые не должно!!!
    // checkConstParametrs();

    // TODO: Обновить параметры, которые можно изменять
    // TODO: Проверять наличие входов и корректность входных значений(диапазон)!!!

    double qs = InPortQs->getInput().at(0); // Расход фрагментов ТВЭЛов, шт/ч
    if (!checkParamDiapason(qs, 25, 35))
    {
        error = "Ошибка!\n Значение входа \"Расход фрагментов ТВЭЛов\" вне диапазона!";
        return false;
    }
    qs=qs/3600;

    double omega = InPortOmega->getInput().at(0); // Частота вращения реторты, об/мин
    if (!checkParamDiapason(omega, 0.01, 0.05))
    {
        error = "Ошибка!\n Значение входа \"Частота вращения реторты\" вне диапазона!";
        return false;
    }
    omega=(omega/60)*2*pi;

    double qG = InPortQG->getInput().at(0);  // расход газа, м3/ч
    if (!checkParamDiapason(qG, 10, 20))
    {
        error = "Ошибка!\n Значение входа \"Расход газа\" вне диапазона!";
        return false;
    }
    qG=qG/3600;

    //------------------------------------
    double dbg = (3*(d_tabl*d_tabl)*h_tabl/2);
    double ao = pow( (3*(d_tabl*d_tabl)*h_tabl/2) , 1.0/3.0); // характерный размер таблетки
    double S = pi*(ao*ao)*Num_tabl; // Площадь контакта газа с ОЯТ в одном фрагменте ТВЭЛа, м2
    double S1 = (pi*D*D/4)*0.99; // Площадь сечения реторты, свободная от ТВЭЛов, м2

    double vo = theta*(D/2)*omega/mu; // Скорость движения фрагментов ТВЭЛов
    double uo = 4*qG/(pi*D*D); // Линейная скорость движения газа

    double gamma=(1+delta_UN/3)*(l*qs/vo)*(S/(l*S1))*k*(delta_UN*delta_UN)*(pow((rho_INF/rho_MUPNF),3))*(pow((rho_G/rho_G_st),3) ) * (M_G/M_INF)*(rho_INF/rho_G_st);

    double alpha=k*(pow(delta_UN,2))*( pow((rho_INF/rho_MUPNF),3) )*( pow((rho_G/rho_G_st),4) );

    // FIXME: Разобраться с инициализацией вектора, исправить на решение без циклов
    std::vector<double> d_O2, V, T, Coordinate;
    d_O2.resize(Nx+1);
    V.resize(Nx+1);
    T.resize(Nx+1);
    Coordinate.resize(Nx+1);
    for (int i = 0; i < Nx+1; i++)
    {
        d_O2[i] = 0;
        V[i] = 0;
        T[i] = Tret;
        Coordinate[i] = i*dx;
    }

    // NOTE: Осталось от матлаба, удалить, если не понадобится
    // Time=zeros(Nt+1,1);

    //Концентрация в конце аппарата
    d_O2[Nx] = delta_O2;

    // Объем неокисленного ОЯТ во фрагментах твэлов в начале аппарата
    double Vo = (pi*(d_tabl*d_tabl)/4)*h_tabl*Num_tabl; // Начальный объем неокисленного ОЯТ во фрагменте ТВЭЛа, см3
    V[0] = Vo;

    if (t==0)
    {
        for (int i = 0; i < Nx+1; i++)
        {
            // Массив объемов неокисленного ОЯТ по аппарату при t=0
            if (i!=0) V[i] = Vo/i;

            // Распределение концентраци О2 по аппарату при t=0
            d_O2[i] = delta_O2;
        }
    }

    // Температура в конце аппарата (со стороны подачи газа)
    T[Nx] = Tw;

    double popr1=1;
    double popr2=1;

    // Вектора для следующего шага. Топорно, но должно работать. Стоит ли переделывать в d_O2[2][Nx+1]???
    // NOTE: Изучить std::vector, как от инициализируется, присваивается и прочее

    // FIXME: вынести необходимые части в init();
    if (t == 0)
    {
        nd_O2.resize(Nx+1);
        nT.resize(Nx+1);
        nV.resize(Nx+1);

        nV = V;
        nT = T;
        nd_O2 = d_O2;
    }

    V = nV;
    T = nT;
    d_O2 = nd_O2;

    for (int i = 0; i < Nx; i++)
    {
        //d_O2(it+1,ix) = (uo*(d_O2(it,ix+1)-d_O2(it,ix))/dx-popr1*gamma*(d_O2(it,ix))^4)*dt+d_O2(it,ix);
        nd_O2[i] = (uo*(d_O2[i+1]-d_O2[i])/dx-popr1*gamma*pow((d_O2[i]),4))*dt+d_O2[i];
        //V(it+1,ix+1)=(vo*((V(it,ix)-V(it,ix+1))/dx)-popr2*alpha*S*(d_O2(it,ix))^4)*dt+V(it,ix+1);
        nV[i+1] = (vo*((V[i]-V[i+1])/dx)-popr2*alpha*S* (pow(d_O2[i], 4))) *dt+V[i+1];

        if (nV[i+1]<0) nV[i+1] = 0;
    }

    for (int i = Nx-1; i >= 0; i--)
    {
        double C2P1 = (rho_INF*(Q1*delta_UN+Q2*(1-delta_UN))*alpha*pi*qs*Num_tabl*ao*ao*pow(d_O2[i],4));
        double C2P2 = (M_UN*uo*vo*Cair*rho_G*S1);
        double C2 = C2P1/C2P2;
        double C3 = (2*pi*0.5*D*beta*Tw)/(uo*Cair*rho_G*S1);
        T[i] = T[i+1]-(-C2*(pow((1-(i)*dx/L),2))*(pow((d_O2[i]/d_O2[Nx-1]),4))+C3*(T[i+1]-Tw)/Tw)*dx;
    }

    // FIXME: Возможно поплывут точки, так как где-то считается h+1, где-то h
    nT = T;
    for (int i = 0; i < Nx+1; i++)
    {
        V[i] = V[i] * 1000000;
    }
    OutPortDO2->setOut(d_O2);
    OutPortT->setOut(T);
    OutPortV->setOut(V);

    return true;
}


ICalcElement *Create()
{
    return new MP_VOL_VOLOX();
}

void Release(ICalcElement *block)
{
    if(block->type() == "MP_VOL_VOLOX") delete block;
}

std::string Type()
{
    return "MP_VOL_VOLOX";
}
