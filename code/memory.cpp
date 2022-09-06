
#include "nes.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"

static u8 ReadPpuRegister(u16 address);
static void WritePpuRegister(u8 bte, u16 address);
static u8 ReadApuRegister(u16 address);
static void WriteApuRegister(u8 byte, u16 address);

// TODO: Forward declarations, is this the best idea?
static void RunPpu(Ppu *ppu, u16 clocksToRun);

static void RunPpuCatchup(u8 clocksIntoCurrentOp)
{
	// TODO: Find a better way to get Global values?
	Cpu *cpu = &globalNes.cpu;
	Ppu *ppu = &globalNes.ppu;
	//ppu *Apu = &globalNes.Apu;

	// New ClocksInto Op should be minus 1. Because we send in what
	// cycle we want to catch up too. We don't want to run that cycle
	// yet. Just one behind it
	u16 newClocks = (clocksIntoCurrentOp - 1) - cpu->lastClocksIntoOp;

	// Add the clocks already elapsed in Op.
	u16 clocksToRun = cpu->catchupClocks + newClocks;

	u16 ppuClocksToRun = clocksToRun * 3;

	RunPpu(ppu, ppuClocksToRun);

	cpu->catchupClocks = 0;
	cpu->lastClocksIntoOp = (clocksIntoCurrentOp - 1);
	Assert((clocksIntoCurrentOp - 1) >= 0);
}

static u8 Read8(u8 *address, u8 currentCycle)
{
	/*
	if((0x2000 <= address && address <= 0x2007) || address == 0x4014)
	{
		runPpuCatchup(CurrentCycle);
	}
	*/
	
	return *address;
}


static u16 CpuMemoryMirror(u16 address)
{
	// NOTE: Mirrors the address for the 2kb ram 
	if(0x0800 <= address && address < 0x2000)
	{
		address = (address % 0x0800);
	}
	// NOTE: Mirror for PPU Registers
	if(0x2008 <= address && address < 0x4000)
	{
		address = (address % (0x2008 - 0x2000)) + 0x2000;
	}

	return(address);
}

static u8 ReadCpu8(u16 address, Cpu *cpu)
{
	address = CpuMemoryMirror(address);

	if((0x2000 <= address && address < 0x2008) ||
		(address == 0x4014))
	{
		return ReadPpuRegister(address);
	}
	else if((0x4000 <= address && address <= 0x4013) ||
		address == 0x4015 || address == 0x4017)
	{
		return ReadApuRegister(address);
	}

	u8 value = cpu->memory[address];

	// Input
	if(address == 0x4016 || address == 0x4017)
	{
		if(!cpu->padStrobe)
		{
			if(address == 0x4016)
			{
				cpu->pad1CurrentButton = ++(cpu->pad1CurrentButton);
			}
			else
			{
				cpu->pad2CurrentButton = ++(cpu->pad2CurrentButton);
			}
		}

		u16 inputAddress;
		u8 btnValue;

		if(address == 0x4016)
		{
			inputAddress = 0x4016;
			btnValue = cpu->inputPad1.buttons[cpu->pad1CurrentButton] & 1;
		}
		else
		{
			inputAddress = 0x4017;
			btnValue = cpu->inputPad2.buttons[cpu->pad2CurrentButton] & 1;
		}

		u8 currentValue = cpu->memory[inputAddress];
		u8 newValue = (currentValue & 0xFE) | btnValue;
		cpu->memory[inputAddress] = newValue;
	}

	return(value);
}

static u8 ReadCpu8(u16 address, Cpu *cpu, u8 currentCycle)
{
	if((0x2000 <= address && address <= 0x2007) || address == 0x4014)
	{
		RunPpuCatchup(currentCycle);
	}

	return ReadCpu8(address, cpu);
}

static u16 ReadCpu16(u16 address, Cpu *cpu)
{
	// NOTE: Little Endian
	u8 lowByte = ReadCpu8(address, cpu);
	u8 highByte = ReadCpu8(address + 1, cpu);

	u16 newAddress = (highByte << 8) | lowByte;
	return(newAddress);
}

