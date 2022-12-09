#pragma once

struct Square
{
	u8 dutyCycle;
	bool lengthCounterHalt;
	bool envelopeDisable;
	u8 volumePeriod;
	bool enableSweep;
	u8 sweepPeriod;
	bool negative;
	u8 shiftCount;
	u8 periodLow;
	u8 lengthCounter;
	u8 periodHigh;


	u8 sweepDivider;
	bool sweepReset;

	u8 envDivider;
	u8 envCounter;
	bool restartEnv;

	u8 sequenceValue;
	u8 sequenceStep;
};

struct Triangle
{
	bool linearCtrl;
	u8 linearCounter;
	u8 periodLow;
	u8 lengthCounter;
	u8 periodHigh;
};

struct Noise
{
	bool lengthCounterHalt;
	bool envelopeDisable;
	u8 volumePeriod;
	bool loopNoise;
	u8 loopPeriod;
	u8 lengthCounter;

	bool restartEnv;
};

struct Dmc
{
	bool irqEnable;
	bool loop;
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

	bool dmcEnabled;
	bool noiseEnabled;
	bool triangleEnabled;
	bool square1Enabled;
	bool square2Enabled;

	bool dmcInterrupt;
	bool frameInterrupt;

	bool mode;
	bool irqInhibit;

	u8 frameCounter;

	r32 finalOutput;
};