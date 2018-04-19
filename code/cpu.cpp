/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#include "cpu.h"

inline void writeStack(uint8 Byte, cpu *Cpu)
{
    writeCpu8(Byte, (uint16)Cpu->StackPtr | STACK_ADDRESS, Cpu);
    --Cpu->StackPtr;  
}
inline uint8 readStack(cpu *Cpu)
{
    ++Cpu->StackPtr;  
    uint8 Value = readCpu8((uint16)Cpu->StackPtr | STACK_ADDRESS, Cpu);
    return(Value);
}

// Status flag functions
inline void setCarry(uint8 *Flags)       { *Flags = *Flags | CARRY_BIT; }
inline void clearCarry(uint8 *Flags)     { *Flags = *Flags & ~CARRY_BIT; }
inline void setInterrupt(uint8 *Flags)   { *Flags = *Flags | INTERRUPT_BIT; }
inline void clearInterrupt(uint8 *Flags) { *Flags = *Flags & ~INTERRUPT_BIT; }
inline void setDecimal(uint8 *Flags)     { *Flags = *Flags | DECIMAL_BIT; }
inline void clearDecimal(uint8 *Flags)   { *Flags = *Flags & ~DECIMAL_BIT; }
inline void setBreak(uint8 *Flags)       { *Flags = *Flags | BREAK_BIT; }
inline void clearBreak(uint8 *Flags)     { *Flags = *Flags & ~BREAK_BIT; }
inline void setBlank(uint8 *Flags)       { *Flags = *Flags | BLANK_BIT; }
inline void clearBlank(uint8 *Flags)     { *Flags = *Flags & ~BLANK_BIT; }
inline void setOverflow(uint8 *Flags)    { *Flags = *Flags | OVERFLOW_BIT; }
inline void clearOverflow(uint8 *Flags)  { *Flags = *Flags & ~OVERFLOW_BIT; }
inline void setZero(uint8 Value, uint8 *Flags) {
    *Flags = (Value == 0x00) ? (*Flags | ZERO_BIT) : (*Flags & ~ZERO_BIT);
}
inline void setNegative(uint8 Value, uint8 *Flags) {
    *Flags = (Value >= 0x00 && Value <= 0x7F) ? (*Flags & ~NEGATIVE_BIT) : (*Flags | NEGATIVE_BIT);
}
inline bool32 isBitSet(uint8 Bit, uint8 Flags) {
    return((Bit & Flags) != 0);
}

#include "operations.cpp"

static void fetchOpcode(cpu *Cpu)
{
    if(NmiInterruptSet)
    {
        NmiInterruptSet = false;
        Cpu->OpCode = NMI_OP;
    }
    else if(IRQInterruptSet)
    {
        IRQInterruptSet = false;
        Cpu->OpCode = IRQ_OP;            
    }
    else
    {
        Cpu->OpCode = readCpu8(Cpu->PrgCounter++, Cpu);            
    }
    
    Cpu->AddressType = opAddressType[Cpu->OpCode];
    Cpu->OpName = opName[Cpu->OpCode];
    Cpu->OpClockTotal = opClocks[Cpu->OpCode];
}

static uint8 runCpu(cpu *Cpu, input *NewInput)
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

    fetchOpcode(Cpu);
    operationAddressModes[Cpu->AddressType](Cpu);

    // Add how many clocks in the Op, minus any clocks already run for catchup.
    Cpu->CatchupClocks += (Cpu->OpClockTotal - Cpu->LastClocksIntoOp);
    Cpu->LastClocksIntoOp = 0;

    return(Cpu->OpClockTotal);
}
    
static void
initCpu(cpu *Cpu, uint64 MemoryBase)
{
    ZeroMemory((uint8 *)MemoryBase, Kilobytes(64));

    // DEBUG at moment. Matching FCEUX initial cpu memory state
    for(uint16 index = 0; index < 0x2000; ++index)
    {
        if(index % 8 >= 4)
        {
            uint8 *NewAddress = (uint8 *)(index + MemoryBase);
            *NewAddress = 0xFF;
        }
    }

    for(uint16 index = 0x4008; index < 0x5000; ++index)
    {
        uint8 *NewAddress = (uint8 *)(index + MemoryBase);
        *NewAddress = 0xFF;
    }
    
    *Cpu = {};
    
    Cpu->MemoryBase = MemoryBase;
    Cpu->StackPtr = 0xFD;
    Cpu->Flags = 0x04;

    Cpu->OpName = "NUL";
}

