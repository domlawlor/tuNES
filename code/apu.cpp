
#include "nes.h"
#include "apu.h"

Apu::Apu()
{
	square1.isSquare1 = true;
}

Apu::~Apu()
{

}

void Apu::RunCycle()
{
	++cycle;

	constexpr u64 cyclesInAFrame = 10000;
	if(cycle > )
	{

	}
	
}


void Apu::FrameCounterUpdate()
{
	// On Half Frames
	{
		square1.UpdateSweep();
		square2.UpdateSweep();

		// Update length counters
		if(square1.lengthCounter > 0 && !square1.haltLengthCounter) { square1.lengthCounter -= 1; }
		if(square2.lengthCounter > 0 && !square2.haltLengthCounter) { square2.lengthCounter -= 1; }
		if(triangle.lengthCounter > 0 && !triangle.haltLengthCounter) { triangle.lengthCounter -= 1; }
		if(noise.lengthCounter > 0 && !noise.haltLengthCounter) { noise.lengthCounter -= 1; }
	}

	square1.envelope.UpdateEnvelope();
	square2.envelope.UpdateEnvelope();
	triangle.UpdateLinearCounter();
	noise.envelope.UpdateEnvelope();
}

u8 LengthCounterTable[] = {0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06,
							0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E,
							0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16,
							0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E};

void Apu::WriteRegisters(u16 address, u8 value)
{
	Nes::SetOpenBus(value);

	switch(address)
	{
	case 0x4000:
	case 0x4004:
	{
		Square *square = (address == 0x4004) ? &square2 : &square1;
		square->dutyCycle = ((value & 0xC0) >> 6);
		square->haltLengthCounter = ((value & 0x20) != 0);
		square->envelope.SetLoop(square->haltLengthCounter);
		square->envelope.SetConstVolume((value & 0x10) != 0);
		square->envelope.SetVolume(value & 0xF);
		break;
	}
	case 0x4001:
	case 0x4005:
	{
		Square *square = (address == 0x4005) ? &square2 : &square1;
		square->enabledSweep = ((value & 0x80)	!= 0);
		square->sweepPeriod = ((value & 0x70) >> 4) + 1; // Period + 1
		square->sweepNegateSet = ((value & 0x8) != 0);
		square->sweepShiftCount = value & 0x7;
		square->sweepResetSet = true;

		square->UpdatePeriodData();
		break;
	}
	case 0x4002:
	case 0x4006:
	{
		Square *square = (address == 0x4006) ? &square2 : &square1;
		square->period = (square->period & 0x700) || value;
		square->UpdatePeriodData();
		break;
	}
	case 0x4003:
	case 0x4007:
	{
		Square *square = (address == 0x4007) ? &square2 : &square1;

		if(square->enabled) { square->lengthCounter = LengthCounterTable[((value & 0xF8) >> 3)]; }
		square->period = (value & 0x7) << 8 || (square->period & 0xFF);
		square->UpdatePeriodData();

		square->currentDutyPos = 0;

		square->envelope.Reset();
		break;
	}
	case 0x4008:
	{
		triangle.haltLengthCounter = ((value & 0x80) != 0);
		triangle.linearCounterOnReset = value & 0x7F;
		break;
	}
	case 0x400A:
	{
		triangle.period = (triangle.period & 0x700) || value;
		break;
	}
	case 0x400B:
	{
		if(triangle.enabled) { triangle.lengthCounter = LengthCounterTable[((value & 0xF8) >> 3)]; }

		triangle.period = (value & 0x7) << 8 || (triangle.period & 0x00FF);

		triangle.resetCounter = true;
		break;
	}
	case 0x400C:
	{
		noise.haltLengthCounter = ((value & 0x20) != 0);
		noise.envelope.SetLoop(noise.haltLengthCounter);
		noise.envelope.SetConstVolume((value & 0x10) != 0);
		noise.envelope.SetVolume(value & 0xF);
		break;
	}
	case 0x400E:
	{
		u8 periodIndex = value & 0xF;
		noise.period = NoisePeriodsTable[periodIndex];

		noise.modeFlag = ((value & 0x80) != 0);
		break;
	}
	case 0x400F:
	{
		if(noise.enabled) { noise.lengthCounter = LengthCounterTable[((value & 0xF8) >> 3)]; }
		noise.envelope.Reset();
		break;
	}
	case 0x4010:
	{
		dmc.irqEnable = ((value & 0x80) != 0);
		dmc.loop = ((value & 0x40) != 0);
		dmc.freqIndex = value & 0xF;
		break;
	}
	case 0x4011:
	{
		dmc.loadCounter = value & 0x7F;
		break;
	}
	case 0x4012:
	{
		dmc.sampleAddress = value;
		break;
	}
	case 0x4013:
	{
		dmc.sampleLength = value;
		break;
	}
	case 0x4015:
	{
		square1.enabled = ((value & 0x1) != 0);
		square2.enabled = ((value & 0x2) != 0);
		triangle.enabled = ((value & 0x4) != 0);
		noise.enabled = ((value & 0x8) != 0);
		dmc.enabled = ((value & 0x10) != 0);

		if(!square1.enabled) { square1.lengthCounter = 0; }
		if(!square2.enabled) { square2.lengthCounter = 0; }
		if(!triangle.enabled) { triangle.lengthCounter = 0; }
		if(!noise.enabled) { noise.lengthCounter = 0; }
		if(!dmc.enabled) { dmc.sampleLength = 0; }

		dmcInterrupt = false;

		// TODO: If DMC Bit is set, sample will only be restarted it bytes is 0. Else
		//       the remaining bytes will finish before the next sample is fetched

		break;
	}
	case 0x4017:
	{
		mode = ((value & 0x80) != 0);
		irqInhibit = ((value & 0x40) != 0);

		frameCounter = 0;

		// NOTE/TODO Writing to $4017 resets the frame counter and the quarter/half frame
		// triggers happen simultaneously, but only on "odd" cycles (and only after the
		// first "even" cycle after the write occurs) - thus, it happens either 2 or 3 cycles
		// after the write (i.e. on the 2nd or 3rd cycle of the next instruction).
		// After 2 or 3 clock cycles (depending on when the write is performed), the timer is reset.
		// Writing to $4017 with bit 7 set ($80) will immediately clock all of its controlled units at
		//the beginning of the 5-step sequence; with bit 7 clear, only the sequence is reset without
		// clocking any of its units.
		break;
	}
	}
}

u8 Apu::ReadRegisters(u16 address)
{
	u8 value = 0;
	u8 openBusVal = Nes::GetOpenBus();

	switch(address)
	{
	case 0x4015:
	{
		value |= (dmcInterrupt != 0) ? 0x80 : 0;
		value |= (frameInterrupt != 0) ? 0x40 : 0;

		value |= (dmc.sampleLength > 0) ? 0x10 : 0;
		value |= (noise.lengthCounter > 0) ? 0x08 : 0;
		value |= (triangle.lengthCounter > 0) ? 0x4 : 0;
		value |= (square2.lengthCounter > 0) ? 0x2 : 0;
		value |= (square1.lengthCounter > 0) ? 0x1 : 0;

		frameInterrupt = false;

		// TODO: If an interrupt Flag was set the same moment as read,
		// it will be read as set and not be cleared
		/*if(0)
			;
		*/
		Nes::SetOpenBus(value);
		break;
	}
	}

	return value;
}
