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
    /*2*/    ABS, INDX, IMPL, INDX, ZERO, ZERO, ZERO, ZERO, IMPL, IMED,  ACM, IMED,  ABS,  ABS,  ABS,  ABS,
    /*3*/    REL, INDY, IMPL, INDY, ZERX, ZERX, ZERX, ZERX, IMPL, ABSY, IMPL, ABSY, ABSX, ABSX, ABSX, ABSX,
    /*4*/   IMPL, INDX, IMPL, INDX, ZERO, ZERO, ZERO, ZERO, IMPL, IMED,  ACM, IMED,  ABS,  ABS,  ABS,  ABS,
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

uint8 (*instrOps[INSTRUCTION_COUNT])(uint8 InByte, cpu *Cpu) =
{
    /*         0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F        */
    /*0*/    brk,ora,kil,slo,nop,ora,asl,slo,php,ora,asl,anc,nop,ora,asl,slo,
    /*1*/    bpl,ora,kil,slo,nop,ora,asl,slo,clc,ora,nop,slo,nop,ora,asl,slo,
    /*2*/    jsr,AND,kil,rla,bit,AND,rol,rla,plp,AND,rol,anc,bit,AND,rol,rla,
    /*3*/    bmi,AND,kil,rla,nop,AND,rol,rla,sec,AND,nop,rla,nop,AND,rol,rla,
    /*4*/    rti,eor,kil,sre,nop,eor,lsr,sre,pha,eor,lsr,alr,jmp,eor,lsr,sre,
    /*5*/    bvc,eor,kil,sre,nop,eor,lsr,sre,cli,eor,nop,sre,nop,eor,lsr,sre,
    /*6*/    rts,adc,kil,rra,nop,adc,ror,rra,pla,adc,ror,arr,jmp,adc,ror,rra,
    /*7*/    bvs,adc,kil,rra,nop,adc,ror,rra,sei,adc,nop,rra,nop,adc,ror,rra,
    /*8*/    nop,sta,nop,sax,sty,sta,stx,sax,dey,nop,txa,xaa,sty,sta,stx,sax,
    /*9*/    bcc,sta,kil,ahx,sty,sta,stx,sax,tya,sta,txs,tas,shy,sta,shx,ahx,
    /*A*/    ldy,lda,ldx,lax,ldy,lda,ldx,lax,tay,lda,tax,lax,ldy,lda,ldx,lax,
    /*B*/    bcs,lda,kil,lax,ldy,lda,ldx,lax,clv,lda,tsx,las,ldy,lda,ldx,lax,
    /*C*/    cpy,cmp,nop,dcp,cpy,cmp,dec,dcp,iny,cmp,dex,axs,cpy,cmp,dec,dcp,
    /*D*/    bne,cmp,kil,dcp,nop,cmp,dec,dcp,cld,cmp,nop,dcp,nop,cmp,dec,dcp,
    /*E*/    cpx,sbc,nop,isc,cpx,sbc,inc,isc,inx,sbc,nop,sbc,cpx,sbc,inc,isc,
    /*F*/    beq,sbc,kil,isc,nop,sbc,inc,isc,sed,sbc,nop,isc,nop,sbc,inc,isc
};



// Calling ops on address type and mode

void acm_or_impl(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        instrOps[Instruction](0, Cpu);
        Cpu->NextCycle = 1;
    }
}

void immediate(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 Value = readCpu8(Cpu->PrgCounter++, Cpu);
        instrOps[Instruction](Value, Cpu);
        Cpu->NextCycle = 1;
    }
}

void absRead(uint8 Instruction, cpu *Cpu)
{
    uint16 Address;

    if(Cpu->Cycle == 2)
    {
        Address = 0;
        Address = (Address & 0xFF00) | readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        Address = (readCpu8(Cpu->PrgCounter++, Cpu) << 8) | (Address & 0x00FF);
    }
    if(Cpu->Cycle == 4)
    {
        uint8 Value = readCpu8(Address, Cpu);
        instrOps[Instruction](Value, Cpu);
        Cpu->NextCycle = 1;
    }
}