static void WriteCpu8(u8 byte, u16 address, Cpu *cpu)
{
	address = CpuMemoryMirror(address);

	if((0x2000 <= address && address < 0x2008) ||
		(address == 0x4014))
	{
		WritePpuRegister(byte, address);
	}
	else if((0x4000 <= address && address <= 0x4013) ||
		address == 0x4015 || address == 0x4017)
	{
		WriteApuRegister(byte, address);
	}

	if(address < 0x8000)
	{
		cpu->memory[address] = byte;
	}

	// Input
	if(address == 0x4016 || address == 0x4017)
	{
		u8 reg1Value = cpu->memory[0x4016];
		u8 reg2Value = cpu->memory[0x4017];

		u8 bit0 = (reg1Value | reg2Value) & 1;

		if(bit0 == 0)
		{
			if(cpu->padStrobe)
			{
				cpu->pad1CurrentButton = cpu->pad2CurrentButton = Input::B_A;
			}
			cpu->padStrobe = false;
		}
		else if(bit0 == 1)
		{
			cpu->padStrobe = true;
		}

		u8 btnValue = cpu->inputPad1.buttons[cpu->pad1CurrentButton] & 1;
		cpu->memory[0x4016] = btnValue;

		btnValue = cpu->inputPad2.buttons[cpu->pad2CurrentButton] & 1;
		cpu->memory[0x4017] = btnValue;
	}

	// Mapper
	if(address >= 0x8000)
	{
		Assert(globalNes.cartridge.mapperNum < ArrayCount(MapperUpdate));
		MapperUpdate[globalNes.cartridge.mapperNum](&globalNes, byte, address);
	}
}

static void WriteCpu8(u8 byte, u16 address, Cpu *cpu, u8 currentCycle)
{
	if((0x2000 <= address && address <= 0x2007) || address == 0x4014)
	{
		RunPpuCatchup(currentCycle);
	}
	WriteCpu8(byte, address, cpu);
}

static u8 *GetNametableBank(u16 address, Ppu *ppu)
{
	u8 *result = 0;

	switch(ppu->mirrorType)
	{
	case SINGLE_SCREEN_BANK_A:
	{
		result = ppu->nametableBankA;
		break;
	}
	case SINGLE_SCREEN_BANK_B:
	{
		result = ppu->nametableBankB;
		break;
	}
	case VERTICAL_MIRROR:
	{
		if(address < 0x2400 || (0x2800 <= address && address < 0x2C00))
		{
			result = ppu->nametableBankA;
		}
		else
		{
			result = ppu->nametableBankB;
		}
		break;
	}
	case HORIZONTAL_MIRROR:
	{
		if(address < 0x2800)
		{
			result = ppu->nametableBankA;
		}
		else
		{
			result = ppu->nametableBankB;
		}
		break;
	}
	case FOUR_SCREEN_MIRROR:
	{
		Assert(0);
		if(address < 0x2400)
		{
			result = ppu->nametableBankA;
		}
		else if(address < 0x2800)
		{
			result = ppu->nametableBankB;
		}
		else if(address < 0x2C00)
		{
			result = ppu->nametableBankC;
		}
		else
		{
			result = ppu->nametableBankD;
		}
		break;
	}
	}

	return result;
}

static u8 ReadNametable(u16 address, Ppu *ppu)
{
	u8 result = 0;

	u8 *nametable = GetNametableBank(address, ppu);
	result = nametable[address % 0x400];

	return result;
}

static void WriteNametable(u8 byte, u16 address, Ppu *ppu)
{
	u8 *nametable = GetNametableBank(address, ppu);
	nametable[address % 0x400] = byte;
}

static u16 PpuMemoryMirror(u16 address)
{
	Ppu *ppu = &globalNes.ppu;

	// Over half of the memory map is mirrored
	if(address >= 0x4000) {
		address = address % 0x4000;
	}

	if(0x3F20 <= address && address < 0x4000) {
		address = (address % 0x20) + 0x3F00;
	}

	if(0x3F00 <= address && address < 0x3F20) // Palette
	{
		if(address == 0x3F10) { address = 0x3F00; }
		else if(address == 0x3F14) { address = 0x3F04; }
		else if(address == 0x3F18) { address = 0x3F08; }
		else if(address == 0x3F1C) { address = 0x3F0C; }
	}

	// NOTE: Nametable Mirroring. Controlled by Cartridge
	 // This first as it maps to the nametable range
	if(0x3000 <= address && address < 0x3F00) {
		address -= 0x1000;
	}

	return address;
}

static u8 ReadPpu8(u16 address, Ppu *ppu)
{
	u8 result;

	address = PpuMemoryMirror(address);

	if((ppu->showBackground || ppu->showSprites) &&
		(address == 0x3F04 || address == 0x3F08 || address == 0x3F0C))
	{
		address = 0x3F00;
	}

	// If address in nametable range. Then map to the current mirror state and return
	if(0x2000 <= address && address < 0x3000)
	{
		result = ReadNametable(address, ppu);
	}
	else
	{
		result = ppu->memory[address];
	}
	return(result);
}

