#if !defined(APU_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

struct Square
{
    u8 dutyCycle;
    b32 lengthCounterHalt;
    b32 envelopeDisable;
    u8 volumePeriod;
    b32 enableSweep;
    u8 sweepPeriod;
    b32 negative;
    u8 shiftCount;
    u8 periodLow;
    u8 lengthCounter;
    u8 periodHigh;


    u8 sweepDivider;
    b32 sweepReset;
    
    u8 envDivider;
    u8 envCounter;
    b32 restartEnv;
    
    u8 sequenceValue;
    u8 sequenceStep;
};

struct Triangle
{
    b32 linearCtrl;
    u8 linearCounter;    
    u8 periodLow;
    u8 lengthCounter;
    u8 periodHigh;
};

struct Noise
{   
    b32 lengthCounterHalt;
    b32 envelopeDisable;
    u8 volumePeriod;
    b32 loopNoise;
    u8 loopPeriod;
    u8 lengthCounter;

    b32 restartEnv;
};

struct Dmc
{
    b32 irqEnable;
    b32 loop;
    u8 freqIndex;

    u8 loadCounter;
    u8 sampleAddress;
    u8 sampleLength;
};

struct Apu
{
    Square square1;
    Square square2;
    Triangle triangle;
    Noise noise;
    Dmc dmc;

    b32 dmcEnabled;
    b32 noiseEnabled;
    b32 triangleEnabled;
    b32 square1Enabled;
    b32 square2Enabled;

    b32 dmcInterrupt;
    b32 frameInterrupt;

    b32 mode;
    b32 irqInhibit;

    u8 frameCounter;

    r32 finalOutput;
};

#define APU_H
#endif