void absReadWrite(uint8 Instruction, cpu *Cpu)
{
    uint16 Address;

    if(Cpu->Cycle == 2)
    {
        Address = 0;
        Address = (Address & 0xFF00) | readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        Address = (readCpu8(Cpu->PrgCounter++, Cpu) << 8) | (Address & 0x00FF);
    }
    if(Cpu->Cycle == 4)
    {
        uint8 Value = readCpu8(HighByte << 8 | LowByte, Cpu);
    }
    else if(Cpu->Cycle == 5)
    {
        instrOps[Instruction](Value, Cpu);
    }
    else if(Cpu->Cycle == 6)
    {
        writeCpu8(Value, Address, Cpu);
        Cpu->NextCycle = 1;
    }
}

void absWrite(uint8 Instruction, cpu *Cpu)
{
    uint16 Address;

    if(Cpu->Cycle == 2)
    {
        Address = 0;
        Address = (Address & 0xFF00) | readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        Address = (readCpu8(Cpu->PrgCounter++, Cpu) << 8) | (Address & 0x00FF);
    }
    if(Cpu->Cycle == 4)
    {
        uint8 Byte = instrOps[Instruction](Value, Cpu);
        writeCpu8(Byte, HighByte << 8 | LowByte, Cpu);
        Cpu->NextCycle = 1;
    }
}

void zeroRead(uint8 Instruction, cpu *Cpu)
{
    uint8 Byte;

    if(Cpu->Cycle == 2)
    {
        Byte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    if(Cpu->Cycle == 3)
    {
        uint8 Value = readCpu8(Byte, Cpu);
        instrOps[Instruction](Value, Cpu);
        Cpu->NextCycle = 1;
    }
}

void zeroReadWrite(uint8 Instruction, cpu *Cpu)
{
    uint8 Byte;

    if(Cpu->Cycle == 2)
    {
        Byte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        uint8 Value = readCpu8(Byte, Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 Value = instrOps[Instruction](Value, Cpu);
    }
    else if(Cpu->Cycle == 5)
    {
        writeCpu8(Value, Address, Cpu);
        Cpu->NextCycle = 1;
    }    
}

void zeroWrite(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 Byte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        uint8 Value = instrOps[Instruction](Value, Cpu);
        writeCpu8(Value, Byte, Cpu);
        Cpu->NextCycle = 1;
    }
}


void zeroXIndexRead(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 Byte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        Byte += Cpu->X;        
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 Value = readCpu8(Byte, Cpu);
        instrOps[Instruction](Value, Cpu);// Do Op pass in value
        Cpu->NextCycle = 1;
    }   
}

void zeroXIndexReadWrite(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 Byte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        Byte += Cpu->X;        
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 Value = readCpu8(Byte, Cpu);
    }
    else if(Cpu->Cycle == 5)
    {
        uint8 Value = instrOps[Instruction](Value, Cpu);// Do op on value and return
    }
    else if(Cpu->Cycle == 6)
    {
        writeCpu8(Value, Byte, Cpu);
        Cpu->NextCycle = 1;
    }
}

void zeroXIndexWrite(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 Byte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        Byte += Cpu->X;
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 Value = instrOps[Instruction](Value, Cpu);
        writeCpu8(Value, Byte, Cpu);
        Cpu->NextCycle = 1;
    }
}


void zeroYIndexRead(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 Byte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        Byte += Cpu->Y;        
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 Value = readCpu8(Byte, Cpu);
        instrOps[Instruction](Value, Cpu);
        Cpu->NextCycle = 1;
    }   
}

void zeroYIndexReadWrite(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 Byte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        Byte += Cpu->Y;        
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 Value = readCpu8(Byte, Cpu);
    }
    else if(Cpu->Cycle == 5)
    {
        uint8 Value = instrOps[Instruction](Value, Cpu);
    }
    else if(Cpu->Cycle == 6)
    {
        writeCpu8(Value, Byte, Cpu);
        Cpu->NextCycle = 1;
    }
}