static void WritePpu8(u8 byte, u16 address, Ppu *ppu)
{
	address = PpuMemoryMirror(address);

	if(0x2000 <= address && address < 0x3000)
	{
		WriteNametable(byte, address, ppu);
	}
	else
	{
		ppu->memory[address] = byte;
	}
}

static void WritePpuRegister(u8 byte, u16 address)
{
	Ppu *ppu = &globalNes.ppu;

	globalNes.openBus = byte;

	switch(address)
	{
	case 0x2000:
	{
		ppu->nametableBase = byte & 3;
		ppu->vRamIncrement = ((byte & 4) != 0) ? 32 : 1;
		ppu->sPRTPattenBase = ((byte & 8) != 0) ? 0x1000 : 0;
		ppu->bGPatternBase = ((byte & 16) != 0) ? 0x1000 : 0;
		ppu->spriteSize8x16 = ((byte & 32) != 0);
		ppu->ppuSlave = ((byte & 64) != 0);
		ppu->generateNMI = ((byte & 128) != 0);

		ppu->tempVRamAdrs &= ~0xC00;
		ppu->tempVRamAdrs |= (ppu->nametableBase) << 10;

		// Nmi On Timing
		if(!(ppu->scanline == 261 && ppu->scanlineCycle == 1))
		{
			SetNmi(ppu->generateNMI && ppu->verticalBlank);
		}

		// Nmi off timing test
		if(ppu->scanline == 241 && (ppu->scanlineCycle == 4))
		{
			SetNmi(true);
		}

		break;
	}
	case 0x2001:
	{
		ppu->greyScale = ((byte & 1) != 0);
		ppu->showBGLeft8Pixels = ((byte & 2) != 0);
		ppu->showSPRTLeft8Pixels = ((byte & 4) != 0);
		ppu->showBackground = ((byte & 8) != 0);
		ppu->showSprites = ((byte & 16) != 0);
		ppu->emphasizeRed = ((byte & 32) != 0);
		ppu->emphasizeGreen = ((byte & 64) != 0);
		ppu->emphasizeBlue = ((byte & 128) != 0);

		ppu->renderingEnabled = (ppu->showBackground || ppu->showSprites);
		break;
	}
	case 0x2003:
	{
		ppu->oamAddress = byte;
		break;
	}
	case 0x2004:
	{
		// If Writing OAM Data while rendering, then a glitch increments it by 4 instead
		if(ppu->scanline < 240 || ppu->scanline == 261 || ppu->showBackground || ppu->showSprites)
		{
			ppu->oamAddress += 4;
		}
		else
		{
			ppu->oam[ppu->oamAddress] = byte;
			ppu->oamAddress++;
		}
		break;
	}
	case 0x2005:
	{
		if(ppu->latchWrite == 0)
		{
			ppu->fineX = byte & 7; // Bit 0,1, and 2 are fine X
			ppu->tempVRamAdrs &= ~(0x001F); // Clear Bits
			ppu->tempVRamAdrs |= ((u16)byte) >> 3;
			ppu->latchWrite = 1;
		}
		else
		{
			ppu->tempVRamAdrs &= ~(0x73E0); // Clear Bits
			ppu->tempVRamAdrs |= ((u16)(byte & 0x7)) << 12; // Set fine scroll Y, bits 0-2 set bit 12-14
			ppu->tempVRamAdrs |= ((u16)(byte & 0xF8)) << 2; // Set coarse Y, bits 3-7 set bit 5-9
			ppu->latchWrite = 0;
		}

		break;
	}
	case 0x2006:
	{
		if(ppu->latchWrite == 0)
		{
			ppu->tempVRamAdrs &= 0xC0FF; // Clear Bits About to be set 
			ppu->tempVRamAdrs |= ((u16)(byte & 0x003F)) << 8;
			ppu->tempVRamAdrs &= 0x3FFF; // Clear 14th bit 
			ppu->latchWrite = 1;
		}
		else
		{
			ppu->tempVRamAdrs &= 0x7F00; // Clear low byte
			ppu->tempVRamAdrs |= (u16)(byte & 0x00FF);
			ppu->vRamAdrs = ppu->tempVRamAdrs;
			ppu->latchWrite = 0;
		}

		break;
	}
	case 0x2007:
	{
		WritePpu8(byte, ppu->vRamAdrs, ppu);

		if(!(ppu->showBackground || ppu->showSprites) ||
			(ppu->scanline > 240 && ppu->scanline <= 260))
		{
			ppu->vRamAdrs += ppu->vRamIncrement;
		}
		break;
	}
	case 0x4014:
	{
		// NOTE: OAM DMA Write
		// TODO: Happens over time!
		for(u16 byteCount = 0; byteCount < 256; ++byteCount)
		{
			u16 newAddress = (byte << 8) | byteCount;

			u8 index = (ppu->oamAddress + byteCount);
			ppu->oam[index] = globalNes.cpu.memory[newAddress];
		}
		break;
	}
	}
}

