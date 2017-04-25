/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#define NMI_VEC     0xFFFA
#define RESET_VEC   0xFFFC
#define IRQ_BRK_VEC 0xFFFE 
#define INSTRUCTION_COUNT 256
#define STACK_ADDRESS 0x100

struct cpu
{
    uint8 A;
    uint8 X;
    uint8 Y; 
    uint8 Flags;
    uint8 StackPtr;
    uint16 PrgCounter;
    uint64 MemoryBase;
};

internal uint8 readCpu8(uint16 Address, uint64 MemoryOffset)
{
    // NOTE: Mirrors the address for the 2kb ram 
    if(0x800 <= Address && Address < 0x2000)
        Address = (Address % 0x800);
    // NOTE: Mirror for PPU Registers
    if(0x2008 <= Address && Address < 0x4000)
        Address = (Address % (0x2008 - 0x2000)) + 0x2000;
    
    if(Address == 0x2007) // Reading from the IO of ppu. First read is junk, unless its the colour palette
        IOReadFromCpu = true;
        
    uint8 Value = read8(Address, MemoryOffset);
            
    if(Address == 0x2002)
    {
        // NOTE: Will reset 2005 and 2006 registers, and turn off bit 7 of 0x2002
        ResetScrollIOAdrs = true;
        ResetVRamIOAdrs = true;
        
        uint8 ResetValue = Value & ~(1 << 7);
        write8(ResetValue, Address, MemoryOffset);
    }
    
    return(Value);
}

internal void writeCpu8(uint8 Byte, uint16 Address, uint64 MemoryOffset)
{
    // NOTE: Mirrors the address for the 2kb ram 
    if(0x800 <= Address && Address < 0x2000)
        Address = (Address % 0x800);
    // NOTE: Mirror for PPU Registers
    if(0x2008 <= Address && Address < 0x4000)
        Address = (Address % (0x2008 - 0x2000)) + 0x2000;
    if(0x8000 < Address || Address == 0x2002)
        Assert(0); // TODO: Writing to Program ROM, bank switching?     
    
    write8(Byte, Address, MemoryOffset);

    if(Address == 0x2004) // OAM data
        OamDataChange = true;
    if(Address == 0x2005) // Scroll address
        ScrollAdrsChange = true;
    if(Address == 0x2006) // Writing to ppu io address register
        VRamAdrsChange = true;
    if(Address == 0x2007) // Write to IO for ppu. Happens after two writes to 0x2006
        IOWriteFromCpu = true;


    // NOTE: OAM DMA Write
    if(Address == 0x4014)
    {
        uint8 OamAddress = read8(0x2003, MemoryOffset);

        if(OamData == 0)
        {
            Assert(0);
        }
        
        for(uint16 index = OamAddress; index < OAM_SIZE; ++index)
        {
            uint16 NewAddress = (Byte << 8) | index; 
            OamData[index] = read8(NewAddress, MemoryOffset);
        }
        
    }
}

internal uint16 readCpu16(uint16 Address, uint64 MemoryOffset)
{
    // NOTE: Little Endian
    uint8 LowByte = readCpu8(Address, MemoryOffset);
    uint8 HighByte = readCpu8(Address+1, MemoryOffset);
        
    uint16 NewAddress = (HighByte << 8) | LowByte;
    return(NewAddress);
}


internal uint16 bugReadCpu16(uint16 Address, uint64 MemoryOffset)
{
    // NOTE: This is a bug in the nes 6502 that will wrap the value instead of going to new page.
    //       Only happens with indirect addressing.
    
    uint8 LowByte = readCpu8(Address, MemoryOffset);
    uint16 Byte2Adrs = (Address & 0xFF00) | (uint16)((uint8)(Address + 1));
    uint8 HighByte = readCpu8(Byte2Adrs, MemoryOffset);
        
    uint16 NewAddress = (HighByte << 8) | LowByte;
    return(NewAddress);
}

internal void push(uint8 Byte, cpu *Cpu)
{
    writeCpu8(Byte, (uint16)Cpu->StackPtr | STACK_ADDRESS, Cpu->MemoryBase);
    --Cpu->StackPtr;  
}
internal uint8 pop(cpu *Cpu)
{
    ++Cpu->StackPtr;
    uint8 Value = readCpu8((uint16)Cpu->StackPtr | STACK_ADDRESS, Cpu->MemoryBase);
    return(Value);
}


#define CARRY_BIT     0x01
#define ZERO_BIT      0x02
#define INTERRUPT_BIT 0x04
#define DECIMAL_BIT   0x08
#define BREAK_BIT     0x10
#define BLANK_BIT     0x20
#define OVERFLOW_BIT  0x40
#define NEGATIVE_BIT  0x80
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

