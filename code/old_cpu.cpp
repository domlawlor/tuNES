/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#include "cpu.h"

// Stack operations
inline void incrementStack(cpu *Cpu)
{
    ++Cpu->StackPtr;  
}

inline void decrementStack(cpu *Cpu)
{
    --Cpu->StackPtr;  
}

inline void writeStack(uint8 Byte, cpu *Cpu)
{
    writeCpu8(Byte, (uint16)Cpu->StackPtr | STACK_ADDRESS, Cpu);    
}
inline uint8 readStack(cpu *Cpu)
{
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
inline void setZero(uint8 Value, uint8 *Flags)
{
    *Flags = (Value == 0x00) ?
        (*Flags | ZERO_BIT) : (*Flags & ~ZERO_BIT);
}
inline void setNegative(uint8 Value, uint8 *Flags)
{  
    *Flags = (Value >= 0x00 && Value <= 0x7F) ?
        (*Flags & ~NEGATIVE_BIT) : (*Flags | NEGATIVE_BIT);  
}

inline bool32 isBitSet(uint8 Bit, uint8 Flags)
{
    return((Bit & Flags) != 0);
}


#include "operations.cpp"

/*
static void logCpu(cpu* Cpu)
{
    if(Cpu->LogHandle != INVALID_HANDLE_VALUE)
    {
        char logString[512];

        // NOTE: Go through each of the Cpu flags, and capitalise the coresponding letter
        //       in the string.     eg flags - 1000 0000, then string becomes Nvubdizc
        char flagString[9] = "nvubdizc";
        for(int i = 0; i < 8; ++i)
        {
            if(Cpu->LogFlags & (1 << (7 - i)))
            {
                flagString[i] -= 0x20;
            }
        }
        
        uint32 byteCount = sprintf(logString,
                                   "A:%02X X:%02X Y:%02X S:%02X P:%s  $%04X:%02X %2s %2s  %s%s\n",
                                   Cpu->LogA, Cpu->LogX, Cpu->LogY,
                                   Cpu->LogSP, flagString, 
                                   Cpu->LogPC, Cpu->LogOp,
                                   Cpu->LogData1, Cpu->LogData2,
                                   opName[Cpu->LogOp],
                                   Cpu->LogExtraInfo);

        uint32 bytesWritten;
        
        if(!writeLog(logString, byteCount, &bytesWritten, Cpu->LogHandle))
        {
            // Failed
        }
    }
    
    Cpu->LogData1[0] = '\0';
    Cpu->LogData2[0] = '\0';
    Cpu->LogExtraInfo[0] = '\0';
}
*/


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
        Cpu->OpCode = readCpu8(Cpu->PrgCounter, Cpu);            
    }
    
    Cpu->AddressType = opAddressType[Cpu->OpCode];
    Cpu->OpName = opName[Cpu->OpCode];
        
#if CPU_LOG
    Cpu->LogA = Cpu->A;
    Cpu->LogX = Cpu->X;
    Cpu->LogY = Cpu->Y;
    Cpu->LogSP = Cpu->StackPtr;
    Cpu->LogFlags = Cpu->Flags;
    Cpu->LogPC = Cpu->PrgCounter;
    Cpu->LogOp = Cpu->OpCode;
#endif
}

static uint8 cpuTick(cpu *Cpu, input *NewInput)
{    
    Cpu->NextCycle = Cpu->Cycle + 1;
    
    // Input read
    if(Cpu->PadStrobe)
    {
        for(uint8 idx = 0; idx < input::BUTTON_NUM; ++idx)
            Cpu->InputPad1.buttons[idx] = NewInput->buttons[idx];
    }

    // If first cycle, then get instruction opcode. The operation handles incrementing PrgCounter
    if(Cpu->Cycle == 1)
    {            
#if CPU_LOG
        //logCpu(Cpu); // Log last Op
#endif

        fetchOpcode(Cpu);
    }
    
    operationAddressModes[Cpu->AddressType](Cpu);
    
    if(Cpu->OpBranched) // NOTE: If branched, then cycle one of next instruction is done on last relative cycle.
    {
        Cpu->OpBranched = false;
        Cpu->Cycle = 1;

        // If branched, then run the next cycle now. It was pipelined and executed the last cycle of branching
        cpuTick(Cpu, NewInput); // NOTE: Recursion to run the tick. TODO: Each tick reads input(padstrobe), should this happen again??
    }

    Cpu->Cycle = Cpu->NextCycle;    
    return(1);
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
    Cpu->Cycle = 1;
    Cpu->StackPtr = 0xFD;
    Cpu->Flags = 0x04;

    Cpu->OpName = "NUL";
    
#if CPU_LOG
    Cpu->LogHandle = createLog("cpu.log");
#endif
}
