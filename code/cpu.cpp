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
    /*         0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F  */
    /*0*/   IMPL, INDX, IMPL, INDX, ZERO, ZERO, ZERO, ZERO, IMPL, IMED,  ACM, IMED,  ABS,  ABS,  ABS,  ABS,        
    /*1*/    REL, INDY, IMPL, INDY, ZERX, ZERX, ZERX, ZERX, IMPL, ABSY, IMPL, ABSY, ABSX, ABSX, ABSX, ABSX,
    /*2*/   IMPL, INDX, IMPL, INDX, ZERO, ZERO, ZERO, ZERO, IMPL, IMED,  ACM, IMED,  ABS,  ABS,  ABS,  ABS,
    /*3*/    REL, INDY, IMPL, INDY, ZERX, ZERX, ZERX, ZERX, IMPL, ABSY, IMPL, ABSY, ABSX, ABSX, ABSX, ABSX,
    /*4*/   IMPL, INDX, IMPL, INDX, ZERO, ZERO, ZERO, ZERO, IMPL, IMED,  ACM, IMED, ABSJ,  ABS,  ABS,  ABS,
    /*5*/    REL, INDY, IMPL, INDY, ZERX, ZERX, ZERX, ZERX, IMPL, ABSY, IMPL, ABSY, ABSX, ABSX, ABSX, ABSX,
    /*6*/   IMPL, INDX, IMPL, INDX, ZERO, ZERO, ZERO, ZERO, IMPL, IMED,  ACM, IMED, INDI,  ABS,  ABS,  ABS,
    /*7*/    REL, INDY, IMPL, INDY, ZERX, ZERX, ZERX, ZERX, IMPL, ABSY, IMPL, ABSY, ABSX, ABSX, ABSX, ABSX,
    /*8*/   IMED, INDX, IMED, INDX, ZERO, ZERO, ZERO, ZERO, IMPL, IMED, IMPL, IMED,  ABS,  ABS,  ABS,  ABS,
    /*9*/    REL, INDY, IMPL, INDY, ZERX, ZERX, ZERY, ZERY, IMPL, ABSY, IMPL, ABSY, ABSX, ABSX, ABSY, ABSY,
    /*A*/   IMED, INDX, IMED, INDX, ZERO, ZERO, ZERO, ZERO, IMPL, IMED, IMPL, IMED,  ABS,  ABS,  ABS,  ABS,
    /*B*/    REL, INDY, IMPL, INDY, ZERX, ZERX, ZERY, ZERY, IMPL, ABSY, IMPL, ABSY, ABSX, ABSX, ABSY, ABSY,
    /*C*/   IMED, INDX, IMED, INDX, ZERO, ZERO, ZERO, ZERO, IMPL, IMED, IMPL, IMED,  ABS,  ABS,  ABS,  ABS,
    /*D*/    REL, INDY, IMPL, INDY, ZERX, ZERX, ZERX, ZERX, IMPL, ABSY, IMPL, ABSY, ABSX, ABSX, ABSX, ABSX,
    /*E*/   IMED, INDX, IMED, INDX, ZERO, ZERO, ZERO, ZERO, IMPL, IMED, IMPL, IMED,  ABS,  ABS,  ABS,  ABS,
    /*F*/    REL, INDY, IMPL, INDY, ZERX, ZERX, ZERX, ZERX, IMPL, ABSY, IMPL, ABSY, ABSX, ABSX, ABSX, ABSX,
};

global uint8 instAddressMode[INSTRUCTION_COUNT] =
{
    /*        0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F  */
    /*0*/    NL,  R, NL, RW,  R,  R, RW, RW, NL,  R, RW, NL,  R,  R, RW, RW,
    /*1*/    NL,  R, NL, RW,  R,  R, RW, RW, NL,  R,  R, RW,  R,  R, RW, RW,
    /*2*/    NL,  R, NL, RW,  R,  R, RW, RW, NL,  R, RW, NL,  R,  R, RW, RW,
    /*3*/    NL,  R, NL, RW,  R,  R, RW, RW, NL,  R,  R, RW,  R,  R, RW, RW,
    /*4*/    NL,  R, NL, RW,  R,  R, RW, RW, NL,  R, RW, NL, NL,  R, RW, RW,
    /*5*/    NL,  R, NL, RW,  R,  R, RW, RW, NL,  R,  R, RW,  R,  R, RW, RW,
    /*6*/    NL,  R, NL, RW,  R,  R, RW, RW, NL,  R, RW, NL, NL,  R, RW, RW,
    /*7*/    NL,  R, NL, RW,  R,  R, RW, RW, NL,  R,  R, RW,  R,  R, RW, RW,
    /*8*/     R,  W,  R,  W,  W,  W,  W,  W, NL,  R, NL, NL,  W,  W,  W,  W,
    /*9*/    NL,  W, NL, NL,  W,  W,  W,  W, NL,  W, NL, NL, NL,  W, NL, NL,
    /*A*/     R,  R,  R,  R,  R,  R,  R,  R, NL,  R, NL,  R,  R,  R,  R,  R,
    /*B*/    NL,  R, NL,  R,  R,  R,  R,  R, NL,  R, NL, NL,  R,  R,  R,  R,
    /*C*/    NL,  R,  R, RW, NL,  R, RW, RW, NL,  R, NL, NL, NL,  R, RW, RW,
    /*D*/    NL,  R, NL, RW,  R,  R, RW, RW, NL,  R,  R, RW,  R,  R, RW, RW,
    /*E*/    NL,  R,  R, RW, NL,  R, RW, RW, NL,  R,  R,  R, NL,  R, RW, RW,
    /*F*/    NL,  R, NL, RW,  R,  R, RW, RW, NL,  R,  R, RW,  R,  R, RW, RW,
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
    }
}