enum addressMode
{
    NUL = 0, ACM, IMED,
    ZERO, ZERX, ZERY,
    ABS, ABSX, ABSY, IMPL, REL,
    INDX, INDY, INDI
};

global uint8 instAddressMode[INSTRUCTION_COUNT] =
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

global uint8 instLength[INSTRUCTION_COUNT] =
{
    /*      0 1 2 3 4 5 6 7 8 9 A B C D E F      */
    /*0*/   2,2,1,2,2,2,2,2,1,2,1,2,3,3,3,3,
    /*1*/   2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,
    /*2*/   3,2,1,2,2,2,2,2,1,2,1,2,3,3,3,3,
    /*3*/   2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,
    /*4*/   1,2,1,2,2,2,2,2,1,2,1,2,3,3,3,3,
    /*5*/   2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,
    /*6*/   1,2,1,2,2,2,2,2,1,2,1,2,3,3,3,3,
    /*7*/   2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,
    /*8*/   2,2,2,2,2,2,2,2,1,2,1,2,3,3,3,3,
    /*9*/   2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,
    /*A*/   2,2,2,2,2,2,2,2,1,2,1,2,3,3,3,3,
    /*B*/   2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,
    /*C*/   2,2,2,2,2,2,2,2,1,2,1,2,3,3,3,3,
    /*D*/   2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,
    /*E*/   2,2,2,2,2,2,2,2,1,2,1,2,3,3,3,3,
    /*F*/   2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,
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

global uint8 instCycles[INSTRUCTION_COUNT] =
{
    /*     0 1 2 3 4 5 6 7 8 9 A B C D E F      */
    /*0*/  7,6,0,8,3,3,5,5,3,2,2,2,4,4,6,6,
    /*1*/  2,5,0,8,4,4,6,6,2,4,2,7,4,4,7,7, 
    /*2*/  6,6,0,8,3,3,5,5,4,2,2,2,4,4,6,6,
    /*3*/  2,5,0,8,4,4,6,6,2,4,2,7,4,4,7,7,
    /*4*/  6,6,0,8,3,3,5,5,3,2,2,2,3,4,6,6,
    /*5*/  2,5,0,8,4,4,6,6,2,4,2,7,4,4,7,7,
    /*6*/  6,6,0,8,3,3,5,5,4,2,2,2,5,4,6,6,
    /*7*/  2,5,0,8,4,4,6,6,2,4,2,7,4,4,7,7,
    /*8*/  2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,
    /*9*/  2,6,0,6,4,4,4,4,2,5,2,5,5,5,5,5,
    /*A*/  2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,
    /*B*/  2,5,0,5,4,4,4,4,2,4,2,4,4,4,4,4,
    /*C*/  2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,
    /*D*/  2,5,0,8,4,4,6,6,2,4,2,7,4,4,7,7,
    /*E*/  2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,
    /*F*/  2,5,0,8,4,4,6,6,2,4,2,7,4,4,7,7
};

global uint8 instBoundaryCheck[INSTRUCTION_COUNT] =
{
    /*     0 1 2 3 4 5 6 7 8 9 A B C D E F      */
    /*0*/  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*1*/  1,1,0,0,0,0,0,0,0,1,0,0,1,1,0,0,
    /*2*/  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*3*/  1,1,0,0,0,0,0,0,0,1,0,0,1,1,0,0,
    /*4*/  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*5*/  1,1,0,0,0,0,0,0,0,1,0,0,1,1,0,0,
    /*6*/  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*7*/  1,1,0,0,0,0,0,0,0,1,0,0,1,1,0,0,
    /*8*/  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*9*/  1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*A*/  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*B*/  1,1,0,1,0,0,0,0,0,1,0,1,1,1,1,1,
    /*C*/  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*D*/  1,1,0,0,0,0,0,0,0,1,0,0,1,1,0,0,
    /*E*/  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*F*/  1,1,0,0,0,0,0,0,0,1,0,0,1,1,0,0,
};

#include "operations.cpp"

uint8 (*instrOps[INSTRUCTION_COUNT])(uint16 Address, cpu *Cpu, uint8 AddressMode) =
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


internal uint8 nmi_irq(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 HighByte = (uint8)(Cpu->PrgCounter >> 8);
    uint8 LowByte = (uint8)Cpu->PrgCounter; 
    push(HighByte, Cpu);
    push(LowByte, Cpu);

    clearBreak(&Cpu->Flags);
    push(Cpu->Flags, Cpu); 
    setInterrupt(&Cpu->Flags);

    Cpu->PrgCounter = readCpu16(Address, Cpu->MemoryBase);
    return(0);
}

internal uint8 cpuTick(cpu *Cpu)
{
    uint64 MemoryBase = Cpu->MemoryBase;
    uint8 CyclesElapsed = 0;

    uint16 Address = 0;
    bool32 CrossedPage = 0;
    
    uint8 Instruction;
    uint8 AddressMode;
    uint8 InstrLength;
    char *InstrName;
    uint8 InstrCycles;
    uint8 InstrData[3]; // Stores data for each instruction

    // NOTE: Logging: Save Cpu before changes. Print out later
    cpu LogCpu = *Cpu;
    
    if(NmiTriggered)
    {
        LogCpu.PrgCounter = NMI_VEC;
        Address = NMI_VEC;
        AddressMode = IMPL;
        InstrLength = 0;
        InstrName = "NMI";
        InstrCycles = 7;
        InstrData[0] = 0;
    }
    else if(IrqTriggered)
    {
        LogCpu.PrgCounter = IRQ_BRK_VEC;
        Address = IRQ_BRK_VEC;
        AddressMode = IMPL;
        InstrLength = 0;
        InstrName = "IRQ";
        InstrCycles = 7;
        InstrData[0] = 0;
    }
    else
    {    
        Instruction = readCpu8(Cpu->PrgCounter, MemoryBase);
        AddressMode = instAddressMode[Instruction];
        InstrLength = instLength[Instruction];
        InstrName = instName[Instruction];
        InstrCycles = instCycles[Instruction];

        for(int i = 0; i < InstrLength; ++i)
        {
            InstrData[i] = readCpu8(Cpu->PrgCounter + i, MemoryBase); 
        }
    }
    
    switch(AddressMode)
    {
        case ACM:
            break;            
        case IMPL:
            break;
        case IMED:
            Address = Cpu->PrgCounter + 1;
            break;
        case ZERO:
            Address = (uint16)InstrData[1];
            break;
        case ZERX:
            Address = ((uint16)InstrData[1] + Cpu->X) & 0xFF;
            break;
        case ZERY:
            Address = ((uint16)InstrData[1] + Cpu->Y) & 0xFF;
            break;
        case ABS:
            Address = ((uint16)InstrData[2] << 8) | InstrData[1];
            break;
        case ABSX:
            Address = (((uint16)InstrData[2] << 8) | InstrData[1]) + Cpu->X;
            CrossedPage = crossedPageCheck(Address - Cpu->X, Address);
            break;
        case ABSY:
            Address = (((uint16)InstrData[2] << 8) | InstrData[1]) + Cpu->Y;
            CrossedPage = crossedPageCheck(Address - Cpu->Y, Address);
            break;
        case REL:
        {
            int8 RelOffset = InstrData[1];
            Address = Cpu->PrgCounter + 2 + RelOffset;
            break;
        }
        case INDX:
        {
            uint8 ZeroAddress = InstrData[1];
            uint8 AddedAddress = (ZeroAddress + Cpu->X) & 0xFF;
            Address = bugReadCpu16(AddedAddress, Cpu->MemoryBase);
            break;
        }
        case INDY:
        {
            uint8 ZeroAddress = InstrData[1];
            Address = bugReadCpu16(ZeroAddress, Cpu->MemoryBase) + Cpu->Y;
            CrossedPage = crossedPageCheck(Address - Cpu->Y, Address);
            break;
        }
        case INDI:
        {
            uint16 IndirectAddress = ((uint16)InstrData[2] << 8) | InstrData[1];
            Address = bugReadCpu16(IndirectAddress, Cpu->MemoryBase);
            break;
        }
        case NUL:
        {
            Assert(0);
            break;
        }        
    }
    
    Cpu->PrgCounter += InstrLength;
    CyclesElapsed += InstrCycles;

    if(NmiTriggered || IrqTriggered)
    {
        if(NmiTriggered)
        {
            NmiTriggered = false;
        }
        else
        {
            IrqTriggered = false;
        }
        nmi_irq(Address, Cpu, AddressMode);
    }
    else
    {
        // NOTE: This is where the operation is executed, returning extra cycles, for branch ops
        if(CrossedPage)
            CyclesElapsed += instBoundaryCheck[Instruction];
     
        uint8 AdditionalCycles = instrOps[Instruction](Address, Cpu, AddressMode);
        CyclesElapsed += AdditionalCycles;
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
    
    return(CyclesElapsed);
}



