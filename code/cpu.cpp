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
    if(Value == 0x00)
        *Flags = *Flags | ZERO_BIT;
    else
        *Flags = *Flags & ~ZERO_BIT;
}
inline void setNegative(uint8 Value, uint8 *Flags)
{  
    if(Value >= 0x00 && Value <= 0x7F)
        *Flags = *Flags & ~NEGATIVE_BIT; // clear negative flag
    else
        *Flags = *Flags | NEGATIVE_BIT; // set negative flag
}

inline bool32 isBitSet(uint8 Bit, uint8 Flags)
{
    return((Bit & Flags) != 0);
}

global uint8 instAddressType[INSTRUCTION_COUNT] =
{
    /*         0        1      2        3       4       5        6        7     8       9     A        B       C     D     E     F  */
    /*0*/   IMPL,  INDX_R,  IMPL, INDX_RW, ZERO_R, ZERO_R, ZERO_RW, ZERO_RW, IMPL,   IMED,  ACM,    IMED,  ABS_R,  ABS_R,  ABS_RW,  ABS_RW,        
    /*1*/    REL,  INDY_R,  IMPL, INDY_RW, ZERX_R, ZERX_R, ZERX_RW, ZERX_RW, IMPL, ABSY_R, IMPL, ABSY_RW, ABSX_R, ABSX_R, ABSX_RW, ABSX_RW,
    /*2*/   IMPL,  INDX_R,  IMPL, INDX_RW, ZERO_R, ZERO_R, ZERO_RW, ZERO_RW, IMPL,   IMED,  ACM,    IMED,  ABS_R,  ABS_R,  ABS_RW,  ABS_RW,
    /*3*/    REL,  INDY_R,  IMPL, INDY_RW, ZERX_R, ZERX_R, ZERX_RW, ZERX_RW, IMPL, ABSY_R, IMPL, ABSY_RW, ABSX_R, ABSX_R, ABSX_RW, ABSX_RW,
    /*4*/   IMPL,  INDX_R,  IMPL, INDX_RW, ZERO_R, ZERO_R, ZERO_RW, ZERO_RW, IMPL,   IMED,  ACM,    IMED,   ABSJ,  ABS_R,  ABS_RW,  ABS_RW,
    /*5*/    REL,  INDY_R,  IMPL, INDY_RW, ZERX_R, ZERX_R, ZERX_RW, ZERX_RW, IMPL, ABSY_R, IMPL, ABSY_RW, ABSX_R, ABSX_R, ABSX_RW, ABSX_RW,
    /*6*/   IMPL,  INDX_R,  IMPL, INDX_RW, ZERO_R, ZERO_R, ZERO_RW, ZERO_RW, IMPL,   IMED,  ACM,    IMED,   INDI,  ABS_R,  ABS_RW,  ABS_RW,
    /*7*/    REL,  INDY_R,  IMPL, INDY_RW, ZERX_R, ZERX_R, ZERX_RW, ZERX_RW, IMPL, ABSY_R, IMPL, ABSY_RW, ABSX_R, ABSX_R, ABSX_RW, ABSX_RW,
    /*8*/   IMED,  INDX_W,  IMED,  INDX_W, ZERO_W, ZERO_W,  ZERO_W,  ZERO_W, IMPL,   IMED, IMPL,    IMED,  ABS_W,  ABS_W,   ABS_W,   ABS_W,
    /*9*/    REL,  INDY_W,  IMPL,  INDY_W, ZERX_W, ZERX_W,  ZERY_W,  ZERY_W, IMPL, ABSY_W, IMPL,  ABSY_W, ABSX_W, ABSX_W,  ABSY_W,  ABSY_W,
    /*A*/   IMED,  INDX_R,  IMED,  INDX_R, ZERO_R, ZERO_R,  ZERO_R,  ZERO_R, IMPL,   IMED, IMPL,    IMED,  ABS_R,  ABS_R,   ABS_R,   ABS_R,
    /*B*/    REL,  INDY_R,  IMPL,  INDY_R, ZERX_R, ZERX_R,  ZERY_R,  ZERY_R, IMPL, ABSY_R, IMPL,  ABSY_R, ABSX_R, ABSX_R,  ABSY_R,  ABSY_R,
    /*C*/   IMED,  INDX_R,  IMED, INDX_RW, ZERO_R, ZERO_R, ZERO_RW, ZERO_RW, IMPL,   IMED, IMPL,    IMED,  ABS_R,  ABS_R,  ABS_RW,  ABS_RW,
    /*D*/    REL,  INDY_R,  IMPL, INDY_RW, ZERX_R, ZERX_R, ZERX_RW, ZERX_RW, IMPL, ABSY_R, IMPL, ABSY_RW, ABSX_R, ABSX_R, ABSX_RW, ABSX_RW,
    /*E*/   IMED,  INDX_R,  IMED, INDX_RW, ZERO_R, ZERO_R, ZERO_RW, ZERO_RW, IMPL,   IMED, IMPL,    IMED,  ABS_R,  ABS_R,  ABS_RW,  ABS_RW,
    /*F*/    REL,  INDY_R,  IMPL, INDY_RW, ZERX_R, ZERX_R, ZERX_RW, ZERX_RW, IMPL, ABSY_R, IMPL, ABSY_RW, ABSX_R, ABSX_R, ABSX_RW, ABSX_RW,
};

