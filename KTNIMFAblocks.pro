TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = Port \
    CalcElement \
    DynMath \
    Const \
    InertionOne \
    Amplifier \
    Differentiation \
    Integration \
    LimitTop \
    LimitBottom \
    LimitTopBottom \
    Multiconst \
    Step \
    #EX_EXTRACTOR \
    #EX_EXTRACTOR_2_4 \
    #EX_PUMP \
    #EX_RECEIVER \
    #EX_TANK_PUMP_VALVE \
    Transporter \
    SignalTester \
    CMN_NormallyOpenContact \
    CMN_PIDController \
    CMN_SelectableMul \
    CMN_VectorSelector \
    CMN_Summ \
    Hysteresis \
    KTS_AV_MIXER \
    MP_VOL_VOLOX \
    MP_EV_EVAPORATOR \
    MP_EV_IN_TankPump \
    MP_EV_Tank \
    MP_CR_Crystallizer \
    MP_CR_FeedController \
    MP_CR_HeatSetter \
    MP_CR_InitTank \
    MP_CR_PeristalticPump \
    MP_CR_Syringe \
    CMN_Mux \
    CMN_Demux \
    MP_EX_TankPump \
    MP_EX_Branch \
    CMN_Summator \
    MP_EX_EXTRACTOR \
    TR_EXTRACTOR \
    TR_EXTRACTOR_2 \
    #TR_REEXTRACTOR \
    TR_EXTRACTOR_4 \
    From_OPC \
    To_OPC \
    TEST_AvariikaTest \
    TR_VAPE \
    TR_REF \
    HYDR \
    ScaleTester \
    E2E_MFR\
    GCS
