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

inline bool32 isBitSet(uint8 Bit, uint8 Flags) { return(Bit & Flags); }
inline bool32 crossedPageCheck(uint16 Before, uint16 Now) { return((Before & 0xFF00) != (Now & 0xFF00));}

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
        setInterrupt(&Cpu->Flags);
        Cpu->NextCycle = 1;
        Nmi.ExecutingNmi = false;
    }
}

#if 0
        char Buf[1024];
        sprintf(Buf, "%X\n", WriteValue);
        OutputDebugString(Buf);
#endif   

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
        Cpu->OpInstruction = readCpu8(Cpu->PrgCounter, Cpu);
    }

    if(Nmi.ExecutingNmi)
    {
        nmi(Cpu);        
    }
    else
    {
        Cpu->AddressType = instAddressType[Cpu->OpInstruction];
        Cpu->InstrName = instName[Cpu->OpInstruction];
        operationAddressModes[Cpu->AddressType](Cpu);
    }

    // NOTE: If Next cycle is the same or less than the current, then new op about to be run
    //       Check for interrupts
    if(Cpu->NextCycle <= Cpu->Cycle)
    {
        if(Nmi.NmiInterrupt)
        {
            Nmi.NmiInterrupt = false;
            Nmi.ExecutingNmi = true;
            Cpu->AddressType = IMPL;
            Cpu->InstrName = "Nmi";

            OutputDebugString("NMI!!!\n");
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



