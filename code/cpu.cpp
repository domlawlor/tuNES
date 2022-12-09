#include "nes.h"

#include "cpu.h"

#include <string.h>

void Cpu::UpdateInterrupts()
{
	prevTriggerNmi = triggerNmi;
	if(!prevNmiSet && nmiSet)
	{
		triggerNmi = true;
	}
	prevNmiSet = nmiSet;

	prevTriggerIrq = triggerIrq;
	if(!IsFlagBitSet(INTERRUPT_BIT))
	{
		triggerIrq = (irqFlag & activeIrqs) ? 1 : 0;
	}
}


u8 Cpu::RawReadMemory(u16 address)
{
	if(address < 0x2000) // ram address
	{
		address = (address % 0x0800); // NOTE: address mirrored for the 2kb ram until 0x2000
		return ramMemory[address];
	}
	else if(address >= 0x2000 && address < 0x4000) // Ppu registers
	{
		address = (address % 8) + 0x2000; // Mirror ppu registers
		return Nes::GetPpu()->ReadRegisters(address);
	} 
	else if(address == 0x4014) // No mirroring
	{
		return Nes::GetPpu()->ReadRegisters(address);
	}
	else if((address >= 0x4000 && address <= 0x4013) || address == 0x4015)
	{
		return 0;
		// Apu
	}
	else if(address == 0x4016 || address == 0x4017)
	{
		Input *input = Nes::GetInput();

		u8 btnValue;
		if(address == 0x4016)
		{
			if(input->pad1CurrentButton > Input::BUTTON_NUM)
			{
				btnValue = 1;
			}
			else
			{
				btnValue = input->pad1Buttons[input->pad1CurrentButton] ? 1 : 0;
			}
		}
		else
		{
			if(input->pad2CurrentButton > Input::BUTTON_NUM)
			{
				btnValue = 1;
			}
			else
			{
				btnValue = input->pad2Buttons[input->pad2CurrentButton] ? 1 : 0;
			}
		}

		// each read will move to the next button
		if(!input->padStrobe)
		{
			if(address == 0x4016) { ++input->pad1CurrentButton; }
			else { ++input->pad2CurrentButton; }
		}

		return 0x40 | btnValue;
	}
	else if(address >= 0x6000)
	{
		Cartridge *cartridge = Nes::GetCartridge();
		return cartridge->ReadMemory(address);
	}

	return 0;
}

void Cpu::RawWriteMemory(u16 address, u8 value)
{
	if(address < 0x2000) // ram address
	{
		address = (address % 0x0800); // address mirrored for the 2kb ram until 0x2000
		ramMemory[address] = value;
	}
	else if(address >= 0x2000 && address < 0x4000) // Ppu registers
	{
		address = (address % 8) + 0x2000; // Mirror ppu registers
		Nes::GetPpu()->WriteRegisters(address, value);
	}
	else if(address == 0x4014) // No mirroring
	{
		Nes::GetPpu()->WriteRegisters(address, value);
	}
	else if((address >= 0x4000 && address <= 0x4013) || address == 0x4015 || address == 0x4017)
	{
		// Apu
	}
	else if(address == 0x4016)
	{
		Input *input = Nes::GetInput();
		input->padStrobe = (value & 1) > 0;
		input->pad1CurrentButton = Input::B_A;
		input->pad2CurrentButton = Input::B_A;
	}
	else if(address >= 0x6000)
	{
		Nes::GetCartridge()->WriteMemory(address, value);
	}
}

void Cpu::RunPreMemoryCycles(bool isRead)
{
	cycle += 1;

	masterClock += isRead ? preReadClockCycles : preWriteClockCycles;
	Nes::GetPpu()->RunCatchup(masterClock);
	//_console->ProcessCpuClock();
}

void Cpu::RunPostMemoryCycles(bool isRead)
{
	masterClock += isRead ? postReadClockCycles : postWriteClockCycles;
	Nes::GetPpu()->RunCatchup(masterClock);
	UpdateInterrupts();
}

u8 Cpu::ReadMemory(u16 address)
{
	// One memory read is 1 cycles worth. 
	// Run other systems (Ppu, Apu) for half of this Cpu cycle, then run second half after the read

	if(cpuHaltQueued)
	{
		RunActiveDMA(address);
	}

	RunPreMemoryCycles(true);

	u8 value = RawReadMemory(address);

	RunPostMemoryCycles(true);

	return value;
}