global char * instName[INSTRUCTION_COUNT] =
{
    /*         0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F        */
    /*0*/  "BRK","ORA","KIL","SLO","NOP","ORA","ASL","SLO","PHP","ORA","ASL","ANC","NOP","ORA","ASL","SLO",
    /*1*/  "BPL","ORA","KIL","SLO","NOP","ORA","ASL","SLO","CLC","ORA","NOP","SLO","NOP","ORA","ASL","SLO",
    /*2*/  "JSR","AND","KIL","RLA","BIT","AND","ROL","RLA","PLP","AND","ROL","ANC","BIT","AND","ROL","RLA", 
    /*3*/  "BMI","AND","KIL","RLA","NOP","AND","ROL","RLA","SEC","AND","NOP","RLA","NOP","AND","ROL","RLA",
    /*4*/  "RTI","EOR","KIL","SRE","NOP","EOR","LSR","SRE","PHA","EOR","LSR","ALR","JMP","EOR","LSR","SRE",
    /*5*/  "BVC","EOR","KIL","SRE","NOP","EOR","LSR","SRE","CLI","EOR","NOP","SRE","NOP","EOR","LSR","SRE",
    /*6*/  "RTS","ADC","KIL","RRA","NOP","ADC","ROR","RRA","PLA","ADC","ROR","ARR","JMP","ADC","ROR","RRA",
    /*7*/  "BVS","ADC","KIL","RRA","NOP","ADC","ROR","RRA","SEI","ADC","NOP","RRA","NOP","ADC","ROR","RRA",
    /*8*/  "NOP","STA","NOP","SAX","STY","STA","STX","SAX","DEY","NOP","TXA","XAA","STY","STA","STX","SAX",
    /*9*/  "BCC","STA","KIL","AHX","STY","STA","STX","SAX","TYA","STA","TXS","TAS","SHY","STA","SHX","AHX",
    /*A*/  "LDY","LDA","LDX","LAX","LDY","LDA","LDX","LAX","TAY","LDA","TAX","LAX","LDY","LDA","LDX","LAX",
    /*B*/  "BCS","LDA","KIL","LAX","LDY","LDA","LDX","LAX","CLV","LDA","TSX","LAS","LDY","LDA","LDX","LAX",
    /*C*/  "CPY","CMP","NOP","DCP","CPY","CMP","DEC","DCP","INY","CMP","DEX","AXS","CPY","CMP","DEC","DCP",
    /*D*/  "BNE","CMP","KIL","DCP","NOP","CMP","DEC","DCP","CLD","CMP","NOP","DCP","NOP","CMP","DEC","DCP",
    /*E*/  "CPX","SBC","NOP","ISC","CPX","SBC","INC","ISC","INX","SBC","NOP","SBC","CPX","SBC","INC","ISC",
    /*F*/  "BEQ","SBC","KIL","ISC","NOP","SBC","INC","ISC","SED","SBC","NOP","ISC","NOP","SBC","INC","ISC"
};


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
                                   instName[Cpu->LogOp],
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


#include "operations.cpp"