void zeroYIndexWrite(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 Byte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        Byte += Cpu->Y;
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 Value = instrOps[Instruction](Value, Cpu);
        writeCpu8(Value, Byte, Cpu);
    }
}

void absXIndexRead(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 LowByte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        uint8 HighByte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        Byte += Cpu->X;        
    }
    else if(Cpu->Cycle == 5)
    {
        uint8 Value = readCpu8(Byte, Cpu);
        instrOps[Instruction](Value, Cpu);
        Cpu->NextCycle = 1;
    }
    // TODO: PAGE BOUNDARY CROSSING
}

void absXIndexReadWrite(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 LowByte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        uint8 HighByte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        Byte += Cpu->X;        
    }
    else if(Cpu->Cycle == 5)
    {
        uint8 Value = readCpu8(Byte, Cpu);
    }
    else if(Cpu->Cycle == 6)
    {
        uint8 Value = instrOps[Instruction](Value, Cpu);
    }
    else if(Cpu->Cycle == 7)
    {
        writeCpu8(Value, Byte, Cpu);
        Cpu->NextCycle = 1;
    }
}

void absXIndexWrite(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 LowByte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        uint8 HighByte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        Byte += Cpu->X;        
    }
    else if(Cpu->Cycle == 5)
    {
        uint8 Value = instrOps[Instruction](Value, Cpu);
        writeCpu8(Value, Byte, Cpu);
        Cpu->NextCycle = 1;
    }
}


void absYIndexRead(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 LowByte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        uint8 HighByte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        Byte += Cpu->Y;        
    }
    else if(Cpu->Cycle == 5)
    {
        uint8 Value = readCpu8(Byte, Cpu);
        instrOps[Instruction](Value, Cpu);
        Cpu->NextCycle = 1;
    }
    // TODO: PAGE BOUNDARY CROSSING
}

void absYIndexReadWrite(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 LowByte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        uint8 HighByte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        Byte += Cpu->Y;        
    }
    else if(Cpu->Cycle == 5)
    {
        uint8 Value = readCpu8(Byte, Cpu);
    }
    else if(Cpu->Cycle == 6)
    {
        uint8 Value = instrOps[Instruction](Value, Cpu);
    }
    else if(Cpu->Cycle == 7)
    {
        writeCpu8(Value, Byte, Cpu);
        Cpu->NextCycle = 1;
    }
}

void absYIndexWrite(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 LowByte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        uint8 HighByte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        Byte += Cpu->Y;        
    }
    else if(Cpu->Cycle == 5)
    {
        uint8 Value = instrOps[Instruction](Value, Cpu);
        writeCpu8(Value, Byte, Cpu);
        Cpu->NextCycle = 1;
    }
}

// TODO: Save the instruction loaded in cpu 
void relative(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 Operand = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        uint8 NextOp = readCpu8(Cpu->PrgCounter, Cpu);

        uint8 Branch = instrOps[Instruction](Value, Cpu);
        if(Branch)
        {
            Cpu->PrgCounter = (Cpu->PrgCounter & 0xFF00) | (Cpu->PrgCounter & 0x00FF) + Operand;
        }
        else
        {
            ++Cpu->PrgCounter;
            Cpu->NextCycle = 2;
        }
    }
    else if(Cpu->Cycle == 4)
    {
        NextOp = readCpu8(Cpu->PrgCounter, Cpu);
        // TODO: Fix Boundary cross prgcounter
        //if(not changed)
        //{
            ++Cpu->PrgCounter;
            Cpu->NextCycle = 2;
            //}
    }
    else if(Cpu->Cycle == 5)
    {
        NextOp = readCpu8(Cpu->PrgCounter++, Cpu);
        Cpu->NextCycle = 2;
    }
}