void Cpu::WriteMemory(u16 address, u8 value)
{
	// Cycle accuracy inspired by mesen
	RunPreMemoryCycles(false);
	RawWriteMemory(address, value);
	RunPostMemoryCycles(false);
}

Cpu::Cpu()
{
	// Copy operations into CPU operation buffer
	// Must happen at runtime when we have an instance
	Operation operationsFuncs[INSTRUCTION_COUNT] =
	{
		/*          0          1          2          3          4          5          6          7          8          9          A          B          C          D          E          F   */
		/*0*/ &Cpu::BRK, &Cpu::ORA, &Cpu::KIL, &Cpu::SLO, &Cpu::SKB, &Cpu::ORA, &Cpu::ASL, &Cpu::SLO, &Cpu::PHP, &Cpu::ORA, &Cpu::ASL, &Cpu::ANC, &Cpu::SKW, &Cpu::ORA, &Cpu::ASL, &Cpu::SLO,
		/*1*/ &Cpu::BPL, &Cpu::ORA, &Cpu::KIL, &Cpu::SLO, &Cpu::SKB, &Cpu::ORA, &Cpu::ASL, &Cpu::SLO, &Cpu::CLC, &Cpu::ORA, &Cpu::NOP, &Cpu::SLO, &Cpu::SKW, &Cpu::ORA, &Cpu::ASL, &Cpu::SLO,
		/*2*/ &Cpu::JSR, &Cpu::AND, &Cpu::KIL, &Cpu::RLA, &Cpu::BIT, &Cpu::AND, &Cpu::ROL, &Cpu::RLA, &Cpu::PLP, &Cpu::AND, &Cpu::ROL, &Cpu::ANC, &Cpu::BIT, &Cpu::AND, &Cpu::ROL, &Cpu::RLA,
		/*3*/ &Cpu::BMI, &Cpu::AND, &Cpu::KIL, &Cpu::RLA, &Cpu::SKB, &Cpu::AND, &Cpu::ROL, &Cpu::RLA, &Cpu::SEC, &Cpu::AND, &Cpu::NOP, &Cpu::RLA, &Cpu::SKW, &Cpu::AND, &Cpu::ROL, &Cpu::RLA,
		/*4*/ &Cpu::RTI, &Cpu::EOR, &Cpu::KIL, &Cpu::SRE, &Cpu::SKB, &Cpu::EOR, &Cpu::LSR, &Cpu::SRE, &Cpu::PHA, &Cpu::EOR, &Cpu::LSR, &Cpu::ALR, &Cpu::JMP, &Cpu::EOR, &Cpu::LSR, &Cpu::SRE,
		/*5*/ &Cpu::BVC, &Cpu::EOR, &Cpu::KIL, &Cpu::SRE, &Cpu::SKB, &Cpu::EOR, &Cpu::LSR, &Cpu::SRE, &Cpu::CLI, &Cpu::EOR, &Cpu::NOP, &Cpu::SRE, &Cpu::SKW, &Cpu::EOR, &Cpu::LSR, &Cpu::SRE,
		/*6*/ &Cpu::RTS, &Cpu::ADC, &Cpu::KIL, &Cpu::RRA, &Cpu::SKB, &Cpu::ADC, &Cpu::ROR, &Cpu::RRA, &Cpu::PLA, &Cpu::ADC, &Cpu::ROR, &Cpu::ARR, &Cpu::JMP, &Cpu::ADC, &Cpu::ROR, &Cpu::RRA,
		/*7*/ &Cpu::BVS, &Cpu::ADC, &Cpu::KIL, &Cpu::RRA, &Cpu::SKB, &Cpu::ADC, &Cpu::ROR, &Cpu::RRA, &Cpu::SEI, &Cpu::ADC, &Cpu::NOP, &Cpu::RRA, &Cpu::SKW, &Cpu::ADC, &Cpu::ROR, &Cpu::RRA,
		/*8*/ &Cpu::SKB, &Cpu::STA, &Cpu::SKB, &Cpu::SAX, &Cpu::STY, &Cpu::STA, &Cpu::STX, &Cpu::SAX, &Cpu::DEY, &Cpu::SKB, &Cpu::TXA, &Cpu::XAA, &Cpu::STY, &Cpu::STA, &Cpu::STX, &Cpu::SAX,
		/*9*/ &Cpu::BCC, &Cpu::STA, &Cpu::KIL, &Cpu::AHX, &Cpu::STY, &Cpu::STA, &Cpu::STX, &Cpu::SAX, &Cpu::TYA, &Cpu::STA, &Cpu::TXS, &Cpu::TAS, &Cpu::SHY, &Cpu::STA, &Cpu::SHX, &Cpu::AHX,
		/*A*/ &Cpu::LDY, &Cpu::LDA, &Cpu::LDX, &Cpu::LAX, &Cpu::LDY, &Cpu::LDA, &Cpu::LDX, &Cpu::LAX, &Cpu::TAY, &Cpu::LDA, &Cpu::TAX, &Cpu::LAX, &Cpu::LDY, &Cpu::LDA, &Cpu::LDX, &Cpu::LAX,
		/*B*/ &Cpu::BCS, &Cpu::LDA, &Cpu::KIL, &Cpu::LAX, &Cpu::LDY, &Cpu::LDA, &Cpu::LDX, &Cpu::LAX, &Cpu::CLV, &Cpu::LDA, &Cpu::TSX, &Cpu::LAS, &Cpu::LDY, &Cpu::LDA, &Cpu::LDX, &Cpu::LAX,
		/*C*/ &Cpu::CPY, &Cpu::CMP, &Cpu::SKB, &Cpu::DCP, &Cpu::CPY, &Cpu::CMP, &Cpu::DEC, &Cpu::DCP, &Cpu::INY, &Cpu::CMP, &Cpu::DEX, &Cpu::AXS, &Cpu::CPY, &Cpu::CMP, &Cpu::DEC, &Cpu::DCP,
		/*D*/ &Cpu::BNE, &Cpu::CMP, &Cpu::KIL, &Cpu::DCP, &Cpu::SKB, &Cpu::CMP, &Cpu::DEC, &Cpu::DCP, &Cpu::CLD, &Cpu::CMP, &Cpu::NOP, &Cpu::DCP, &Cpu::SKW, &Cpu::CMP, &Cpu::DEC, &Cpu::DCP,
		/*E*/ &Cpu::CPX, &Cpu::SBC, &Cpu::SKB, &Cpu::ISC, &Cpu::CPX, &Cpu::SBC, &Cpu::INC, &Cpu::ISC, &Cpu::INX, &Cpu::SBC, &Cpu::NOP, &Cpu::SBC, &Cpu::CPX, &Cpu::SBC, &Cpu::INC, &Cpu::ISC,
		/*F*/ &Cpu::BEQ, &Cpu::SBC, &Cpu::KIL, &Cpu::ISC, &Cpu::SKB, &Cpu::SBC, &Cpu::INC, &Cpu::ISC, &Cpu::SED, &Cpu::SBC, &Cpu::NOP, &Cpu::ISC, &Cpu::SKW, &Cpu::SBC, &Cpu::INC, &Cpu::ISC
	};

	for(u16 opNum = 0; opNum < INSTRUCTION_COUNT; ++opNum)
	{
		operations[opNum] = operationsFuncs[opNum];
	}

	logFile = fopen("cpu.log", "w+");
}