static void nmi(cpu *Cpu)
{
    // Cycle 1 and 2 fetch next opcode and dicard it. The prgcounter is not incremented
    if(Cpu->Cycle == 3)
    {
        writeStack((Cpu->PrgCounter >> 8), Cpu);
        decrementStack(Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        writeStack((Cpu->PrgCounter & 0xFF), Cpu);
        decrementStack(Cpu);
    }
    else if(Cpu->Cycle == 5)
    {
        // At this point, which interrupt is detrmined. Can be hijacked
        // Current implementation is just seperating the nmi irq brk functions, may change
        clearBreak(&Cpu->Flags);
        writeStack(Cpu->Flags, Cpu);
        decrementStack(Cpu);
    }
    else if(Cpu->Cycle == 6)
    {
        Cpu->PrgCounter = (Cpu->PrgCounter & 0xFF00) | read8(NMI_VEC, Cpu->MemoryBase);        
    }
    else if(Cpu->Cycle == 7)
    {
        Cpu->PrgCounter = (read8(NMI_VEC+1, Cpu->MemoryBase) << 8) | (Cpu->PrgCounter & 0xFF);
        clearBreak(&Cpu->Flags);
        setInterrupt(&Cpu->Flags);
        Cpu->NextCycle = 1;
        ExecutingNmi = false;
    }
}

static void fetchOpcode(cpu *Cpu)
{
    Cpu->OpInstruction = readCpu8(Cpu->PrgCounter, Cpu);
    Cpu->AddressType = instAddressType[Cpu->OpInstruction];
    Cpu->InstrName = instName[Cpu->OpInstruction];

#if CPU_LOG
    Cpu->LogA = Cpu->A;
    Cpu->LogX = Cpu->X;
    Cpu->LogY = Cpu->Y;
    Cpu->LogSP = Cpu->StackPtr;
    Cpu->LogFlags = Cpu->Flags;
    Cpu->LogPC = Cpu->PrgCounter;
    Cpu->LogOp = Cpu->OpInstruction;
#endif
}

static uint8 cpuTick(cpu *Cpu, input *NewInput)
{    
    // NOTE: Cycle Zero checks for Interrupts. And logs?
    if(Cpu->Cycle == 0)
    {
#if CPU_LOG
        logCpu(Cpu); // Log last Op
#endif
        
        if(TriggerNmi)
        {
            TriggerNmi = false;
            ExecutingNmi = true;
            Cpu->AddressType = IMPL;
            Cpu->InstrName = "Nmi";
        }
        
        Cpu->Cycle = 1; // NOTE: Cycle zero isn't a real cycle, move to next one.
    }

    Cpu->NextCycle = Cpu->Cycle + 1;
    
    // Input read
    if(Cpu->PadStrobe)
    {
        for(uint8 idx = 0; idx < input::BUTTON_NUM; ++idx)
            Cpu->InputPad1.buttons[idx] = NewInput->buttons[idx];
    }
    
    if(ExecutingNmi)
    {
        nmi(Cpu);        
    }
    else
    {
        // If first cycle, then get instruction opcode. The operation handles incrementing PrgCounter
        if(Cpu->Cycle == 1)
        {            
            fetchOpcode(Cpu);
        }
        
        operationAddressModes[Cpu->AddressType](Cpu);
    }

    if(Cpu->Branched) // NOTE: If branched, then cycle one of next instruction is done on last relative cycle.
    {
        Cpu->Branched = false;
        
#if CPU_LOG
        logCpu(Cpu);
#endif

        // Check if interrupt happened before getting next instruction
        if(TriggerNmi)
        {
            TriggerNmi = false;
            ExecutingNmi = true;
            Cpu->AddressType = IMPL;
            Cpu->InstrName = "Nmi";
        }

        Cpu->Cycle = 1;
        Cpu->NextCycle = Cpu->Cycle + 1;
        
        if(ExecutingNmi)
        {
            nmi(Cpu);        
        }
        else
        {
            // NOTE: Branch happend so next code is fetched and first cycle executed. Is always cycle 1
            fetchOpcode(Cpu);            
            operationAddressModes[Cpu->AddressType](Cpu);
        }
    }

    Cpu->Cycle = Cpu->NextCycle;    
    return(1);
}


#if 0
            // NOTE: CPU Log options
            char LogBuffer[1024];
            sprintf(LogBuffer, "%4X %2X %s, SP=%2X\n", Cpu->PrgCounter, Cpu->OpInstruction, Cpu->InstrName, Cpu->StackPtr);
            OutputDebugString(LogBuffer);
#endif

#if 0
    char LogInstrData[16];
    if(InstrLength == 3)
        sprintf(LogInstrData, "%2X %2X %2X", InstrData[0], InstrData[1], InstrData[2]);
    else if(InstrLength == 2)
        sprintf(LogInstrData, "%2X %2X   ", InstrData[0], InstrData[1]);
    else
        sprintf(LogInstrData, "%2X      ", InstrData[0]);

    char LogOpInfo[64];
//    sprintf(LogOpInfo, ""
    
    char LogCpuInfo[64];
    sprintf(LogCpuInfo, "A:%2X X:%2X Y:%2X P:%2X SP:%2X  CYC: %d",
            LogCpu.A, LogCpu.X, LogCpu.Y, LogCpu.Flags, LogCpu.StackPtr, CyclesElapsed);

    // NOTE: CPU Log options
    char LogBuffer[1024];
    sprintf(LogBuffer, "%4X %s    %s\n", LogCpu.PrgCounter, LogInstrData, LogCpuInfo);
    OutputDebugString(LogBuffer);
#endif