void idxXRead(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 Pointer = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        Pointer += Cpu->X;
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 LowByte = readCpu(Pointer, Cpu);
    }
    else if(Cpu->Cycle == 5)
    {
        uint8 HighByte = readCpu(Pointer+1, Cpu);
    }
    else if(Cpu->Cycle == 6)
    {
        uint8 Value = readCpu8(HighByte << 8 | LowByte, Cpu);
        instrOps[Instruction](Value, Cpu);
        Cpu->NextCycle = 1;
    }
}

void idxXReadWrite(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 Pointer = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        Pointer += Cpu->X;
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 LowByte = readCpu(Pointer, Cpu);
    }
    else if(Cpu->Cycle == 5)
    {
        uint8 HighByte = readCpu(Pointer+1, Cpu);
    }
    else if(Cpu->Cycle == 6)
    {
        uint8 Value = readCpu8(HighByte << 8 | LowByte, Cpu);
    }
    else if(Cpu->Cycle == 7)
    {
        uint8 Value = instrOps[Instruction](Value, Cpu);
    }
    else if(Cpu->Cycle == 8)
    {
        writeCpu8(Value, HighByte << 8 | LowByte, Cpu);
        Cpu->NextCycle = 1;
    }
}


void idxXWrite(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 Pointer = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        Pointer += Cpu->X;
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 LowByte = readCpu(Pointer, Cpu);
    }
    else if(Cpu->Cycle == 5)
    {
        uint8 HighByte = readCpu(Pointer+1, Cpu);
    }
    else if(Cpu->Cycle == 6)
    {
        uint8 Value = instrOps[Instruction](Value, Cpu);
        writeCpu8(Value, HighByte << 8 | LowByte, Cpu);
        Cpu->NextCycle = 1;
    }
}

void idxYRead(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 Pointer = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        uint8 LowByte = readCpu(Pointer, Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 HighByte = readCpu(Pointer+1, Cpu);
        LowByte += Cpu->Y; // TODO: Add to 16bit address instead!
    }
    else if(Cpu->Cycle == 5)
    {
        // IF Address is good, then stop here TODO:
        // check boundary

        //if(AddressWas Fine)
            Cpu->NextCycle = 1;
    }
    else if(Cpu->Cycle == 6)
    {
        // Will not enter if boundary not crossed
        uint8 Value = readCpu8(HighByte << 8 | LowByte, Cpu);
        Cpu->NextCycle = 1;
    }
    // TODO: BOUNDARY CROSS CHECK
}

void idxYReadWrite(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 Pointer = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        uint8 LowByte = readCpu(Pointer, Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 HighByte = readCpu(Pointer+1, Cpu);
        LowByte += Cpu->Y;
    }
    else if(Cpu->Cycle == 5)
    {
        // BOUNDARY FIX here TODO:
    }
    else if(Cpu->Cycle == 6)
    {
        uint8 Value = readCpu8(HighByte << 8 | LowByte, Cpu);
    }
    else if(Cpu->Cycle == 7)
    {
        uint8 Value = instrOps[Instruction](Value, Cpu);
    }
    else if(Cpu->Cycle == 8)
    {
        writeCpu8(Value, HighByte << 8 | LowByte, Cpu);
        Cpu->NextCycle = 1;
    }
}


void idxYWrite(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 Pointer = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        uint8 LowByte = readCpu(Pointer, Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 HighByte = readCpu(Pointer+1, Cpu);
        LowByte += Cpu->Y;
    }
    else if(Cpu->Cycle == 5)
    {
        // TODO: Boundary FIX
    }
    else if(Cpu->Cycle == 6)
    {
        uint8 Value = instrOps[Instruction](Value, Cpu);
        writeCpu8(Value, HighByte << 8 | LowByte, Cpu);
        Cpu->NextCycle = 1;
    }
}

void absIndJmp(uint8 Instruction, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        uint8 PointerLow = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        uint8 PointerHigh = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 Latch = readCpu8((PointerHigh << 8) | PointerLow, Cpu);
    }
    else if(Cpu->Cycle == 5)
    {
        Cpu->PrgCounter = (readCpu8((PointerHigh << 8) | PointerLow, Cpu) << 8) | Latch;
        Cpu->NextCycle = 1;
    }   
}



