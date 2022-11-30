#include "nes.h"

#include "cpu.h"


#include "operations.cpp"


u8 Cpu::ReadMemory(u16 address)
{
	// One memory read is 1 cycles worth. 
	// Run other systems (Ppu, Apu) for half of this Cpu cycle, then run second half after the read


	u8 value = RawReadMemory(address);

	return value;
}

void Cpu::WriteMemory(u16 address, u8 value)
{
	RawWriteMemory(address, value);
}

u8 Cpu::RawReadMemory(u16 address)
{
	return memory[address];
}

void Cpu::RawWriteMemory(u16 address, u8 value)
{
	memory[address] = value;
}

void Cpu::InitCpu()
{
	MemorySet(memory, 0, sizeof(CpuMemorySize));

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
}

u8 Cpu::ReadOpcode()
{
	u8 opcode = ReadMemory(prgCounter);
	++prgCounter;
	return opcode;
}

u16 Cpu::GetOperand()
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
	else if(addressMode == AddressMode::IND)
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
	else if(addressMode == AddressMode::IND_Y)
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
		// Check to see if page has been crossed
		if((indAddress & 0xFF00) != (address & 0xFF00))
		{
			ReadMemory(indAddress - 0x100); // Duummy Read
		}
		return indAddress;
	}
	else if(addressMode == AddressMode::IND_YW)
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
		bool crossedPage = (indAddress & 0xFF00) != (address & 0xFF00);
		if(crossedPage)
		{
			ReadMemory(indAddress - 0x100); // Duummy Read
		}
		else
		{
			ReadMemory(indAddress);
		}
		return indAddress;
	}
}

void Cpu::Run()
{
	// NOTE: How cpu keeps track of clocks: Calling into a op will
	// execute all clocks of the instruction at once. If there is any
	// I/O reads or writes, then ppu and apu are run to catch up to
	// that point. To know where to catch up too, we have a running
	// total of 'catchup' clocks.  These clocks will be stored in the
	// cpu struct. If after running an instruction, the clocks that
	// need to be added to the 'catchup' total are returned.  After a
	// frames worth of clocks are run, then we display the frame and
	// update the audio on the platform
	opCode = ReadOpcode();
	opName = OpNames[opCode];

	addressMode = OpAddressModes[opCode];
	operand = GetOperand();


	OperationAddressModes[addressMode](cpu);
	

	if(_prevRunIrq || _prevNeedNmi) {
		IRQ();
		/*
	if(nmiInterruptSet)
	{
		nmiInterruptSet = false;
		cpu->opCode = NMI_OP;
	}
	else if(irqInterruptSet)
	{
		irqInterruptSet = false;
		cpu->opCode = IRQ_OP;
	}
*/
	}

}



