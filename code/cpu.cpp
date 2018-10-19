/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

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
inline void SetCarry(u8 *flags)       { *flags = *flags | CARRY_BIT; }
inline void ClearCarry(u8 *flags)     { *flags = *flags & ~CARRY_BIT; }
inline void SetInterrupt(u8 *flags)   { *flags = *flags | INTERRUPT_BIT; }
inline void ClearInterrupt(u8 *flags) { *flags = *flags & ~INTERRUPT_BIT; }
inline void SetDecimal(u8 *flags)     { *flags = *flags | DECIMAL_BIT; }
inline void ClearDecimal(u8 *flags)   { *flags = *flags & ~DECIMAL_BIT; }
inline void SetBreak(u8 *flags)       { *flags = *flags | BREAK_BIT; }
inline void ClearBreak(u8 *flags)     { *flags = *flags & ~BREAK_BIT; }
inline void SetBlank(u8 *flags)       { *flags = *flags | BLANK_BIT; }
inline void ClearBlank(u8 *flags)     { *flags = *flags & ~BLANK_BIT; }
inline void SetOverflow(u8 *flags)    { *flags = *flags | OVERFLOW_BIT; }
inline void ClearOverflow(u8 *flags)  { *flags = *flags & ~OVERFLOW_BIT; }
inline void SetZero(u8 value, u8 *flags) {
    *flags = (value == 0x00) ? (*flags | ZERO_BIT) : (*flags & ~ZERO_BIT);
}
inline void SetNegative(u8 value, u8 *flags) {
    *flags = (value >= 0x00 && value <= 0x7F) ? (*flags & ~NEGATIVE_BIT) : (*flags | NEGATIVE_BIT);
}
inline b32 IsBitSet(u8 bit, u8 flags) {
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

    FetchOpcode(cpu);
    OperationAddressModes[cpu->addressType](cpu);

    // Add how many clocks in the Op, minus any clocks already run for catchup.
	cpu->catchupClocks += (cpu->opClockTotal - cpu->lastClocksIntoOp);
	cpu->lastClocksIntoOp = 0;

    return(cpu->opClockTotal);
}
    
static void InitCpu(Cpu *cpu, u64 memoryBase)
{
    ZeroMemory((u8 *)memoryBase, Kilobytes(64));

    // DEBUG at moment. Matching FCEUX initial cpu memory state
    for(u16 index = 0; index < 0x2000; ++index)
    {
        if(index % 8 >= 4)
        {
            u8 *newAddress = (u8 *)(index + memoryBase);
            *newAddress = 0xFF;
        }
    }

    for(u16 index = 0x4008; index < 0x5000; ++index)
    {
        u8 *newAddress = (u8 *)(index + memoryBase);
        *newAddress = 0xFF;
    }
    
    *cpu = {};
    
	cpu->memoryBase = memoryBase;
	cpu->stackPtr = 0xFD;
	cpu->flags = 0x04;

	cpu->opName = "NUL";
}

