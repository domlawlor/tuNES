#include "apu.h"

static void ClockLengthCounters(Apu *apu)
{
	if(!apu->square1.lengthCounterHalt && apu->square1.lengthCounter > 0)
		--apu->square1.lengthCounter;

	if(!apu->square2.lengthCounterHalt && apu->square2.lengthCounter > 0)
		--apu->square2.lengthCounter;
	if(!apu->triangle.linearCtrl && apu->triangle.lengthCounter > 0)
		--apu->triangle.lengthCounter;
	if(!apu->noise.lengthCounterHalt && apu->noise.lengthCounter > 0)
		--apu->noise.lengthCounter;
}

static void ClockSweep(Apu *apu)
{
	--apu->square1.sweepDivider;

	// Output clock
	if(apu->square1.enableSweep && apu->square1.sweepDivider == 0)
	{
		u16 period = ((u16)apu->square1.periodHigh << 8) | apu->square1.periodLow;

		u16 shift = period >> apu->square1.shiftCount;

		u16 resultPeriod;

		if(apu->square1.negative)
		{
			resultPeriod = period - shift;
			// NOTE: TODO: Only on the square two, the shift is subtracted then incremented by 1.
			//   See all the details, not completely sure yet
		}
		else
		{
			resultPeriod = period + shift;
		}

		// TODO: MORE WORK ON MUTING IS REQUIRED. Something about it happening even when disabled??

		// Mute
		if(period < 8 || resultPeriod > 0x7FF)
		{
			// Channel DAC receives 0 and sweep unit doesn't change channel period.
		}
		else if(apu->square1.shiftCount > 0)
		{
			apu->square1.periodHigh = (resultPeriod >> 8) & 0x7;
			apu->square1.periodLow = (u8)resultPeriod;
		}


	}

	if(apu->square1.sweepReset || apu->square1.sweepDivider == 0)
	{
		apu->square1.sweepReset = false;
		apu->square1.sweepDivider = apu->square1.sweepPeriod + 1;
	}
}

static void ClockSequencer(Apu *apu)
{
	/*
	d = 0
	_ - _ _ _ _ _ _
	d = 1
	_ - - _ _ _ _ _
	d = 2
	_ - - - - _ _ _
	d = 3
	- _ _ - - - - -
	*/

	// Period is decreased each frame. When it reaches zero(goes from 0 to t) it is clocked. 

	// Sequencer starts at zero, and is decremented. Goes from 0 to 7, 6, 5, 4 so on

/*
	  The mixer receives the current envelope volume except when
	  The sequencer output is zero, or
	  overflow from the sweep unit's adder is silencing the channel, or
	  the length counter is zero, or
	  the timer has a value less than eight.
*/
}

static void ClockEnvelope(Apu *apu)
{
	if(apu->square1.restartEnv)
	{
		apu->square1.restartEnv = false;
		apu->square1.envDivider = apu->square1.volumePeriod + 1;
		apu->square1.envCounter = 15;
	}
	else
	{
		--apu->square1.envDivider;

		if(apu->square1.envDivider == 0)
		{
			apu->square1.envDivider = apu->square1.volumePeriod + 1;

			// TODO:
			if(apu->square1.envCounter > 0)
				--apu->square1.envCounter;
			else if(apu->square1.lengthCounterHalt)
				apu->square1.envCounter = 15;

			// NOTE TODO: Output CLOCK here!!
		}
	}
}

static void ApuTick(Apu *apu)
{
	// Frame Sequencer clocks everything
	if(!apu->mode)
	{
		switch(apu->frameCounter)
		{
		case 0:
		{
			ClockEnvelope(apu);
			break;
		}
		case 1:
		{
			ClockLengthCounters(apu);
			ClockSweep(apu);
			ClockEnvelope(apu);
			break;
		}
		case 2:
		{
			ClockEnvelope(apu);
			break;
		}
		case 3:
		{
			ClockLengthCounters(apu);
			ClockSweep(apu);
			ClockEnvelope(apu);

			if(!apu->irqInhibit)
			{
				triggerIRQ = true;
			}

			break;
		}
		}

		++apu->frameCounter;

		if(apu->frameCounter > 3)
			apu->frameCounter = 0;
	}
	else
	{
		switch(apu->frameCounter)
		{
		case 0:
		{
			ClockLengthCounters(apu);
			ClockSweep(apu);
			ClockEnvelope(apu);
			break;
		}
		case 1:
		{
			ClockEnvelope(apu);
			break;
		}
		case 2:
		{
			ClockLengthCounters(apu);
			ClockSweep(apu);
			ClockEnvelope(apu);
			break;
		}
		case 3:
		{
			ClockEnvelope(apu);
			break;
		}
		case 4:
		{
			// NOTHING HAPPENS ON THIS CLOCK
			break;
		}
		}

		++apu->frameCounter;

		if(apu->frameCounter > 4)
			apu->frameCounter = 0;
	}

	// Square 1
		// Envelop picks volume

	u8 square1Out = 0;

	// Square 2
	u8 square2Out = 0;

	// Triangle
	u8 triangleOut = 0;

	// Noise
	u8 noiseOut = 0;

	// DMC
	u8 dmcOut = 0;

	// Mixing
	apu->finalOutput = ((0.00752f * (square1Out + square2Out)) +
		(0.00851f * triangleOut) +
		(0.00494f * noiseOut) +
		(0.00335f * dmcOut));
}


static void InitApu(Apu *apu)
{
	*apu = {};
	// All registers are clear
}