static uint8 nmi_irq(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 HighByte = (uint8)(Cpu->PrgCounter >> 8);
    uint8 LowByte = (uint8)Cpu->PrgCounter; 
    push(HighByte, Cpu);
    push(LowByte, Cpu);

    clearBreak(&Cpu->Flags);
    push(Cpu->Flags, Cpu); 
    setInterrupt(&Cpu->Flags);

    Cpu->PrgCounter = (read8(Address+1, Cpu->MemoryBase) << 8) | read8(Address, Cpu->MemoryBase);
    return(0);
}

#if 0
        char Buf[1024];
        sprintf(Buf, "%X\n", WriteValue);
        OutputDebugString(Buf);
#endif   

static uint8 cpuTick(cpu *Cpu, input *NewInput)
{
    uint8 CyclesElapsed = 0;
    
    uint16 Address = 0;

    uint8 Instruction;
    uint8 AddressType;
    uint8 AddressMode;
    uint8 InstrLength;
    char *InstrName;
    uint8 InstrCycles;
    uint8 InstrData[3]; // Stores data for each instruction

    // Input read
    if(Cpu->PadStrobe)
    {
        for(uint8 idx = 0; idx < input::BUTTON_NUM; ++idx)
            Cpu->InputPad1.buttons[idx] = NewInput->buttons[idx];
    }

  
    if(Nmi.NmiInterrupt)
    {
        Nmi.NmiInterrupt = false;
        //NmiTriggered = true;
        //Cpu->StartNmi = false;
                
        LogCpu.PrgCounter = NMI_VEC;
        Address = NMI_VEC;
        AddressMode = IMPL;
        InstrLength = 0;
        InstrName = "NMI";
        InstrCycles = 7;
        InstrData[0] = 0;

        nmi_irq(Address, Cpu, AddressMode);
    }
    else if(IrqTriggered)
    {
        LogCpu.PrgCounter = readCpu16(IRQ_BRK_VEC, Cpu);
        Address = IRQ_BRK_VEC;
        AddressMode = IMPL;
        InstrLength = 0;
        InstrName = "IRQ";
        InstrCycles = 7;
        InstrData[0] = 0;
    }
    else
    {
        // The next cycle, set to 1 if operation is ended and new opcode should be read
        // else is just the next cycle
        Cpu->NextCycle = Cpu->Cycle + 1;
        
        if(Cpu->Cycle == 1)
        {
            Instruction = readCpu8(Cpu->PrgCounter++, Cpu);
        }    
        else
        {
            // NOTE: Read the Instruction information here.
            //    Will allow for branching to load the next code for us and still have the info.
            if(Cpu->Cycle == 2)
            {
                AddressType = instAddressType[Instruction];
                AddressMode = instAddressMode[Instruction];
                InstrLength = instLength[Instruction];
                InstrName = instName[Instruction];
                InstrCycles = instCycles[Instruction];
            }

            if(AddressMode == ACM || AddressMode == IMPL)
            {
                acm_or_impl();
            }
            else if(AddressMode == IMED)
            {
                immediate();
            }
            else if(AddressMode == REL)
            {
                relative();
            }
            else if(AddressMode == ZERO)
            {
                
            }
            else if(AddressMode == ZERX)
            {
                
            }
            else if(AddressMode == ZERY)
            {
                
            }
            else if(AddressMode == ABS)
            {
                
            }
            else if(AddressMode == ABSX)
            {
                
            }
            else if(AddressMode == ABSY)
            {
                
            }
            else if(AddressMode == INDX)
            {
                
            }
            else if(AddressMode == INDY)
            {
                
            }
            else if(AddressMode == INDI)
            {
                
            }
        }
        
        Cpu->Cycle = Cpu->NextCycle;
        
        //
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



