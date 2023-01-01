#pragma once

#include "globals.h"

#include "blip_buf.h"

constexpr u64 CyclesEachAudioFrame = 29780;//10000;
constexpr int MaxSamplesInBlipBuffer = 100000;//44100;
constexpr int MaxSamplesInTempBuffer = 1024;

class Apu;

class Envelope
{
public:
	void Reset()
	{
		*this = {};
	}

	u8 GetEnvelopeVolume()
	{
		return (m_constVolume) ? m_volume : m_counter;
	}

	void UpdateEnvelope()
	{
		if(m_resetSet)
		{
			m_resetSet = false;
			m_counter = 15;
			m_divider = m_volume;
		}
		else
		{
			m_divider -= 1;
			if(m_divider < 0) // Clock envelope here 
			{
				m_divider = m_volume;
				if(m_counter > 0) { m_counter -= 1; }
				else if(m_loopSet) { m_counter = 15; }
			}
		}
	}


	void SetLoop(bool isLooping) { m_loopSet = isLooping; }
	void SetVolume(u8 volume) { m_volume = volume; }
	void SetConstVolume(bool isConstVolume) { m_constVolume = isConstVolume; }
	void RegisterReset() { m_resetSet = true; }

private:
	bool m_resetSet = false;
	bool m_loopSet = false;
	bool m_constVolume = false;
	s8 m_divider = 0;
	u8 m_counter = 0;
	u8 m_volume = 0;
};

constexpr u8 SquareDutySequences[4][8] = 
{
	{ 0, 0, 0, 0, 0, 0, 0, 1 },
	{ 0, 0, 0, 0, 0, 0, 1, 1 },
	{ 0, 0, 0, 0, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1, 1, 0, 0 }
};

struct SquareChannel
{
	s16 *outputDeltas;
	//s16 currentOutput = 0;
	s16 lastOutput = 0;

	u64 lastCycle = 0;

	Envelope envelope;

	bool enabled = false;

	u8 dutyCycle = 0;

	bool haltLengthCounter = false;
	u8 lengthCounter = 0;

	u16 period = 0;
	u16 doublePeriod = 0;

	u8 currentDutyPos = 0;

	u32 targetSweepPeriod = 0;
	u8 sweepDivider = 0;
	u8 sweepPeriod = 0;
	u8 sweepShiftCount = 0;
	bool enabledSweep = false;
	bool sweepResetSet = false;
	bool sweepNegateSet = false;

	bool isSquare1 = false;

	u16 timer = 0;
	

	SquareChannel()
	{
		outputDeltas = (s16 *)MemAlloc(CyclesEachAudioFrame * sizeof(s16));
		Reset();
	}

	~SquareChannel()
	{
		MemFree(outputDeltas);
	}

	void Reset()
	{
		MemorySet(outputDeltas, 0, CyclesEachAudioFrame * sizeof(s16));

		//currentOutput = 0;
		lastOutput = 0;

		lastCycle = 0;

		envelope.Reset();

		enabled = false;

		dutyCycle = 0;

		haltLengthCounter = false;
		lengthCounter = 0;

		period = 0;
		doublePeriod = 0;

		currentDutyPos = 0;

		targetSweepPeriod = 0;
		sweepDivider = 0;
		sweepPeriod = 0;
		sweepShiftCount = 0;
		enabledSweep = false;
		sweepResetSet = false;
		sweepNegateSet = false;

		isSquare1 = false;

		timer = 0;
	}

	bool IsUnmuted()
	{
		bool sweepMuted = !sweepNegateSet && targetSweepPeriod > 0x7FF;
		bool periodMuted = period < 8;
		return !(periodMuted || sweepMuted);
	}

	void UpdatePeriodData()
	{
		doublePeriod = (period * 2) + 1;

		u16 sweepShiftValue = period >> sweepShiftCount;
		if(sweepNegateSet)
		{
			targetSweepPeriod = period - sweepShiftValue;
			if(isSquare1) { targetSweepPeriod -= 1; }
		}
		else 
		{
			targetSweepPeriod = period + sweepShiftValue;
		}
	}

	void UpdateSweep()
	{
		sweepDivider -= 1;
		if(sweepDivider == 0)
		{
			if(enabledSweep && sweepShiftCount > 0 && period >= 8 && targetSweepPeriod <= 0x7FF)
			{
				period = targetSweepPeriod;
				UpdatePeriodData();
			}
			sweepDivider = sweepPeriod;
		}

		if(sweepResetSet)
		{
			sweepResetSet = false;
			sweepDivider = sweepPeriod;	
		}
	}

	void ClockUpdate(u64 currentCycle, Apu *apu);
};

