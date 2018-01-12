#if !defined(APU_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

struct square
{
    uint8 DutyCycle;
    bool32 LengthCounterHalt;
    bool32 EnvelopeDisable;
    uint8 VolumePeriod;
    bool32 EnableSweep;
    uint8 SweepPeriod;
    bool32 Negative;
    uint8 ShiftCount;
    uint8 PeriodLow;
    uint8 LengthCounter;
    uint8 PeriodHigh;


    uint8 SweepDivider;
    bool32 SweepReset;
    
    uint8 EnvDivider;
    uint8 EnvCounter;
    bool32 RestartEnv;
    
    uint8 SequenceValue;
    uint8 SequenceStep;
};

struct triangle
{
    bool32 LinearCtrl;
    uint8 LinearCounter;    
    uint8 PeriodLow;
    uint8 LengthCounter;
    uint8 PeriodHigh;
};

struct noise
{   
    bool32 LengthCounterHalt;
    bool32 EnvelopeDisable;
    uint8 VolumePeriod;
    bool32 LoopNoise;
    uint8 LoopPeriod;
    uint8 LengthCounter;

    bool32 RestartEnv;
};

struct dmc
{
    bool32 IRQEnable;
    bool32 Loop;
    uint8 FreqIndex;

    uint8 LoadCounter;
    uint8 SampleAddress;
    uint8 SampleLength;
};

struct apu
{
    square Square1;
    square Square2;
    triangle Triangle;
    noise Noise;
    dmc Dmc;

    bool32 DmcEnabled;
    bool32 NoiseEnabled;
    bool32 TriangleEnabled;
    bool32 Square1Enabled;
    bool32 Square2Enabled;

    bool32 DmcInterrupt;
    bool32 FrameInterrupt;

    bool32 Mode;
    bool32 IRQInhibit;

    uint8 FrameCounter;


    real32 FinalOutput;
};

#define APU_H
#endif