static u8 ReadPpuRegister(u16 address)
{
	Ppu *ppu = &globalNes.ppu;

	u8 byte = 0;

	switch(address)
	{
	case 0x2002:
	{
		// NOTE: Reading VBL one cycle before it is set, returns clear and suppresses vbl
		if(!(ppu->scanline == 241 && (ppu->scanlineCycle == 1)))
		{
			byte |= ppu->verticalBlank ? 0x80 : 0;
			ppu->supressVbl = false;
		}
		else
		{
			ppu->supressVbl = true;
		}

		// NMI Suppression
		if(ppu->scanline == 241 &&
			(ppu->scanlineCycle == 1 || ppu->scanlineCycle == 2 || ppu->scanlineCycle == 3))
		{
			SetNmi(false);
			triggerNmi = false;

			if(ppu->scanlineCycle == 1)
				ppu->supressNmiSet = true;
		}

		ppu->verticalBlank = false;

		byte |= ppu->spriteZeroHit ? 0x40 : 0;
		byte |= ppu->spriteOverflow ? 0x20 : 0;
		byte |= (globalNes.openBus & 0x1F); // Low 5 bits is the open bus

		ppu->latchWrite = 0; // VRAM latch reset
		globalNes.openBus = byte;
		break;
	}
	case 0x2004:
	{
		globalNes.openBus = ppu->oam[ppu->oamAddress];
		break;
	}
	case 0x2007:
	{
		bool onPalette = !((ppu->vRamAdrs & 0x3FFF) < 0x3F00);

		if(onPalette)
		{
			ppu->vRamDataBuffer = ReadPpu8(ppu->vRamAdrs - 0x1000, ppu);
			byte = ReadPpu8(ppu->vRamAdrs, ppu);

			// Pulled from nes dev forum
			byte &= 0x3F;
			byte |= globalNes.openBus & 0xC0;
		}
		else
		{
			byte = ppu->vRamDataBuffer;
			ppu->vRamDataBuffer = ReadPpu8(ppu->vRamAdrs, ppu);
		}

		if(!(ppu->renderingEnabled) ||
			(ppu->scanline > 240 && ppu->scanline <= 260))
		{
			ppu->vRamAdrs += ppu->vRamIncrement;
		}
		else
		{
			// TODO: Weird update of vRam, check ppu_scrolling on wiki
		}

		globalNes.openBus = byte;
		break;
	}
	case 0x4014:
	{
		break;
	}
	}

	return(globalNes.openBus);
}


u8 LengthCounterTable[] = {0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06,
							  0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E,
							  0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16,
							  0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E};

