#pragma once

#include "globals.h"

class Envelope
{
public:
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
	void Reset() { m_resetSet = true; }

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

struct Square
{
	bool enabled;

	u8 dutyCycle;

	Envelope envelope;

	bool haltLengthCounter = false;
	u8 lengthCounter = 0;

	u16 period = 0;
	u16 cyclesPerPeriod = 0;

	u8 currentDutyPos = 0;

	u32 targetSweepPeriod = 0;
	u8 sweepDivider = 0;
	u8 sweepPeriod = 0;
	u8 sweepShiftCount = 0;
	bool enabledSweep = false;
	bool sweepResetSet = false;
	bool sweepNegateSet = false;

	bool isSquare1 = false;

	bool IsUnmuted()
	{
		bool sweepMuted = !sweepNegateSet && targetSweepPeriod > 0x7FF;
		bool periodMuted = period < 8;
		return !(periodMuted || sweepMuted);
	}

	void UpdatePeriodData()
	{
		cyclesPerPeriod = (period * 2) + 1;

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

	void ClockUpdate()
	{
		currentDutyPos = (currentDutyPos - 1) & 0x07;
	}
};

constexpr  u8 TriangleSequence[32] = 
{ 
	15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

struct Triangle
{
	bool enabled;

	bool haltLengthCounter = false;
	u8 lengthCounter = 0;

	u16 period = 0;

	u8 linearCounterOnReset = 0;
	u8 currentLinearCounter = 0;
	bool resetCounter = false;

	u8 triangleSequencePos = 0;

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

	void ClockUpdate()
	{
		bool isActive = currentLinearCounter > 0 && lengthCounter > 0;

		if(!isActive)
		{
			return;
		}
		
		triangleSequencePos = (triangleSequencePos + 1) & 0x1F;
	}
};

constexpr u16 NoisePeriodsTable[16] = 
{
	4, 8, 16, 32,
	64, 96, 128, 160,
	202, 254, 380, 508,
	762, 1016, 2034, 4068
};

constexpr u16 NoisePeriodsTablePAL[16] =
{
	4, 8, 14, 30,
	60, 88, 118, 148,
	188, 236, 354, 472,
	708,  944, 1890, 3778
};


struct Noise
{
	bool enabled;
	
	Envelope envelope;

	bool haltLengthCounter = false;
	u8 lengthCounter = 0;

	bool modeFlag;
	u8 period;

	u16 shiftRegister = 1;

	bool IsUnmuted() { return shiftRegister == 1; }

	void Update()
	{
		uint16_t feedback = (shiftRegister & 0x01) ^ ((shiftRegister >> (modeFlag ? 6 : 1)) & 0x01);
		shiftRegister >>= 1;
		shiftRegister |= (feedback << 14);
	}
};

struct Dmc
{
	bool enabled;
	bool irqEnable;
	bool loop;
	u8 freqIndex;

	u8 loadCounter;
	u8 sampleAddress;
	u8 sampleLength;
};

class Apu
{
public:
	Apu();
	~Apu();

	void Reset();

	void RunCycle();

	u8 ReadRegisters(u16 address);
	void WriteRegisters(u16 address, u8 value);

	void FrameCounterUpdate();

	void CalculateSample()
	{
		u8 square1Value = 0;
		u8 square2Value = 0;
		u8 triangleValue = 0;
		u8 noiseValue = 0;
		u8 dmcValue = 0;

		if(square1.lengthCounter > 0 && square1.IsUnmuted())
		{
			u8 volume = square1.envelope.GetEnvelopeVolume();
			u8 sequenceValue = SquareDutySequences[square1.dutyCycle][square1.currentDutyPos];
			square1Value = volume * sequenceValue;
		}
		if(square2.lengthCounter > 0 && square2.IsUnmuted())
		{
			u8 volume = square2.envelope.GetEnvelopeVolume();
			u8 square2Value = SquareDutySequences[square2.dutyCycle][square2.currentDutyPos];
			square2Value = volume * square2Value;
		}
		if(triangle.lengthCounter > 0 && triangle.IsUnmuted())
		{
			triangleValue = TriangleSequence[triangle.triangleSequencePos];
		}
		if(noise.lengthCounter > 0 && noise.IsUnmuted())
		{
			noiseValue = noise.envelope.GetEnvelopeVolume();
		}

		float squareOutput = square1Value * square2Value;
		float tndOutput = dmcValue + (2 * noiseValue) + (3 * triangleValue);

		// Taken from mesen.. not exactly what is on NesDev. Worth testing
		u16 squareVolume = (u16)(477600 / (8128.0 / squareOutput + 100.0));
		u16 tndVolume = (u16)(818350 / (24329.0 / tndOutput + 100.0));

		s16 resultSample = squareVolume + tndVolume;
	}

private:
	Square square1;
	Square square2;
	Triangle triangle;
	Noise noise;
	Dmc dmc;

	bool dmcInterrupt;
	bool frameInterrupt;

	bool mode;
	bool irqInhibit;

	u64 cycle;

	//r32 finalOutput;
};