#if 0
        char Buf[1024];
        sprintf(Buf, "%X\n", WriteValue);
        OutputDebugString(Buf);
#endif   

static uint8 cpuTick(cpu *Cpu, input *NewInput)
{
    uint8 CyclesElapsed = 0;

    uint8 AddressType;
    uint8 AddressMode;
    char *InstrName;

    // Input read
    if(Cpu->PadStrobe)
    {
        for(uint8 idx = 0; idx < input::BUTTON_NUM; ++idx)
            Cpu->InputPad1.buttons[idx] = NewInput->buttons[idx];
    }
  
    if(Nmi.NmiInterrupt)
    {
        OutputDebugString("!!!!NMI");
        Nmi.NmiInterrupt = false;

        AddressMode = IMPL;
        InstrName = "NMI";
        nmi(Cpu);
    }
    else if(IrqTriggered)
    {
        //IRQ_BRK_VEC;
        AddressMode = IMPL;
        InstrName = "IRQ";
    }
    else
    {
        // TODO: NMI WORK!! INTERUPT HAS CYCLES TOO> SO NEED TO PUT IN MAIN LOOP.
        
        // The next cycle, set to 1 if operation is ended and new opcode should be read
        // else is just the next cycle
        Cpu->NextCycle = Cpu->Cycle + 1;
        
        if(Cpu->Cycle == 1)
        {
            pollInterrupts();
            Cpu->OpInstruction = readCpu8(Cpu->PrgCounter++, Cpu);
        }    
        else
        {
            // NOTE: Read the Instruction information here.
            //    Will allow for branching to load the next code for us and still have the info.
            if(Cpu->Cycle == 2)
            {
                AddressType = instAddressType[Cpu->OpInstruction];
                AddressMode = instAddressMode[Cpu->OpInstruction];
                InstrName = instName[Cpu->OpInstruction];

                // NOTE: CPU Log options
                char LogBuffer[1024];
                sprintf(LogBuffer, "%4X %d %s\n", Cpu->PrgCounter, Cpu->OpInstruction, InstrName);
                OutputDebugString(LogBuffer);
            }

            if(AddressType == IMPL)
            {
                implied(Cpu);
            }
            else if(AddressType == ACM)
            {
                accumulator(Cpu);
            }
            else if(AddressType == IMED)
            {
                immediate(Cpu);
            }
            else if(AddressType == REL)
            {
                relative(Cpu);
            }
            else if(AddressType == ZERO)
            {
                if(AddressMode == R)
                    zeroRead(Cpu);
                else if(AddressMode == RW)
                    zeroReadWrite(Cpu);
                else if(AddressMode == W)
                    zeroWrite(Cpu);
            }
            else if(AddressType == ZERX)
            {
                if(AddressMode == R)
                    zeroXIndexRead(Cpu);
                else if(AddressMode == RW)
                    zeroXIndexReadWrite(Cpu);
                else if(AddressMode == W)
                    zeroXIndexWrite(Cpu);
            }
            else if(AddressType == ZERY)
            {
                if(AddressMode == R)
                    zeroYIndexRead(Cpu);
                else if(AddressMode == RW)
                    zeroYIndexReadWrite(Cpu);
                else if(AddressMode == W)
                    zeroYIndexWrite(Cpu);
            }
            else if(AddressType == ABS)
            {
                if(AddressMode == R)
                    absRead(Cpu);
                else if(AddressMode == RW)
                    absReadWrite(Cpu);
                else if(AddressMode == W)
                    absWrite(Cpu);
            }
            else if(AddressType == ABSJ)
            {
                absJmp(Cpu);
            }
            else if(AddressType == ABSX)
            {
                if(AddressMode == R)
                    absXIndexRead(Cpu);
                else if(AddressMode == RW)
                    absXIndexReadWrite(Cpu);
                else if(AddressMode == W)
                    absXIndexWrite(Cpu);
            }
            else if(AddressType == ABSY)
            {
                if(AddressMode == R)
                    absYIndexRead(Cpu);
                else if(AddressMode == RW)
                    absYIndexReadWrite(Cpu);
                else if(AddressMode == W)
                    absYIndexWrite(Cpu);
            }
            else if(AddressType == INDX)
            {
                if(AddressMode == R)
                    idxXRead(Cpu);
                else if(AddressMode == RW)
                    idxXReadWrite(Cpu);
                else if(AddressMode == W)
                    idxXWrite(Cpu);
            }
            else if(AddressType == INDY)
            {
                if(AddressMode == R)
                    idxYRead(Cpu);
                else if(AddressMode == RW)
                    idxYReadWrite(Cpu);
                else if(AddressMode == W)
                    idxYWrite(Cpu);
            }
            else if(AddressType == INDI)
            {
                absIndJmp(Cpu);
            }
        }
        
        Cpu->Cycle = Cpu->NextCycle;
    }
    
    return(1);
}

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