static void WriteApuRegister(u8 byte, u16 address)
{
	Apu *apu = &globalNes.apu;

	globalNes.openBus = byte;

	switch(address)
	{
	case 0x4000:
	case 0x4004:
	{
		Square *square = (0x4000 <= address && address < 0x4004) ? &apu->square1 : &apu->square2;

		square->dutyCycle = ((byte & 0xC0) >> 6);
		square->lengthCounterHalt = ((byte & 0x20) != 0);
		square->envelopeDisable = ((byte & 0x10) != 0);
		square->volumePeriod = (byte & 0xF);
		break;
	}
	case 0x4001:
	case 0x4005:
	{
		Square *square = (0x4000 <= address && address < 0x4004) ? &apu->square1 : &apu->square2;

		square->enableSweep = ((byte & 0x80) != 0);
		square->sweepPeriod = ((byte & 0x70) >> 4);
		square->negative = ((byte & 0x8) != 0);
		square->shiftCount = (byte & 0x7);

		square->sweepReset = true;
		break;
	}
	case 0x4002:
	case 0x4006:
	{
		Square *square = (0x4000 <= address && address < 0x4004) ? &apu->square1 : &apu->square2;
		square->periodLow = byte;
		break;
	}
	case 0x4003:
	{
		if(apu->square1Enabled)
			apu->square1.lengthCounter = LengthCounterTable[((byte & 0xF8) >> 3)];
		apu->square1.periodHigh = (byte & 0x7);

		apu->square1.restartEnv = true;
		break;
	}
	case 0x4007:
	{
		if(apu->square2Enabled)
			apu->square2.lengthCounter = LengthCounterTable[((byte & 0xF8) >> 3)];
		apu->square2.periodHigh = (byte & 0x7);

		apu->square2.restartEnv = true;
		break;
	}
	case 0x4008:
	{
		apu->triangle.linearCtrl = ((byte & 0x80) != 0);
		apu->triangle.linearCounter = (byte & 0x7F);
		break;
	}
	case 0x400A:
	{
		apu->triangle.periodLow = byte;
		break;
	}
	case 0x400B:
	{
		if(apu->triangleEnabled)
			apu->triangle.lengthCounter = LengthCounterTable[((byte & 0xF8) >> 3)];
		apu->triangle.periodHigh = (byte & 0x7);
		break;
	}
	case 0x400C:
	{
		apu->noise.lengthCounterHalt = ((byte & 0x20) != 0);
		apu->noise.envelopeDisable = ((byte & 0x10) != 0);
		apu->noise.volumePeriod = (byte & 0xF);
		break;
	}
	case 0x400E:
	{
		apu->noise.loopNoise = ((byte & 0x80) != 0);
		apu->noise.loopPeriod = (byte & 0xF);
		break;
	}
	case 0x400F:
	{
		if(!apu->noiseEnabled)
			apu->noise.lengthCounter = LengthCounterTable[((byte & 0xF8) >> 3)];
		apu->noise.restartEnv = true; // TODO: Unsure if noise envelope is reset
		break;
	}
	case 0x4010:
	{
		apu->dmc.irqEnable = ((byte & 0x80) != 0);
		apu->dmc.loop = ((byte & 0x40) != 0);
		apu->dmc.freqIndex = (byte & 0xF);
		break;
	}
	case 0x4011:
	{
		apu->dmc.loadCounter = (byte & 0x7F);
		break;
	}
	case 0x4012:
	{
		apu->dmc.sampleAddress = byte;
		break;
	}
	case 0x4013:
	{
		apu->dmc.sampleLength = byte;
		break;
	}
	case 0x4015:
	{
		apu->dmcEnabled = ((byte & 0x10) != 0);
		apu->noiseEnabled = ((byte & 0x8) != 0);
		apu->triangleEnabled = ((byte & 0x4) != 0);
		apu->square2Enabled = ((byte & 0x2) != 0);
		apu->square1Enabled = ((byte & 0x1) != 0);

		if(!apu->dmcEnabled)
		{
			apu->dmc.sampleLength = 0;
		}
		if(!apu->square1Enabled)
		{
			apu->square1.lengthCounter = 0;
		}
		if(!apu->square2Enabled)
		{
			apu->square2.lengthCounter = 0;
		}
		if(!apu->triangleEnabled)
		{
			apu->triangle.lengthCounter = 0;
		}
		if(!apu->noiseEnabled)
		{
			apu->noise.lengthCounter = 0;
		}

		apu->dmcInterrupt = false;

		// TODO: If DMC Bit is set, sample will only be restarted it bytes is 0. Else
		//       the remaining bytes will finish before the next sample is fetched

		break;
	}
	case 0x4017:
	{
		apu->mode = ((byte & 0x80) != 0);
		apu->irqInhibit = ((byte & 0x40) != 0);

		apu->frameCounter = 0;

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

static u8 ReadApuRegister(u16 address)
{
	Apu *apu = &globalNes.apu;

	u8 byte = 0;

	switch(address)
	{
	case 0x4015:
	{
		byte |= (apu->dmcInterrupt != 0) ? 0x80 : 0;
		byte |= (apu->frameInterrupt != 0) ? 0x40 : 0;

		byte |= (apu->dmc.sampleLength > 0) ? 0x10 : 0;
		byte |= (apu->noise.lengthCounter > 0) ? 0x08 : 0;
		byte |= (apu->triangle.lengthCounter > 0) ? 0x4 : 0;
		byte |= (apu->square2.lengthCounter > 0) ? 0x2 : 0;
		byte |= (apu->square1.lengthCounter > 0) ? 0x1 : 0;

		apu->frameInterrupt = false;

		// TODO: If an interrupt Flag was set the same moment as read,
		// it will be read as set and not be cleared
		/*if(0)
			;
		*/
		globalNes.openBus = byte;
		break;
	}
	}

	return(globalNes.openBus);
}


