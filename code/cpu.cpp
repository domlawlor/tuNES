#include "nes.h"

#include "cpu.h"


void Cpu::UpdateInterrupts()
{
	prevTriggerNmi = triggerNmi;
	triggerNmi = nmiSet && !prevTriggerNmi;
	prevTriggerNmi = nmiSet;

	prevTriggerIrq = triggerIrq;
	if(!IsFlagBitSet(INTERRUPT_BIT))
	{
		triggerIrq = (irqFlag & activeIrqs) ? 1 : 0;
	}
}

u8 Cpu::ReadMemory(u16 address)
{
	// One memory read is 1 cycles worth. 
	// Run other systems (Ppu, Apu) for half of this Cpu cycle, then run second half after the read

	// ProcessDMA();

	// Cycle accuracy inspired by mesen
	masterClock += clockCyclesPreCatchup - 1;
	//_cycleCount++;
	Nes::GetPpu()->RunCatchup(masterClock);
	//_console->ProcessCpuClock();


	u8 value = RawReadMemory(address);
	
	masterClock += clockCyclesPostCatchup + 1;
	Nes::GetPpu()->RunCatchup(masterClock);
	UpdateInterrupts();

	return value;
}

void Cpu::WriteMemory(u16 address, u8 value)
{
	// Cycle accuracy inspired by mesen
	
	masterClock += clockCyclesPreCatchup + 1;
	Nes::GetPpu()->RunCatchup(masterClock);

	RawWriteMemory(address, value);

	masterClock += clockCyclesPostCatchup - 1;
	Nes::GetPpu()->RunCatchup(masterClock);
	UpdateInterrupts();
}

u8 Cpu::RawReadMemory(u16 address)
{
	return memory[address];
}

void Cpu::RawWriteMemory(u16 address, u8 value)
{
	memory[address] = value;
}

void Cpu::Init()
{
	MemorySet(memory, 0, CpuMemorySize);

	// DEBUG at moment. Matching FCEUX initial cpu memory state
	for(u16 index = 0; index < 0x2000; ++index)
	{
		if(index % 8 >= 4)
		{
			memory[index] = 0xFF;
		}
	}

	for(u16 index = 0x4008; index < 0x5000; ++index)
	{
		memory[index] = 0xFF;
	}

	A = 0;
	X = 0;
	Y = 0;
	flags = 0x04;
	stackPointer = 0xFD;
	prgCounter = (RawReadMemory(RESET_VEC + 1) << 8) | RawReadMemory(RESET_VEC); // Read the reset vector directly. No need for ppu catchup cycles
	opName = "NUL";

	clockCyclesPreCatchup = 6;
	clockCyclesPostCatchup = 6;
}

void Cpu::OnReset()
{
	// NOTE: The status after reset was taken from nesdev
	stackPointer -= 3;
	SetInterrupt();
}