Cpu::~Cpu()
{
	fclose(logFile);
	logFile = nullptr;
}


void Cpu::Reset(bool powerCycle)
{
	MemorySet(ramMemory, 0, CpuRamSize);

	A = 0;
	X = 0;
	Y = 0;
	flags = 0x04;
	stackPointer = 0xFD;
	prgCounter = (RawReadMemory(RESET_VEC + 1) << 8) | RawReadMemory(RESET_VEC); // Read the reset vector directly. No need for ppu catchup cycles
	opName = "NUL";

	if(!powerCycle)
	{
		// NOTE: The status after reset was taken from nesdev
		stackPointer -= 3;
		SetInterrupt();
	}

	u8 preCatchupClockCycles = 6;
	u8 postCatchupClockCycles = 6;
	preReadClockCycles = preCatchupClockCycles - 1;
	postReadClockCycles = postCatchupClockCycles + 1;
	preWriteClockCycles = preCatchupClockCycles + 1;
	postWriteClockCycles = postCatchupClockCycles - 1;

	masterClock += 12;

	// Cpu just idles 8 cycles before reading any game data
	constexpr u8 idleCycles = 8;
	for(int i = 0; i < idleCycles; i++)
	{
		masterClock += preCatchupClockCycles + postCatchupClockCycles;
		Nes::GetPpu()->RunCatchup(masterClock);
		UpdateInterrupts();
	}
}

