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


//u8 LengthCounterTable[] = {0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06,
//							  0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E,
//							  0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16,
//							  0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E};
//
//static void WriteApuRegister(u8 byte, u16 address)
//{
//	Apu *apu = &globalNes.apu;
//
//	globalNes.openBus = byte;
//
//	switch(address)
//	{
//	case 0x4000:
//	case 0x4004:
//	{
//		Square *square = (0x4000 <= address && address < 0x4004) ? &apu->square1 : &apu->square2;
//
//		square->dutyCycle = ((byte & 0xC0) >> 6);
//		square->lengthCounterHalt = ((byte & 0x20) != 0);
//		square->envelopeDisable = ((byte & 0x10) != 0);
//		square->volumePeriod = (byte & 0xF);
//		break;
//	}
//	case 0x4001:
//	case 0x4005:
//	{
//		Square *square = (0x4000 <= address && address < 0x4004) ? &apu->square1 : &apu->square2;
//
//		square->enableSweep = ((byte & 0x80) != 0);
//		square->sweepPeriod = ((byte & 0x70) >> 4);
//		square->negative = ((byte & 0x8) != 0);
//		square->shiftCount = (byte & 0x7);
//
//		square->sweepReset = true;
//		break;
//	}
//	case 0x4002:
//	case 0x4006:
//	{
//		Square *square = (0x4000 <= address && address < 0x4004) ? &apu->square1 : &apu->square2;
//		square->periodLow = byte;
//		break;
//	}
//	case 0x4003:
//	{
//		if(apu->square1Enabled)
//			apu->square1.lengthCounter = LengthCounterTable[((byte & 0xF8) >> 3)];
//		apu->square1.periodHigh = (byte & 0x7);
//
//		apu->square1.restartEnv = true;
//		break;
//	}
//	case 0x4007:
//	{
//		if(apu->square2Enabled)
//			apu->square2.lengthCounter = LengthCounterTable[((byte & 0xF8) >> 3)];
//		apu->square2.periodHigh = (byte & 0x7);
//
//		apu->square2.restartEnv = true;
//		break;
//	}
//	case 0x4008:
//	{
//		apu->triangle.linearCtrl = ((byte & 0x80) != 0);
//		apu->triangle.linearCounter = (byte & 0x7F);
//		break;
//	}
//	case 0x400A:
//	{
//		apu->triangle.periodLow = byte;
//		break;
//	}
//	case 0x400B:
//	{
//		if(apu->triangleEnabled)
//			apu->triangle.lengthCounter = LengthCounterTable[((byte & 0xF8) >> 3)];
//		apu->triangle.periodHigh = (byte & 0x7);
//		break;
//	}
//	case 0x400C:
//	{
//		apu->noise.lengthCounterHalt = ((byte & 0x20) != 0);
//		apu->noise.envelopeDisable = ((byte & 0x10) != 0);
//		apu->noise.volumePeriod = (byte & 0xF);
//		break;
//	}
//	case 0x400E:
//	{
//		apu->noise.loopNoise = ((byte & 0x80) != 0);
//		apu->noise.loopPeriod = (byte & 0xF);
//		break;
//	}
//	case 0x400F:
//	{
//		if(!apu->noiseEnabled)
//			apu->noise.lengthCounter = LengthCounterTable[((byte & 0xF8) >> 3)];
//		apu->noise.restartEnv = true; // TODO: Unsure if noise envelope is reset
//		break;
//	}
//	case 0x4010:
//	{
//		apu->dmc.irqEnable = ((byte & 0x80) != 0);
//		apu->dmc.loop = ((byte & 0x40) != 0);
//		apu->dmc.freqIndex = (byte & 0xF);
//		break;
//	}
//	case 0x4011:
//	{
//		apu->dmc.loadCounter = (byte & 0x7F);
//		break;
//	}
//	case 0x4012:
//	{
//		apu->dmc.sampleAddress = byte;
//		break;
//	}
//	case 0x4013:
//	{
//		apu->dmc.sampleLength = byte;
//		break;
//	}
//	case 0x4015:
//	{
//		apu->dmcEnabled = ((byte & 0x10) != 0);
//		apu->noiseEnabled = ((byte & 0x8) != 0);
//		apu->triangleEnabled = ((byte & 0x4) != 0);
//		apu->square2Enabled = ((byte & 0x2) != 0);
//		apu->square1Enabled = ((byte & 0x1) != 0);
//
//		if(!apu->dmcEnabled)
//		{
//			apu->dmc.sampleLength = 0;
//		}
//		if(!apu->square1Enabled)
//		{
//			apu->square1.lengthCounter = 0;
//		}
//		if(!apu->square2Enabled)
//		{
//			apu->square2.lengthCounter = 0;
//		}
//		if(!apu->triangleEnabled)
//		{
//			apu->triangle.lengthCounter = 0;
//		}
//		if(!apu->noiseEnabled)
//		{
//			apu->noise.lengthCounter = 0;
//		}
//
//		apu->dmcInterrupt = false;
//
//		// TODO: If DMC Bit is set, sample will only be restarted it bytes is 0. Else
//		//       the remaining bytes will finish before the next sample is fetched
//
//		break;
//	}
//	case 0x4017:
//	{
//		apu->mode = ((byte & 0x80) != 0);
//		apu->irqInhibit = ((byte & 0x40) != 0);
//
//		apu->frameCounter = 0;
//
//		// NOTE/TODO Writing to $4017 resets the frame counter and the quarter/half frame
//		// triggers happen simultaneously, but only on "odd" cycles (and only after the
//		// first "even" cycle after the write occurs) - thus, it happens either 2 or 3 cycles
//		// after the write (i.e. on the 2nd or 3rd cycle of the next instruction).
//		// After 2 or 3 clock cycles (depending on when the write is performed), the timer is reset.
//		// Writing to $4017 with bit 7 set ($80) will immediately clock all of its controlled units at
//		//the beginning of the 5-step sequence; with bit 7 clear, only the sequence is reset without
//		// clocking any of its units.
//		break;
//	}
//	}
//}
//
//static u8 ReadApuRegister(u16 address)
//{
//	Apu *apu = &globalNes.apu;
//
//	u8 byte = 0;
//
//	switch(address)
//	{
//	case 0x4015:
//	{
//		byte |= (apu->dmcInterrupt != 0) ? 0x80 : 0;
//		byte |= (apu->frameInterrupt != 0) ? 0x40 : 0;
//
//		byte |= (apu->dmc.sampleLength > 0) ? 0x10 : 0;
//		byte |= (apu->noise.lengthCounter > 0) ? 0x08 : 0;
//		byte |= (apu->triangle.lengthCounter > 0) ? 0x4 : 0;
//		byte |= (apu->square2.lengthCounter > 0) ? 0x2 : 0;
//		byte |= (apu->square1.lengthCounter > 0) ? 0x1 : 0;
//
//		apu->frameInterrupt = false;
//
//		// TODO: If an interrupt Flag was set the same moment as read,
//		// it will be read as set and not be cleared
//		/*if(0)
//			;
//		*/
//		globalNes.openBus = byte;
//		break;
//	}
//	}
//
//	return(globalNes.openBus);
//}