constexpr  u8 TriangleSequence[32] = 
{ 
	15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

struct TriangleChannel
{
	s16 *outputDeltas;
	//s16 currentOutput = 0;
	s16 lastOutput = 0;

	u64 lastCycle = 0;

	bool enabled = false;

	bool haltLengthCounter = false;
	u8 lengthCounter = 0;

	u16 period = 0;

	u8 linearCounterOnReset = 0;
	u8 currentLinearCounter = 0;
	bool resetCounter = false;

	u8 triangleSequencePos = 0;

	u16 timer = 0;


	TriangleChannel()
	{
		outputDeltas = (s16 *)MemAlloc(CyclesEachAudioFrame * sizeof(s16));
		Reset();
	}

	~TriangleChannel()
	{
		MemFree(outputDeltas);
	}

	void Reset()
	{
		MemorySet(outputDeltas, 0, CyclesEachAudioFrame * sizeof(s16));

		//currentOutput = 0;
		lastOutput = 0;

		lastCycle = 0;

		enabled = false;

		haltLengthCounter = false;
		lengthCounter = 0;

		period = 0;

		linearCounterOnReset = 0;
		currentLinearCounter = 0;
		resetCounter = false;

		triangleSequencePos = 0;

		timer = 0;
	}

	bool IsUnmuted()
	{
		return period >= 2;
	}

	void UpdateLinearCounter()
	{
		if(resetCounter)
		{
			currentLinearCounter = linearCounterOnReset;
		}
		else if(currentLinearCounter > 0)
		{
			currentLinearCounter -= 1;
		}

		if(!haltLengthCounter)
		{
			resetCounter = false;
		}
	}

	void ClockUpdate(u64 currentCycle, Apu *apu);
};

constexpr u16 NoisePeriodTable[16] = 
{
	4, 8, 16, 32,
	64, 96, 128, 160,
	202, 254, 380, 508,
	762, 1016, 2034, 4068
};

constexpr u16 NoisePeriodTablePAL[16] =
{
	4, 8, 14, 30,
	60, 88, 118, 148,
	188, 236, 354, 472,
	708,  944, 1890, 3778
};

struct NoiseChannel
{
	s16 *outputDeltas;
	//s16 currentOutput = 0;
	s16 lastOutput = 0;

	u64 lastCycle = 0;

	Envelope envelope;
	
	u16 period = 0;
	u16 shiftRegister = 1;
	u16 timer = 0;
	u8 lengthCounter = 0;

	bool enabled = false;
	bool modeFlag = false;
	bool haltLengthCounter = false;

	NoiseChannel()
	{
		outputDeltas = (s16 *)MemAlloc(CyclesEachAudioFrame * sizeof(s16));
		Reset();
	}

	~NoiseChannel()
	{
		MemFree(outputDeltas);
	}

	void Reset()
	{
		MemorySet(outputDeltas, 0, CyclesEachAudioFrame * sizeof(s16));

		//currentOutput = 0;
		lastOutput = 0;
		lastCycle = 0;

		envelope.Reset();

		period = NoisePeriodTable[0] - 1;
		shiftRegister = 1;
		timer = 0;
		lengthCounter = 0;

		enabled = false;
		modeFlag = false;
		haltLengthCounter = false;
	}

	bool IsUnmuted() { return (shiftRegister & 0x1) == 0; };
	void ClockUpdate(u64 currentCycle, Apu *apu);
};

constexpr u16 DmcPeriodTable[16] =
{ 
	428, 380, 340, 320,
	286, 254, 226, 214,
	190, 160, 142, 128,
	106,  84,  72,  54
};

constexpr u16 DmcPeriodTablePAL[16] =
{
	398, 354, 316, 298,
	276, 236, 210, 198,
	176, 148, 132, 118,
	98,  78,  66,  50
};

struct DmcChannel
{
	s16 *outputDeltas = nullptr;

	u64 lastCycle = 0;

	bool enabled = false;
	bool irqEnable = false;
	bool loopSet = false;
	bool silenceSet = true;
	bool hasBufferData = false;
	bool updateRequired = false;

	s16 currentOutput = 0;
	s16 lastOutput = 0;

	u16 period = 0;
	u16 timer = 0;
	u16 sampleAddress = 0;
	u16 sampleLength = 0;
	u16 activeAddress = 0;
	u16 bytesLeft = 0;

	u8 bitsToRead = 0;
	u8 shiftRegister = 0;
	u8 readByte = 0;
	u8 enabledCycleDelay = 0;

	DmcChannel()
	{
		outputDeltas = (s16 *)MemAlloc(CyclesEachAudioFrame * sizeof(s16));
		Reset();
	}

	~DmcChannel()
	{
		MemFree(outputDeltas);
	}

	void Reset()
	{
		MemorySet(outputDeltas, 0, CyclesEachAudioFrame * sizeof(s16));
	
		lastCycle = 0;

		enabled = false;
		irqEnable = false;
		loopSet = false;
		silenceSet = true;
		hasBufferData = false;
		updateRequired = false;

		currentOutput = 0;
		lastOutput = 0;

		period = DmcPeriodTable[0] - 1;
		timer = period;
		sampleAddress = 0;
		sampleLength = 0;
		activeAddress = 0;
		bytesLeft = 0;

		bitsToRead = 0;
		shiftRegister = 0;
		readByte = 0;
		enabledCycleDelay = 0;
	}

	
	void ClockUpdate(u64 currentCycle, Apu *apu);

	void StartSample();
	void StartDMCWrite();
	void EnabledUpdated();
	bool NeedsUpdate();
	void SetDmcReadBuffer(u8 value);
	bool IsIrqUpcoming(u32 cycleDelta);
};

// From mesen
constexpr s32 FrameCounterCycleSteps[2][6] =
{
	{ 7457, 14913, 22371, 29828, 29829, 29830},
	{ 7457, 14913, 22371, 29829, 37281, 37282}
};

enum class FrameCounterStepType
{
	NONE,
	QUARTER,
	HALF
};

constexpr FrameCounterStepType FrameCounterStepTypes[2][6] =
{
	{ FrameCounterStepType::QUARTER, FrameCounterStepType::HALF, FrameCounterStepType::QUARTER,
		FrameCounterStepType::NONE, FrameCounterStepType::HALF, FrameCounterStepType::NONE },
	{ FrameCounterStepType::QUARTER, FrameCounterStepType::HALF, FrameCounterStepType::QUARTER,
		FrameCounterStepType::NONE, FrameCounterStepType::HALF, FrameCounterStepType::NONE }
};

constexpr u8 LengthCounterTable[] =
{
	0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06,
	0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E,
	0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16,
	0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E
};


class Apu
{
public:
	Apu();
	~Apu();

	void RunCycle();

	void UpdateCycles();
	void OnFrameCounterClock(bool isHalfFrame);

	u8 ReadRegisters(u16 address);
	void WriteRegisters(u16 address, u8 value);

	bool IsFrameCounterUpdateRequired(u32 cycleDelta);

	void AddDeltaCycleNum(u64 cycleNum);

	void FillAudioBuffer(void *bufferToFill, u32 samplesNeeded);
	void FillAudioBuffer();

	u16 GetDmcActiveAddress() { return dmc.activeAddress; }
	void SetDmcReadBuffer(u8 value) { dmc.SetDmcReadBuffer(value); }
private:
	AudioStream audioStream;
	
	u64 deltaCycleNums[CyclesEachAudioFrame] = {0};
	u32 totalDeltas = 0;

	s16 square1Output = 0;
	s16 square2Output = 0;
	s16 triangleOutput = 0;
	s16 noiseOutput = 0;
	s16 dmcOutput = 0;

	s16 lastMixedOutput = 0;

	blip_t *sampleBuffer = nullptr;

	s16 *tempSampleBuffer = nullptr;

	SquareChannel square1;
	SquareChannel square2;
	TriangleChannel triangle;
	NoiseChannel noise;
	DmcChannel dmc;

	bool forceUpdateSet = false;

	u64 cycle = 0;
	u64 lastCycle = 0;

	u64 frameCounterLastCycle = 0;
	s16 frameCounterPendingWriteValue = 0;
	s8 frameCounterWriteWaitCycles = 0;

	u8 frameCounterMode = 0;
	u8 frameCounterCurrentStep = 0;

	// Skip cycles after clocking to the next Framecounter step
	u8 frameCounterSkipCycles = 0;

	bool frameCounterIrqInhibit = false;

	void Reset()
	{
		MemorySet(deltaCycleNums, 0, CyclesEachAudioFrame * sizeof(u64));
		totalDeltas = 0;

		square1Output = 0;
		square2Output = 0;
		triangleOutput = 0;
		noiseOutput = 0;
		dmcOutput = 0;

		lastMixedOutput = 0;

		blip_clear(sampleBuffer);
		MemorySet(tempSampleBuffer, 0, MaxSamplesInBlipBuffer * sizeof(s16));

		square1.Reset();
		square2.Reset();
		triangle.Reset();
		noise.Reset();
		dmc.Reset();

		forceUpdateSet = false;

		cycle = 0;
		lastCycle = 0;

		frameCounterLastCycle = 0;
		frameCounterPendingWriteValue = 0;
		frameCounterWriteWaitCycles = 0;

		frameCounterMode = 0;
		frameCounterCurrentStep = 0;

		// Skip cycles after clocking to the next Framecounter step
		frameCounterSkipCycles = 0;

		frameCounterIrqInhibit = false;
	}
};