u16 Cpu::ReadOperand()
{
	if(addressMode == AddressMode::ACM || addressMode == AddressMode::IMPL)
	{
		ReadMemory(prgCounter); // Dummy Read for cycle accuracy
		return 0;
	}
	else if(addressMode == AddressMode::IMED || addressMode == AddressMode::REL
		|| addressMode == AddressMode::ZERO)
	{
		opValue1 = ReadMemory(prgCounter);
		prgCounter++;
		return opValue1;
	}
	else if(addressMode == AddressMode::ZERO_X)
	{
		opValue1 = ReadMemory(prgCounter);
		prgCounter++;
		ReadMemory(opValue1); // Dummy read for cycle accuracy
		return opValue1 + X;
	}
	else if(addressMode == AddressMode::ZERO_Y)
	{
		opValue1 = ReadMemory(prgCounter);
		prgCounter++;
		ReadMemory(opValue1); // Dummy read for cycle accuracy
		return opValue1 + Y;
	}
	else if(addressMode == AddressMode::IND || addressMode == AddressMode::ABS)
	{
		opValue1 = ReadMemory(prgCounter);
		opValue2 = ReadMemory(prgCounter + 1);
		u16 value = (opValue2 << 8) | opValue1;
		prgCounter += 2;
		return value;
	}
	else if(addressMode == AddressMode::IND_X)
	{
		opValue1 = ReadMemory(prgCounter);
		u8 indBase = opValue1;
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
		opValue1 = ReadMemory(prgCounter);
		u8 ind = opValue1;
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
		opValue1 = ReadMemory(prgCounter);
		opValue2 = ReadMemory(prgCounter + 1);

		u16 address = (opValue2 << 8) | opValue1;
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
		opValue1 = ReadMemory(prgCounter);
		opValue2 = ReadMemory(prgCounter + 1);

		u16 address = (opValue2 << 8) | opValue1;
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

	// For logging
	lastPrgCounter = prgCounter;

	Ppu *ppu = Nes::GetPpu();
	logPpuCycle = ppu->GetCycle();
	logPpuScanline = ppu->GetScanline();
	if(logPpuScanline == 261) { logPpuScanline = -1; }

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

	LogOp(); // Log Op before executing

	(this->*operations[opCode])();

	if(prevTriggerNmi) { NMI(); }
	else if(prevTriggerIrq) { IRQ(); }
}



void Cpu::NMI()
{
	ReadMemory(prgCounter); // Dummy Read
	ReadMemory(prgCounter); // Dummy Read

	triggerNmi = false;

	u8 highByte = (prgCounter >> 8) & 0xFF;
	u8 lowByte = prgCounter & 0xFF;
	PushStack(highByte);
	PushStack(lowByte);

	
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


void Cpu::RunActiveDMA(u16 address)
{
	RunPreMemoryCycles(true);
	RawReadMemory(address); // Dummy read
	RunPostMemoryCycles(true);

	cpuHaltQueued = false;

	uint16_t spriteDmaCounter = 0;
	uint8_t spriteReadAddr = 0;
	uint8_t readValue = 0;
	bool skipDummyReads = (address == 0x4016 || address == 0x4017);

	auto processCycle = [this] {
		//Sprite DMA cycles count as halt/dummy cycles for the DMC DMA when both run at the same time
		if(cpuHaltQueued)
		{
			cpuHaltQueued = false;
		}
		else if(dmcDmaNeedDummyRead)
		{
			dmcDmaNeedDummyRead = false;
		}
		RunPreMemoryCycles(true);
	};

	while(dmcDmaActive || spriteDmaActive)
	{
		bool isOddCycle = (cycle % 2) > 0;
		if(isOddCycle)
		{
			if(dmcDmaActive && !cpuHaltQueued && !dmcDmaNeedDummyRead) {
				/*
				//DMC DMA is ready to read a byte (both halt and dummy read cycles were performed before this)
				processCycle();
				readValue = RawReadMemory(Nes::GetApu()->GetDmcReadAddress()); // Apu DMC read
				RunPostMemoryCycles(true);
				_console->GetApu()->SetDmcReadBuffer(readValue);
				_dmcDmaRunning = false;
				*/
			}
			else if(spriteDmaActive) {
				//DMC DMA is not running, or not ready, run sprite DMA
				processCycle();
				readValue = RawReadMemory(spriteDmaOffset * 0x100 + spriteReadAddr);
				RunPostMemoryCycles(true);
				spriteReadAddr++;
				spriteDmaCounter++;
			}
			else {
				//DMC DMA is running, but not ready (need halt/dummy read) and sprite DMA isn't runnnig, perform a dummy read
				Assert(cpuHaltQueued || dmcDmaNeedDummyRead);
				processCycle();
				if(!skipDummyReads) {
					RawReadMemory(address); // Dummy Read
				}
				RunPostMemoryCycles(true);
			}
		}
		else
		{
			if(spriteDmaActive && (spriteDmaCounter & 0x01))
			{
				//Sprite DMA write cycle (only do this if a sprite dma read was performed last cycle)
				processCycle();
				Nes::GetPpu()->WriteOAMValue(readValue);
				RunPostMemoryCycles(true);
				spriteDmaCounter++;
				if(spriteDmaCounter == 0x200) {
					spriteDmaActive = false;
				}
			}
			else
			{
				//Align to read cycle before starting sprite DMA (or align to perform DMC read)
				processCycle();
				if(!skipDummyReads)
				{
					RawReadMemory(address); // Dummy Read
				}
				RunPostMemoryCycles(true);
			}
		}
	}
}

void Cpu::StartDMAWrite(u8 value)
{
	cpuHaltQueued = true;
	spriteDmaActive = true;
	spriteDmaOffset = value;
}

void Cpu::StartApuDMCWrite()
{
	cpuHaltQueued = true;
	dmcDmaActive = true;
	dmcDmaNeedDummyRead = true;
}


void Cpu::LogOp()
{
	// PrgCounter OpCode Op1 Op2
	constexpr u8 logStringLength = 128;
	char logString[logStringLength];

	switch(addressMode)
	{
	case AddressMode::ACM:
	case AddressMode::IMPL:
	{
		// eg "C5A5 $88         A:00 X:00 Y:58 P:06 SP:FD"
		const char *formatString = "%04X $%02X         A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%-3u SL:%-3d\n";
		sprintf(logString, formatString, lastPrgCounter, opCode, A, X, Y, flags, stackPointer, logPpuCycle, logPpuScanline);
		break;
	}
	case AddressMode::IMED:
	case AddressMode::REL:
	case AddressMode::ZERO:
	case AddressMode::ZERO_X:
	case AddressMode::ZERO_Y:
	case AddressMode::IND_X:
	case AddressMode::IND_Y:
	case AddressMode::IND_YW:
	{
		// eg "C5A3 $91 $1F     A:00 X:00 Y:5A P:06 SP:FD"
		const char *formatString = "%04X $%02X $%02X     A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%-3u SL:%-3d\n";
		sprintf(logString, formatString, lastPrgCounter, opCode, opValue1, A, X, Y, flags, stackPointer, logPpuCycle, logPpuScanline);
		break;
	}
	case AddressMode::ABS:
	case AddressMode::ABS_X:
	case AddressMode::ABS_XW:
	case AddressMode::ABS_Y:
	case AddressMode::ABS_YW:
	case AddressMode::IND:
	{
		// eg "C008 $AD $02 $20 A:00 X:00 Y:00 P:06 SP:F5"
		const char *formatString = "%04X $%02X $%02X $%02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%-3u SL:%-3d\n";
		sprintf(logString, formatString, lastPrgCounter, opCode, opValue1, opValue2, A, X, Y, flags, stackPointer, logPpuCycle, logPpuScanline);
		break;
	}
	}

	u32 logStringSize = strlen(logString);

	fwrite(logString, 1, logStringSize, logFile);

	//TraceLog(LOG_INFO, logString);
}
