#include "nes.h"

#include "cpu.h"

inline void WriteStack(u8 Byte, Cpu *cpu)
{
	WriteCpu8(Byte, (u16)cpu->stackPtr | STACK_ADDRESS, cpu);
	--cpu->stackPtr;
}
inline u8 readStack(Cpu *cpu)
{
	++cpu->stackPtr;
	u8 value = ReadCpu8((u16)cpu->stackPtr | STACK_ADDRESS, cpu);
	return(value);
}

// Status flag functions
inline void SetCarry(u8 *flags) { *flags = *flags | CARRY_BIT; }
inline void ClearCarry(u8 *flags) { *flags = *flags & ~CARRY_BIT; }
inline void SetInterrupt(u8 *flags) { *flags = *flags | INTERRUPT_BIT; }
inline void ClearInterrupt(u8 *flags) { *flags = *flags & ~INTERRUPT_BIT; }
inline void SetDecimal(u8 *flags) { *flags = *flags | DECIMAL_BIT; }
inline void ClearDecimal(u8 *flags) { *flags = *flags & ~DECIMAL_BIT; }
inline void SetBreak(u8 *flags) { *flags = *flags | BREAK_BIT; }
inline void ClearBreak(u8 *flags) { *flags = *flags & ~BREAK_BIT; }
inline void SetBlank(u8 *flags) { *flags = *flags | BLANK_BIT; }
inline void ClearBlank(u8 *flags) { *flags = *flags & ~BLANK_BIT; }
inline void SetOverflow(u8 *flags) { *flags = *flags | OVERFLOW_BIT; }
inline void ClearOverflow(u8 *flags) { *flags = *flags & ~OVERFLOW_BIT; }
inline void SetZero(u8 value, u8 *flags) {
	*flags = (value == 0x00) ? (*flags | ZERO_BIT) : (*flags & ~ZERO_BIT);
}
inline void SetNegative(u8 value, u8 *flags) {
	*flags = (value >= 0x00 && value <= 0x7F) ? (*flags & ~NEGATIVE_BIT) : (*flags | NEGATIVE_BIT);
}
inline bool IsBitSet(u8 bit, u8 flags) {
	return((bit & flags) != 0);
}

#include "operations.cpp"

static void FetchOpcode(Cpu *cpu)
{
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
	else
	{
		cpu->opCode = ReadCpu8(cpu->prgCounter++, cpu);
	}

	cpu->addressType = OpAddressType[cpu->opCode];
	cpu->opName = OpName[cpu->opCode];
	cpu->opClockTotal = OpClocks[cpu->opCode];
}

static u8 RunCpu(Cpu *cpu, Input *newInput)
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


#if CPU_LOG
	cpu->logA = cpu->A;
	cpu->logX = cpu->X;
	cpu->logY = cpu->Y;
	cpu->logFlags = cpu->flags;
	cpu->logPC = cpu->prgCounter;
	cpu->logSP = cpu->stackPtr;
#endif

	FetchOpcode(cpu);

	OperationAddressModes[cpu->addressType](cpu);

	// Add how many clocks in the Op, minus any clocks already run for catchup.
	cpu->catchupClocks += (cpu->opClockTotal - cpu->lastClocksIntoOp);
	cpu->lastClocksIntoOp = 0;

#if CPU_LOG
	cpu->logOp = cpu->opCode;
	cpu->logOpName = cpu->opName;

	char logString[256] = {};
	snprintf(logString, 256, "0x%04X %s - %02X - A:%02X X:%02X Y:%02X - Flags:%02X SP:%02X\n",
		cpu->logPC, cpu->logOpName, cpu->logOp, cpu->logA, cpu->logX, cpu->logY, cpu->logFlags, cpu->logSP);
	u32 bytesToWrite =  strlen(logString);

	u32 bytesWritten = 0;
	WriteLog(logString, bytesToWrite, &bytesWritten, globalNes.logCpuHandle);
#endif

	return(cpu->opClockTotal);
}

static void InitCpu(Cpu *cpu)
{
	ZeroMemory(cpu, sizeof(Cpu));

	// DEBUG at moment. Matching FCEUX initial cpu memory state
	for(u16 index = 0; index < 0x2000; ++index)
	{
		if(index % 8 >= 4)
		{
			cpu->memory[index] = 0xFF;
		}
	}

	for(u16 index = 0x4008; index < 0x5000; ++index)
	{
		cpu->memory[index] = 0xFF;
	}

	cpu->stackPtr = 0xFD;
	cpu->flags = 0x04;

	cpu->opName = "NUL";
}