u16 Cpu::ReadOperand()
{
	if(addressMode == AddressMode::ACM || addressMode == AddressMode::IMPL)
	{
		ReadMemory(prgCounter); // Dummy Read for cycle accuracy
		return 0;
	}
	else if(addressMode == AddressMode::IMED || addressMode == AddressMode::REL)
	{
		u8 value = ReadMemory(prgCounter);
		prgCounter++;
		return value;
	}
	else if(addressMode == AddressMode::ZERO)
	{
		u8 value = ReadMemory(prgCounter);
		prgCounter++;
		return value;
	}
	else if(addressMode == AddressMode::ZERO_X)
	{
		u8 value = ReadMemory(prgCounter);
		prgCounter++;
		ReadMemory(value); // Dummy read for cycle accuracy
		return value + X;
	}
	else if(addressMode == AddressMode::ZERO_Y)
	{
		u8 value = ReadMemory(prgCounter);
		prgCounter++;
		ReadMemory(value); // Dummy read for cycle accuracy
		return value + Y;
	}
	else if(addressMode == AddressMode::IND || addressMode == AddressMode::ABS)
	{
		u16 value = (ReadMemory(prgCounter + 1) << 8) | ReadMemory(prgCounter);
		prgCounter += 2;
		return value;
	}
	else if(addressMode == AddressMode::IND_X)
	{
		u8 indBase = ReadMemory(prgCounter);
		prgCounter++;
		ReadMemory(indBase); // Dummy read for cycle accuracy

		u8 indDest = indBase + X;

		u16 address = 0x0000;
		if(indDest != 0xFF)
		{
			address = (ReadMemory(indDest + 1) << 8) | ReadMemory(indDest);
		}
		else
		{
			address = (ReadMemory(0x00) << 8) | ReadMemory(0xFF);
		}
		return address;
	}
	else if(addressMode == AddressMode::IND_Y || addressMode == AddressMode::IND_YW)
	{
		u8 ind = ReadMemory(prgCounter);
		prgCounter++;

		u16 address = 0x0000;
		if(ind != 0xFF)
		{
			address = (ReadMemory(ind + 1) << 8) | ReadMemory(ind);
		}
		else
		{
			address = (ReadMemory(0x00) << 8) | ReadMemory(0xFF);
		}

		u16 indAddress = address + Y;

		if(IsCrossPageBoundary(indAddress, address)) // Check if crossed page
		{
			ReadMemory(indAddress - 0x100); // Duummy Read
		}
		else if(addressMode == AddressMode::IND_YW) // write version also has a dummy read here if no page cross
		{
			ReadMemory(indAddress);
		}
		return indAddress;
	}
	else if(addressMode == AddressMode::ABS_X || addressMode == AddressMode::ABS_XW)
	{
		u16 address = (ReadMemory(prgCounter + 1) << 8) | ReadMemory(prgCounter);
		prgCounter += 2;

		u16 offsetAddress = address + X;
		if(IsCrossPageBoundary(offsetAddress, address)) // Check if crossed page
		{
			ReadMemory(offsetAddress - 0x100); // Duummy Read
		}
		else if(addressMode == AddressMode::ABS_XW)
		{
			ReadMemory(offsetAddress);
		}
		return offsetAddress;
	}
	else if(addressMode == AddressMode::ABS_Y || addressMode == AddressMode::ABS_YW)
	{
		u16 address = (ReadMemory(prgCounter + 1) << 8) | ReadMemory(prgCounter);
		prgCounter += 2;

		u16 offsetAddress = address + Y;
		if(IsCrossPageBoundary(offsetAddress, address)) // Check if crossed page
		{
			ReadMemory(offsetAddress - 0x100); // Duummy Read
		}
		else if(addressMode == AddressMode::ABS_YW)
		{
			ReadMemory(offsetAddress);
		}
		return offsetAddress;
	}

	// TODO: Print error here, address mode not implemented??
	return 0;
}

void Cpu::Run()
{
	// NOTE: How timing between CPU, APU and PPU functions
	//	Cpu is the main engine of the emulator. APU and PPU and updated during a CPU tick
	//  We run a full instruction each tick, and on read/writes to memory, we run the ppu and apu.
	//  This way we always keep each component in sync.
	//  See ReadMemory and WriteMemory for the catchup code
	
	// Get operation code for this instruction
	opCode = ReadMemory(prgCounter);
	++prgCounter;

	opName = OpNames[opCode]; // store for debugging

	// Get address mode used for this operation
	addressMode = OpAddressModes[opCode];
	
	// Using the address mode, read in the value used by the operation
	// each address mode does this slightly different, and some also don't read anything
	// We keep cycle accuracy by using dummy reads where the hardware would also do the same. Taking extra cycles
	operand = ReadOperand();

	(this->*operations[opCode])();

	if(prevTriggerNmi) { NMI(); }
	else if(prevTriggerIrq) { IRQ(); }
}

void Cpu::NMI()
{
	ReadMemory(prgCounter); // Dummy Read
	ReadMemory(prgCounter); // Dummy Read

	u8 highByte = (prgCounter >> 8) & 0xFF;
	u8 lowByte = prgCounter & 0xFF;
	PushStack(highByte);
	PushStack(lowByte);

	triggerNmi = false;

	u8 stackFlags = flags | BLANK_BIT;
	PushStack(stackFlags);

	SetInterrupt();

	u16 nmiAddress = (ReadMemory(NMI_VEC + 1) << 8) | ReadMemory(NMI_VEC);
	prgCounter = nmiAddress;
}

void Cpu::IRQ()
{
	ReadMemory(prgCounter); // Dummy Read
	ReadMemory(prgCounter); // Dummy Read

	u8 highByte = (prgCounter >> 8) & 0xFF;
	u8 lowByte = prgCounter & 0xFF;
	PushStack(highByte);
	PushStack(lowByte);

	u8 stackFlags = flags | BLANK_BIT;
	PushStack(stackFlags);

	SetInterrupt();

	u16 irqAddress = (ReadMemory(IRQ_BRK_VEC + 1) << 8) | ReadMemory(IRQ_BRK_VEC);
	prgCounter = irqAddress;
}




