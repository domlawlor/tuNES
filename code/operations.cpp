/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#include "cpu.h"

// Implied Operations

uint8 clc(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 1); // Cycle 1
    clearCarry(&Cpu->Flags);
    return(0);
}
uint8 cld(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 1); // Cycle 1
    clearDecimal(&Cpu->Flags);
    return(0);
}
uint8 cli(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 1); // Cycle 1
    clearInterrupt(&Cpu->Flags);
    return(0);
}
uint8 clv(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 1); // Cycle 1
    clearOverflow(&Cpu->Flags);
    return(0);
}

uint8 dex(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 1); // Cycle 1
    --Cpu->X;
    setZero(Cpu->X, &Cpu->Flags);
    setNegative(Cpu->X, &Cpu->Flags);
    return(0);
}
uint8 dey(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 1); // Cycle 1
    --Cpu->Y;
    setZero(Cpu->Y, &Cpu->Flags);
    setNegative(Cpu->Y, &Cpu->Flags);
    return(0);
}

uint8 inx(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 1); // Cycle 1
    ++Cpu->X;
    setZero(Cpu->X, &Cpu->Flags);
    setNegative(Cpu->X, &Cpu->Flags);    
    return(0);
}
uint8 iny(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 1); // Cycle 1
    ++Cpu->Y;
    setZero(Cpu->Y, &Cpu->Flags);
    setNegative(Cpu->Y, &Cpu->Flags);
    return(0);
}

uint8 sec(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 1); // Cycle 1
    setCarry(&Cpu->Flags);    
    return(0);
}
uint8 sed(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 1); // Cycle 1
    setDecimal(&Cpu->Flags); 
    return(0);
}
uint8 sei(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 1); // Cycle 1
    setInterrupt(&Cpu->Flags); 
    return(0);
}

uint8 tax(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 1); // Cycle 1
    Cpu->X = Cpu->A;
    setZero(Cpu->X, &Cpu->Flags);
    setNegative(Cpu->X, &Cpu->Flags);    
    return(0);
}
uint8 txa(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 1); // Cycle 1
    Cpu->A = Cpu->X;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);    
    return(0);
}

uint8 tay(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 1); // Cycle 1
    Cpu->Y = Cpu->A;
    setZero(Cpu->Y, &Cpu->Flags);
    setNegative(Cpu->Y, &Cpu->Flags);    
    return(0);
}
uint8 tya(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 1); // Cycle 1
    Cpu->A = Cpu->Y;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(0);
}

uint8 tsx(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 1); // Cycle 1
    Cpu->X = Cpu->StackPtr;
    setZero(Cpu->X, &Cpu->Flags);
    setNegative(Cpu->X, &Cpu->Flags);    
    return(0);
}
uint8 txs(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 1); // Cycle 1
    Cpu->StackPtr = Cpu->X;    
    return(0);
}

uint8 nop(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 1); // Cycle 1
    // Does nothing    
    return(0);
}

// TODO: Condense irq and nmi in future
uint8 irq(uint8 Value, cpu *Cpu)
{
    writeStack((Cpu->PrgCounter >> 8), Cpu);
    writeStack((Cpu->PrgCounter & 0xFF), Cpu);
    // At this point, which interrupt is detrmined. Can be hijacked
    // Current implementation is just seperating the nmi irq brk functions, may change // Cycle 5
    clearBreak(&Cpu->Flags);        
    writeStack(Cpu->Flags, Cpu);
    Cpu->PrgCounter = (Cpu->PrgCounter & 0xFF00) | read8(IRQ_BRK_VEC, Cpu->MemoryBase, 6); // Cycle 6      
    Cpu->PrgCounter = (read8(IRQ_BRK_VEC+1, Cpu->MemoryBase, 7) << 8) | (Cpu->PrgCounter & 0xFF); // Cycle 7
    clearBreak(&Cpu->Flags);
    setInterrupt(&Cpu->Flags);   
    return(0);
}


uint8 nmi(uint8 Value, cpu *Cpu)
{
    writeStack((Cpu->PrgCounter >> 8), Cpu);
    writeStack((Cpu->PrgCounter & 0xFF), Cpu);
    // At this point, which interrupt is detrmined. Can be hijacked
    // Current implementation is just seperating the nmi irq brk functions, may change // Cycle 5
    clearBreak(&Cpu->Flags);
    writeStack(Cpu->Flags, Cpu);
    Cpu->PrgCounter = (Cpu->PrgCounter & 0xFF00) | read8(NMI_VEC, Cpu->MemoryBase, 6);     // Cycle 6    
    Cpu->PrgCounter = (read8(NMI_VEC+1, Cpu->MemoryBase, 7) << 8) | (Cpu->PrgCounter & 0xFF); // Cycle 7
    clearBreak(&Cpu->Flags);
    setInterrupt(&Cpu->Flags);
    return(0);
}

uint8 brk(uint8 Value, cpu *Cpu)
{
    readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
    uint8 HighByte = (uint8)(Cpu->PrgCounter >> 8);
    writeStack(HighByte, Cpu);
    uint8 LowByte = (uint8)Cpu->PrgCounter; 
    writeStack(LowByte, Cpu);
    pollInterrupts(Cpu, 4); // Cycle 4
    // TODO: NMI Hijack  // Cycle 4
    setBlank(&Cpu->Flags);
    setBreak(&Cpu->Flags);
    writeStack(Cpu->Flags, Cpu);
    setInterrupt(&Cpu->Flags);
    Cpu->PrgCounter = (Cpu->PrgCounter & 0xFF00) | read8(IRQ_BRK_VEC, Cpu->MemoryBase, 6); // Cycle 6
    Cpu->PrgCounter = (read8(IRQ_BRK_VEC + 1, Cpu->MemoryBase, 7) << 8) | (Cpu->PrgCounter & 0x00FF); // Cycle 7
    return(0);
}

uint8 rti(uint8 Value, cpu *Cpu)
{
    readCpu8(Cpu->PrgCounter, Cpu, 2); // Cycle 2
    Cpu->Flags = readStack(Cpu); // Cycle 4
    Cpu->PrgCounter = (Cpu->PrgCounter & 0xFF00) | readStack(Cpu);
    pollInterrupts(Cpu, 5); // Cycle 5
    Cpu->PrgCounter = (readStack(Cpu) << 8) | (Cpu->PrgCounter & 0x00FF);    
    return(0);
}

uint8 rts(uint8 Value, cpu *Cpu)
{
    readCpu8(Cpu->PrgCounter, Cpu, 2); // Cycle 2
    Cpu->PrgCounter = Cpu->PrgCounter & 0xFF00 | readStack(Cpu);
    Cpu->PrgCounter =  (readStack(Cpu) << 8) | (Cpu->PrgCounter & 0x00FF);
    pollInterrupts(Cpu, 5); // Cycle 5
    ++Cpu->PrgCounter;        
    return(0);
}

uint8 pha(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 2); // Cycle 2
    writeStack(Cpu->A, Cpu);
    return(0);
}

uint8 php(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 2); // Cycle 2
    setBreak(&Cpu->Flags);
    setBlank(&Cpu->Flags);
    writeStack(Cpu->Flags, Cpu);
    return(0);
}

uint8 pla(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 3); // Cycle 3
    Cpu->A = readStack(Cpu);
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(0);
}

uint8 plp(uint8 Value, cpu *Cpu)
{
    pollInterrupts(Cpu, 3); // Cycle 3
    Cpu->Flags = readStack(Cpu);    
    return(0);
}

uint8 jsr(uint8 Value, cpu *Cpu)
{
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
    // TODO: The order of this seems off? Getting low byte first
    // then writing to stack followed by the high byte read??

    // TODO: No Poll interrupts?
    
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
    sprintf(Cpu->LogData2, "%02X", readCpu8(Cpu->PrgCounter, Cpu));
#endif
    
    writeStack((Cpu->PrgCounter >> 8), Cpu); // TODO: Combine decrement and writeStack
    writeStack(Cpu->PrgCounter & 0xFF, Cpu);
    pollInterrupts(Cpu, 5);
    Cpu->PrgCounter = (readCpu8(Cpu->PrgCounter, Cpu, 6) << 8) | Cpu->OpLowByte; // Cycle 6

#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%04X", Cpu->PrgCounter);
#endif
    return(0);
}

// Read
uint8 lda(uint8 Value, cpu *Cpu)
{    
    Cpu->A = Value;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(0);
}
uint8 ldx(uint8 Value, cpu *Cpu)
{
    Cpu->X = Value;
    setZero(Cpu->X, &Cpu->Flags);
    setNegative(Cpu->X, &Cpu->Flags);
    return(0);
}
uint8 ldy(uint8 Value, cpu *Cpu)
{
    Cpu->Y = Value;
    setZero(Cpu->Y, &Cpu->Flags);
    setNegative(Cpu->Y, &Cpu->Flags);
    return(0);
}
uint8 eor(uint8 Value, cpu *Cpu)
{
    Cpu->A = Cpu->A ^ Value;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(Value);
}
uint8 AND(uint8 Value, cpu *Cpu)
{
    Cpu->A = Cpu->A & Value;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(Value);
}

uint8 ora(uint8 Value, cpu *Cpu)
{
    Cpu->A = Cpu->A | Value;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(Value);
}

uint8 adc(uint8 Value, cpu *Cpu)
{    
    uint8 A = Cpu->A;
    uint8 B = Value;
    uint8 C = isBitSet(CARRY_BIT, Cpu->Flags);

    uint16 Sum = (uint16)A + (uint16)B + (uint16)C;

    // Overflow check, taken from the web. One day find out how this works
    if(((A ^ Sum) & (B ^ Sum) & 0x80) == 0x80)
        setOverflow(&Cpu->Flags);
    else
        clearOverflow(&Cpu->Flags);

    if(Sum & 0x100)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);

	// TODO: Check this cast, sum has a negative flag on the 16th bit??
    setZero((uint8)Sum, &Cpu->Flags);
    setNegative((uint8)Sum, &Cpu->Flags);

    Cpu->A = (uint8)Sum;
    return(0);
}

uint8 sbc(uint8 Value, cpu *Cpu)
{   
    uint8 A = Cpu->A;
    uint8 B = ~Value; // NOTE: Using the inverse
    uint8 C = isBitSet(CARRY_BIT, Cpu->Flags);

    uint16 Sum = (uint16)A + (uint16)B + (uint16)C;

    // Overflow check, taken from the web. One day find out how this works
    if(((A ^ Sum) & (B ^ Sum) & 0x80) == 0x80)
        setOverflow(&Cpu->Flags);
    else
        clearOverflow(&Cpu->Flags);

    if(Sum & 0x100)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);

    setZero((uint8)Sum, &Cpu->Flags);
    setNegative((uint8)Sum, &Cpu->Flags);

    Cpu->A = (uint8)Sum;

    return(0);
}

uint8 cmp(uint8 Value, cpu *Cpu)
{
    if(Cpu->A >= Value)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
    
    uint8 CmpValue = Cpu->A - Value;
    setZero(CmpValue, &Cpu->Flags);
    setNegative(CmpValue, &Cpu->Flags);
    return(0);
}

uint8 bit(uint8 Value, cpu *Cpu)
{    
    if(Value & (1 << 6))
        setOverflow(&Cpu->Flags);
    else
        clearOverflow(&Cpu->Flags);
    setNegative(Value, &Cpu->Flags);
    setZero(Cpu->A & Value, &Cpu->Flags);
    return(0);
}

uint8 lax(uint8 Value, cpu *Cpu)
{
    lda(Value, Cpu);
    ldx(Value, Cpu);

    return(0);
}



// Read modify write
uint8 asl(uint8 Value, cpu *Cpu)
{
    if(Value & (1 << 7))
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
    Value = Value << 1;

    setZero(Value, &Cpu->Flags);
    setNegative(Value, &Cpu->Flags);

    return(Value);
}

uint8 lsr(uint8 Value, cpu *Cpu)
{
    if(Value & 1)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
    Value = Value >> 1;

    setZero(Value, &Cpu->Flags);
    setNegative(Value, &Cpu->Flags);
    
    return(Value);
}

uint8 rol(uint8 Value, cpu *Cpu)
{    
    uint8 CarrySet = isBitSet(CARRY_BIT, Cpu->Flags);

    if(Value & (1 << 7))
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
    Value = Value << 1;

    if(CarrySet)
        Value = Value | 1;

    setZero(Value, &Cpu->Flags);
    setNegative(Value, &Cpu->Flags);

    return(Value);
}

uint8 ror(uint8 Value, cpu *Cpu)
{
    uint8 CarrySet = isBitSet(CARRY_BIT, Cpu->Flags);

    if(Value & 1)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
    Value = Value >> 1;
        
    if(CarrySet)
        Value = Value | (1 << 7);

    setZero(Value, &Cpu->Flags);
    setNegative(Value, &Cpu->Flags);

    return(Value);
}

uint8 inc(uint8 Value, cpu *Cpu)
{
    ++Value;
    setZero(Value, &Cpu->Flags);
    setNegative(Value, &Cpu->Flags);
    return(Value);
}

uint8 dec(uint8 Value, cpu *Cpu)
{
    --Value;
    setZero(Value, &Cpu->Flags);
    setNegative(Value, &Cpu->Flags);
    return(Value);
}

uint8 slo(uint8 Value, cpu *Cpu)
{
    Value = asl(Value, Cpu);
    Value = ora(Value, Cpu);
    return(Value);
}
uint8 sre(uint8 Value, cpu *Cpu)
{
    Value = lsr(Value, Cpu);
    Value = eor(Value, Cpu);
    return(Value);
}

uint8 rla(uint8 Value, cpu *Cpu)
{
    Value = rol(Value, Cpu);
    Value = AND(Value, Cpu);
    return(Value);
}
uint8 rra(uint8 Value, cpu *Cpu)
{
    Value = ror(Value, Cpu);
    adc(Value, Cpu);
    return(Value);
}

uint8 isc(uint8 Value, cpu *Cpu)
{
    Value = inc(Value, Cpu);
    sbc(Value, Cpu);
    return(Value);
}

uint8 dcp(uint8 Value, cpu *Cpu)
{
    Value = dec(Value, Cpu);
    cmp(Value, Cpu);
    return(Value);
}

// Write instructions

uint8 sta(uint8 Value, cpu *Cpu)
{
    return(Cpu->A);
}
uint8 stx(uint8 Value, cpu *Cpu)
{
    return(Cpu->X);
}
uint8 sty(uint8 Value, cpu *Cpu)
{
    return(Cpu->Y);
}
uint8 sax(uint8 Value, cpu *Cpu)
{
    return(Cpu->A & Cpu->X);
}


/////////////////////////
// Relative Operations //
/////////////////////////

uint8 bcs(uint8 Value, cpu *Cpu)
{
    return(isBitSet(CARRY_BIT, Cpu->Flags));    
}
uint8 bcc(uint8 Value, cpu *Cpu)
{
    return(!isBitSet(CARRY_BIT, Cpu->Flags));
}

uint8 beq(uint8 Value, cpu *Cpu)
{
    return(isBitSet(ZERO_BIT, Cpu->Flags));
}
uint8 bne(uint8 Value, cpu *Cpu)
{
    return(!isBitSet(ZERO_BIT, Cpu->Flags));    
}

uint8 bmi(uint8 Value, cpu *Cpu)
{
    return(isBitSet(NEGATIVE_BIT, Cpu->Flags));    
}
uint8 bpl(uint8 Value, cpu *Cpu)
{
    return(!isBitSet(NEGATIVE_BIT, Cpu->Flags));    
}

uint8 bvs(uint8 Value, cpu *Cpu)
{
    return(isBitSet(OVERFLOW_BIT, Cpu->Flags));    
}
uint8 bvc(uint8 Value, cpu *Cpu)
{
    return(!isBitSet(OVERFLOW_BIT, Cpu->Flags));    
}
/////////////////////


uint8 jmp(uint8 Value, cpu *Cpu)
{
    Cpu->PrgCounter = (Cpu->OpHighByte << 8) | Cpu->OpLowByte;
    return(0);
}

// Compares

uint8 cpx(uint8 Value, cpu *Cpu)
{
    if(Cpu->X >= Value)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
    
    uint8 CmpValue = Cpu->X - Value;
    setZero(CmpValue, &Cpu->Flags);
    setNegative(CmpValue, &Cpu->Flags);
    return(0);
}
uint8 cpy(uint8 Value, cpu *Cpu)
{
    if(Cpu->Y >= Value)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
    
    uint8 CmpValue = Cpu->Y - Value;
    setZero(CmpValue, &Cpu->Flags);
    setNegative(CmpValue, &Cpu->Flags);
    return(0);
}


uint8 skb(uint8 Value, cpu *Cpu)
{
    // Size 2
    return(0);
}

uint8 skw(uint8 Value, cpu *Cpu)
{
    // Size 3
    return(0);
}

uint8 ahx(uint8 Value, cpu *Cpu)
{
    //Assert(0);
    return(0);
}
uint8 alr(uint8 Value, cpu *Cpu)
{
    Value = AND(Value, Cpu);
    Value = lsr(Value, Cpu);
    return(Value);
}
uint8 anc(uint8 Value, cpu *Cpu)
{
    AND(Value, Cpu);
    
    if(isBitSet(NEGATIVE_BIT, Cpu->Flags))
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
    
    return(0);
}
uint8 arr(uint8 Value, cpu *Cpu)
{
    AND(Value, Cpu);    
    ror(Value, Cpu);

    uint8 bit5 = Cpu->A & (1<<5);
    uint8 bit6 = Cpu->A & (1<<6);

    if(bit6)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);

    if((bit5 && !bit6) || (!bit5 && bit6))
        setOverflow(&Cpu->Flags);
    else
        clearOverflow(&Cpu->Flags);
        
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    
    return(0);
}
uint8 axs(uint8 Value, cpu *Cpu)
{
    uint8 ANDValue = (Cpu->A & Cpu->X);
    Cpu->X = ANDValue - Value;

    if(ANDValue >= Value)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
    
    setZero(Cpu->X, &Cpu->Flags);
    setNegative(Cpu->X, &Cpu->Flags);
    
    return(0);
}
uint8 kil(uint8 Value, cpu *Cpu)
{
    Assert(0);
    return(0);
}
uint8 las(uint8 Value, cpu *Cpu)
{
    //Assert(0);
    return(0); 
}
uint8 shx(uint8 Value, cpu *Cpu)
{
    //Assert(0);
    return(0);
}

uint8 shy(uint8 Value, cpu *Cpu)
{
    //Assert(0);
//    if((Cpu->X + Value) <= 0xFF)
    //writeCpu8(Byte, Address, Cpu); // TODO: This shouldn't happen, need refactoring
    return(0);
}
uint8 tas(uint8 Value, cpu *Cpu)
{
    //Assert(0);
    return(0);
}

uint8 xaa(uint8 Value, cpu *Cpu)
{
    //Assert(0);
    return(0);
}



static uint8 opAddressType[INSTRUCTION_COUNT] =
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

static char * opName[INSTRUCTION_COUNT] =
{
    /*         0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F        */
    /*0*/  "BRK","ORA","NMI","SLO","SKB","ORA","ASL","SLO","PHP","ORA","ASL","ANC","SKW","ORA","ASL","SLO",
    /*1*/  "BPL","ORA","IRQ","SLO","SKB","ORA","ASL","SLO","CLC","ORA","NOP","SLO","SKW","ORA","ASL","SLO",
    /*2*/  "JSR","AND","KIL","RLA","BIT","AND","ROL","RLA","PLP","AND","ROL","ANC","BIT","AND","ROL","RLA", 
    /*3*/  "BMI","AND","KIL","RLA","SKB","AND","ROL","RLA","SEC","AND","NOP","RLA","SKW","AND","ROL","RLA",
    /*4*/  "RTI","EOR","KIL","SRE","SKB","EOR","LSR","SRE","PHA","EOR","LSR","ALR","JMP","EOR","LSR","SRE",
    /*5*/  "BVC","EOR","KIL","SRE","SKB","EOR","LSR","SRE","CLI","EOR","NOP","SRE","SKW","EOR","LSR","SRE",
    /*6*/  "RTS","ADC","KIL","RRA","SKB","ADC","ROR","RRA","PLA","ADC","ROR","ARR","JMP","ADC","ROR","RRA",
    /*7*/  "BVS","ADC","KIL","RRA","SKB","ADC","ROR","RRA","SEI","ADC","NOP","RRA","SKW","ADC","ROR","RRA",
    /*8*/  "SKB","STA","SKB","SAX","STY","STA","STX","SAX","DEY","SKB","TXA","XAA","STY","STA","STX","SAX",
    /*9*/  "BCC","STA","KIL","AHX","STY","STA","STX","SAX","TYA","STA","TXS","TAS","SHY","STA","SHX","AHX",
    /*A*/  "LDY","LDA","LDX","LAX","LDY","LDA","LDX","LAX","TAY","LDA","TAX","LAX","LDY","LDA","LDX","LAX",
    /*B*/  "BCS","LDA","KIL","LAX","LDY","LDA","LDX","LAX","CLV","LDA","TSX","LAS","LDY","LDA","LDX","LAX",
    /*C*/  "CPY","CMP","SKB","DCP","CPY","CMP","DEC","DCP","INY","CMP","DEX","AXS","CPY","CMP","DEC","DCP",
    /*D*/  "BNE","CMP","KIL","DCP","SKB","CMP","DEC","DCP","CLD","CMP","NOP","DCP","SKW","CMP","DEC","DCP",
    /*E*/  "CPX","SBC","SKB","ISC","CPX","SBC","INC","ISC","INX","SBC","NOP","SBC","CPX","SBC","INC","ISC",
    /*F*/  "BEQ","SBC","KIL","ISC","SKB","SBC","INC","ISC","SED","SBC","NOP","ISC","SKW","SBC","INC","ISC"
};

static uint8 opClocks[INSTRUCTION_COUNT] =
{
    /*     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F     */
    /*0*/  7, 6, 7, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
    /*1*/  2, 5, 7, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    /*2*/  6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
    /*3*/  2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    /*4*/  6, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
    /*5*/  2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    /*6*/  6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
    /*7*/  2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    /*8*/  2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
    /*9*/  2, 6, 0, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
    /*A*/  2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
    /*B*/  2, 5, 0, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,
    /*C*/  2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
    /*D*/  2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    /*E*/  2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
    /*F*/  2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
};

uint8 (*operations[INSTRUCTION_COUNT])(uint8 InByte, cpu *Cpu) =
{
    /*         0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F        */
    /*0*/    brk,ora,nmi,slo,skb,ora,asl,slo,php,ora,asl,anc,skw,ora,asl,slo,
    /*1*/    bpl,ora,irq,slo,skb,ora,asl,slo,clc,ora,nop,slo,skw,ora,asl,slo,
    /*2*/    jsr,AND,kil,rla,bit,AND,rol,rla,plp,AND,rol,anc,bit,AND,rol,rla,
    /*3*/    bmi,AND,kil,rla,skb,AND,rol,rla,sec,AND,nop,rla,skw,AND,rol,rla,
    /*4*/    rti,eor,kil,sre,skb,eor,lsr,sre,pha,eor,lsr,alr,jmp,eor,lsr,sre,
    /*5*/    bvc,eor,kil,sre,skb,eor,lsr,sre,cli,eor,nop,sre,skw,eor,lsr,sre,
    /*6*/    rts,adc,kil,rra,skb,adc,ror,rra,pla,adc,ror,arr,jmp,adc,ror,rra,
    /*7*/    bvs,adc,kil,rra,skb,adc,ror,rra,sei,adc,nop,rra,skw,adc,ror,rra,
    /*8*/    skb,sta,skb,sax,sty,sta,stx,sax,dey,skb,txa,xaa,sty,sta,stx,sax,
    /*9*/    bcc,sta,kil,ahx,sty,sta,stx,sax,tya,sta,txs,tas,shy,sta,shx,ahx,
    /*A*/    ldy,lda,ldx,lax,ldy,lda,ldx,lax,tay,lda,tax,lax,ldy,lda,ldx,lax,
    /*B*/    bcs,lda,kil,lax,ldy,lda,ldx,lax,clv,lda,tsx,las,ldy,lda,ldx,lax,
    /*C*/    cpy,cmp,skb,dcp,cpy,cmp,dec,dcp,iny,cmp,dex,axs,cpy,cmp,dec,dcp,
    /*D*/    bne,cmp,kil,dcp,skb,cmp,dec,dcp,cld,cmp,nop,dcp,skw,cmp,dec,dcp,
    /*E*/    cpx,sbc,skb,isc,cpx,sbc,inc,isc,inx,sbc,nop,sbc,cpx,sbc,inc,isc,
    /*F*/    beq,sbc,kil,isc,skb,sbc,inc,isc,sed,sbc,nop,isc,skw,sbc,inc,isc
};


void implied(cpu *Cpu)
{
    operations[Cpu->OpCode](0, Cpu);
}

//////////////////////////////////////
//// ACCUMULATOR AND IMPLIED MODE ////
//////////////////////////////////////

// NOTE: All accumulator ops are 2 cycles
void accumulator(cpu *Cpu)
{
    pollInterrupts(Cpu, 1); // CYcle 1
    Cpu->A = operations[Cpu->OpCode](Cpu->A, Cpu);
}

////////////////////////
//// IMMEDIATE MODE ////
////////////////////////

// NOTE: 2 Cycles
void immediate(cpu *Cpu)
{
    pollInterrupts(Cpu, 1); // Cycle 1
    uint8 Value = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
    operations[Cpu->OpCode](Value, Cpu);
        
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Value);
    sprintf(Cpu->LogExtraInfo, " #$%02X", Value);
#endif
}


///////////////////////
//// RELATIVE MODE ////
///////////////////////

// NOTE: Cycles change depending if branched, and if pages crossed.
void relative(cpu *Cpu)
{
    pollInterrupts(Cpu, 2); // Cycle 2
    Cpu->OpValue = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2

#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpValue);
#endif

    // Cycle 3    
    // The Correct Program Counter. Will be saved into OpHigh and Low Bytes
    uint16 TempPC = Cpu->PrgCounter + (int8)Cpu->OpValue;
    Cpu->OpHighByte = (TempPC & 0xFF00) >> 8;
    Cpu->OpLowByte = TempPC & 0x00FF;

    // The potentiall wrong high byte after branch
    uint8 UnFixedHighByte = (Cpu->PrgCounter & 0xFF00) >> 8;

#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%04X", TempPC);
#endif
        
    uint8 TakeBranch = operations[Cpu->OpCode](0, Cpu);
        
    if(TakeBranch)
    {            
        Cpu->OpClockTotal += 1;
        
        // Update prgcounter, could have unfixed page
        Cpu->PrgCounter = (Cpu->PrgCounter & 0xFF00) | Cpu->OpLowByte;

        // If OpHighByte isn't the same as the unfixed version, then we crossed page
        if(Cpu->OpHighByte != UnFixedHighByte)
        {
            // Page crossed so fix
            Cpu->OpClockTotal += 1;
            pollInterrupts(Cpu, 4); // Cycle 4
            Cpu->PrgCounter = (Cpu->OpHighByte << 8) | (Cpu->PrgCounter & 0xFF);
        }
    }
}

////////////////////////
//// ZERO PAGE MODE ////
////////////////////////

// NOTE: 3 Cycles
void zeroRead(cpu *Cpu)
{
    pollInterrupts(Cpu, 2); // Cycle 2
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
        
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
#endif

    uint8 Value = readCpu8(Cpu->OpLowByte, Cpu, 3); // Cycle 3
    operations[Cpu->OpCode](Value, Cpu);
       
#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%04X = #$%02X", Cpu->OpLowByte, Value);
#endif
}

// NOTE: 5 Cycles
void zeroReadWrite(cpu *Cpu)
{
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2 
        
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
#endif

    Cpu->OpValue = readCpu8(Cpu->OpLowByte, Cpu, 3); // Cycle 3
    Cpu->OpValue = operations[Cpu->OpCode](Cpu->OpValue, Cpu);
    pollInterrupts(Cpu, 4); // Cycle 4
    writeCpu8(Cpu->OpValue, Cpu->OpLowByte, Cpu, 5); // Cycle 5
  
#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%04X = #$%02X", Cpu->OpLowByte, Cpu->OpValue);
#endif
}

// NOTE: 3 Cycles
void zeroWrite(cpu *Cpu)
{
    pollInterrupts(Cpu, 2); // Cycle 2
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2 
        
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
#endif
    
#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%04X = #$%02X", Cpu->OpLowByte, readCpu8(Cpu->OpLowByte, Cpu)); // Cycle 3
#endif

    uint8 Value = operations[Cpu->OpCode](0, Cpu);
    writeCpu8(Value, Cpu->OpLowByte, Cpu, 3); // Cycle 3
}

//////////////////////////
//// ZERO PAGE X MODE ////
//////////////////////////

// 4 cycles
void zeroXIndexRead(cpu *Cpu)
{
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
          
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
#endif

    Cpu->OpLowByte += Cpu->X;
    
    pollInterrupts(Cpu, 3); // Cycle 3
    uint8 Value = readCpu8(Cpu->OpLowByte, Cpu, 4); // Cycle 4
    operations[Cpu->OpCode](Value, Cpu);

#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%02X,X @ $%04X = #$%02X", Cpu->OpLowByte - Cpu->X, Cpu->OpLowByte, Value);
#endif
}

// 6 cycles
void zeroXIndexReadWrite(cpu *Cpu)
{
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
          
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
#endif

    Cpu->OpLowByte += Cpu->X;
    Cpu->OpValue = readCpu8(Cpu->OpLowByte, Cpu, 4); // Cycle 4 

#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%02X,X @ $%04X = #$%02X", Cpu->OpLowByte - Cpu->X, Cpu->OpLowByte, Cpu->OpValue);
#endif

    pollInterrupts(Cpu, 5); // Cycle 5
    Cpu->OpValue = operations[Cpu->OpCode](Cpu->OpValue, Cpu);
    writeCpu8(Cpu->OpValue, Cpu->OpLowByte, Cpu, 6); // Cycle 6
}

// 4 cycles
void zeroXIndexWrite(cpu *Cpu)
{
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
          
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
#endif

    Cpu->OpLowByte += Cpu->X;
    pollInterrupts(Cpu, 3); // Cycle 3

#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%02X,X @ $%04X = #$%02X", Cpu->OpLowByte - Cpu->X, Cpu->OpLowByte, readCpu8(Cpu->OpLowByte, Cpu));
#endif

    uint8 Value = operations[Cpu->OpCode](0, Cpu);
    writeCpu8(Value, Cpu->OpLowByte, Cpu, 4); // Cycle 4
}

//////////////////////////
//// ZERO PAGE Y MODE ////
//////////////////////////

// 4 cycles
void zeroYIndexRead(cpu *Cpu)
{
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
          
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
#endif
    
    Cpu->OpLowByte += Cpu->Y;
    pollInterrupts(Cpu, 3); // Cycle 3
    uint8 Value = readCpu8(Cpu->OpLowByte, Cpu, 4); // Cycle 4
    operations[Cpu->OpCode](Value, Cpu);

#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%02X,Y @ $%04X = #$%02X", Cpu->OpLowByte - Cpu->X, Cpu->OpLowByte, Value);
#endif
}

// 6 cycles
void zeroYIndexReadWrite(cpu *Cpu)
{
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
          
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
#endif
    
    Cpu->OpLowByte += Cpu->Y;
    Cpu->OpValue = readCpu8(Cpu->OpLowByte, Cpu, 4); // Cycle 4

#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%02X,Y @ $%04X = #$%02X", Cpu->OpLowByte - Cpu->X, Cpu->OpLowByte, Cpu->OpValue);
#endif

    Cpu->OpValue = operations[Cpu->OpCode](Cpu->OpValue, Cpu);
    pollInterrupts(Cpu, 5); // Cycle 5
    writeCpu8(Cpu->OpValue, Cpu->OpLowByte, Cpu, 6); // Cycle 6
}

// 4 cycles
void zeroYIndexWrite(cpu *Cpu)
{
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
          
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
#endif
    
    Cpu->OpLowByte += Cpu->Y;
    pollInterrupts(Cpu, 3); // Cycle 3

#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%02X,Y @ $%04X = #$%02X", Cpu->OpLowByte - Cpu->X, Cpu->OpLowByte, readCpu8(Cpu->OpLowByte, Cpu));
#endif

    uint8 Value = operations[Cpu->OpCode](0, Cpu);
    writeCpu8(Value, Cpu->OpLowByte, Cpu, 4); // Cycle 4
}

///////////////////////
//// ABSOLUTE MODE ////
///////////////////////

// 3 cycles
void absJmp(cpu *Cpu)
{
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
    Cpu->OpHighByte = readCpu8(Cpu->PrgCounter++, Cpu, 3); // Cycle 3

#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
    sprintf(Cpu->LogData2, "%02X", Cpu->OpHighByte);
#endif
    
    pollInterrupts(Cpu, 2); // Cycle 2
    operations[Cpu->OpCode](0, Cpu);

#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%04X", (Cpu->OpHighByte << 8) | Cpu->OpLowByte);
#endif
}

// 4 cycles
void absRead(cpu *Cpu)
{
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
    Cpu->OpHighByte = readCpu8(Cpu->PrgCounter++, Cpu, 3); // Cycle 3

#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
    sprintf(Cpu->LogData2, "%02X", Cpu->OpHighByte);
#endif
    
    pollInterrupts(Cpu, 3); // Cycle 3
    uint8 Value = readCpu8(((Cpu->OpHighByte << 8) | Cpu->OpLowByte), Cpu, 4); // Cycle 4
    operations[Cpu->OpCode](Value, Cpu);
               
#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%04X = #$%02X", (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Value);
#endif
}

// 6 cycles
void absReadWrite(cpu *Cpu)
{
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
    Cpu->OpHighByte = readCpu8(Cpu->PrgCounter++, Cpu, 3); // Cycle 3

#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
    sprintf(Cpu->LogData2, "%02X", Cpu->OpHighByte);
#endif
    
    Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 4); // Cycle 4

#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%04X = #$%02X", (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu->OpValue);
#endif

    Cpu->OpValue = operations[Cpu->OpCode](Cpu->OpValue, Cpu);
    pollInterrupts(Cpu, 5); // Cycle 5
    writeCpu8(Cpu->OpValue, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 6); // CYcle 6
}

// 4 cycles
void absWrite(cpu *Cpu)
{
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
    Cpu->OpHighByte = readCpu8(Cpu->PrgCounter++, Cpu, 3); // Cycle 3

#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
    sprintf(Cpu->LogData2, "%02X", Cpu->OpHighByte);
#endif
    
    pollInterrupts(Cpu, 3); // Cycle 3

#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%04X = #$%02X", (Cpu->OpHighByte << 8) | Cpu->OpLowByte, readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu));
#endif

    uint8 Value = operations[Cpu->OpCode](0, Cpu);
    writeCpu8(Value, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 4); // Cycle 4
}

///////////////////////////////
//// ABSOLUTE X INDEX MODE ////
///////////////////////////////

// Cycles 4 or 5 page fix
void absXIndexRead(cpu *Cpu)
{
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
            
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
#endif

    Cpu->OpHighByte = readCpu8(Cpu->PrgCounter++, Cpu, 3); // Cycle 3
    Cpu->OpTemp = Cpu->OpLowByte; 
    Cpu->OpLowByte += Cpu->X; 
          
#if CPU_LOG
    sprintf(Cpu->LogData2, "%02X", Cpu->OpHighByte);
#endif
 
    uint8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 4); // Cycle 4
   
    if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
    {
        Cpu->OpClockTotal += 1;
        ++Cpu->OpHighByte;
        pollInterrupts(Cpu, 4); // Cycle 4
        uint8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 5); // Cycle 5
        operations[Cpu->OpCode](Value, Cpu);
    }
    else // else read was fine, execute instruction, end op
    {
        pollInterrupts(Cpu, 3); // Cycle 3
        operations[Cpu->OpCode](Value, Cpu);
    }

#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%04X,X @ $%04X = #$%02X",
            ((Cpu->OpHighByte << 8) | Cpu->OpLowByte) - Cpu->X, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Value);
#endif
}

// 7 cycles
void absXIndexReadWrite(cpu *Cpu)
{
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
            
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
#endif

    Cpu->OpHighByte = readCpu8(Cpu->PrgCounter++, Cpu, 3); // Cycle 3
    Cpu->OpTemp = Cpu->OpLowByte; 
    Cpu->OpLowByte += Cpu->X; 
          
#if CPU_LOG
    sprintf(Cpu->LogData2, "%02X", Cpu->OpHighByte);
#endif
    
    Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 4); // Cycle 4

    if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
    {
        ++Cpu->OpHighByte; 
    }
    
    Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 5); // Cycle 5

#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%04X,X @ $%04X = #$%02X",
            ((Cpu->OpHighByte << 8) | Cpu->OpLowByte) - Cpu->X, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu->OpValue);
#endif
    
    Cpu->OpValue = operations[Cpu->OpCode](Cpu->OpValue, Cpu);
    pollInterrupts(Cpu, 6); // Cycle 6
    writeCpu8(Cpu->OpValue, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 7); // Cycle 7
}

// 5 cycles
void absXIndexWrite(cpu *Cpu)
{
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
            
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
#endif

    Cpu->OpHighByte = readCpu8(Cpu->PrgCounter++, Cpu, 3); // Cycle 3
    Cpu->OpTemp = Cpu->OpLowByte; 
    Cpu->OpLowByte += Cpu->X; 
          
#if CPU_LOG
    sprintf(Cpu->LogData2, "%02X", Cpu->OpHighByte);
#endif

    Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 4); // Cycle 4

    if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
    {
        ++Cpu->OpHighByte; 
    }

#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%04X,X @ $%04X = #$%02X",
            ((Cpu->OpHighByte << 8) | Cpu->OpLowByte) - Cpu->X, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu->OpValue);
#endif
        
    pollInterrupts(Cpu, 4); // Cycle 4
    uint8 Value = operations[Cpu->OpCode](0, Cpu);
    writeCpu8(Value, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 5); // Cycle 5

}

///////////////////////////////
//// ABSOLUTE Y INDEX MODE ////
///////////////////////////////

// 4 or 5 cycles, fixed page dependant
void absYIndexRead(cpu *Cpu)
{
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
    Cpu->OpHighByte = readCpu8(Cpu->PrgCounter++, Cpu, 3); // Cycle 3
            
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
    sprintf(Cpu->LogData2, "%02X", Cpu->OpHighByte);
#endif
    
    Cpu->OpTemp = Cpu->OpLowByte;
    Cpu->OpLowByte += Cpu->Y;        
    uint8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 4); // Cycle 4
        
    if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
    {
        Cpu->OpClockTotal += 1;
        ++Cpu->OpHighByte;
        pollInterrupts(Cpu, 4); // Cycle 4
        // NOTE: Only reaches here if the page was crossed
        uint8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 5); // Cycle 5
        operations[Cpu->OpCode](Value, Cpu);
    }
    else // else read was fine, execute instruction, end op
    {
        pollInterrupts(Cpu, 3); // Cycle 3
        operations[Cpu->OpCode](Value, Cpu);
    }

#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%04X,Y @ $%04X = #$%02X",
            ((Cpu->OpHighByte << 8) | Cpu->OpLowByte) - Cpu->Y, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Value);
#endif
}

// 7 cycles
void absYIndexReadWrite(cpu *Cpu)
{
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
    Cpu->OpHighByte = readCpu8(Cpu->PrgCounter++, Cpu, 3); // Cycle 3
            
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
    sprintf(Cpu->LogData2, "%02X", Cpu->OpHighByte);
#endif
    
    Cpu->OpTemp = Cpu->OpLowByte;
    Cpu->OpLowByte += Cpu->Y;    
    Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 4); // Cycle 4

    if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
    {
        ++Cpu->OpHighByte; 
    }

    Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 5); // Cycle 5

#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%04X,Y @ $%04X = #$%02X",
            ((Cpu->OpHighByte << 8) | Cpu->OpLowByte) - Cpu->Y, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu->OpValue);
#endif

    Cpu->OpValue = operations[Cpu->OpCode](Cpu->OpValue, Cpu);
    pollInterrupts(Cpu, 6); // Cycle 6
    writeCpu8(Cpu->OpValue, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 7); // Cycle 7
}

// 5 cycles
void absYIndexWrite(cpu *Cpu)
{
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
    Cpu->OpHighByte = readCpu8(Cpu->PrgCounter++, Cpu, 3); // Cycle 3
            
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
    sprintf(Cpu->LogData2, "%02X", Cpu->OpHighByte);
#endif
    
    Cpu->OpTemp = Cpu->OpLowByte;
    Cpu->OpLowByte += Cpu->Y;
    Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 4); // Cycle 4
        
    if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
    {
        ++Cpu->OpHighByte; 
    }

    pollInterrupts(Cpu, 4); // Cycle 4

#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " $%04X,Y @ $%04X = #$%02X",
            ((Cpu->OpHighByte << 8) | Cpu->OpLowByte) - Cpu->Y, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu->OpValue);
#endif

    uint8 Value = operations[Cpu->OpCode](0, Cpu);
    writeCpu8(Value, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 5); // Cycle 5
}

///////////////////////////////
//// INDEXED INDIRECT MODE ////
///////////////////////////////

// 6 Cycles
void idxXRead(cpu *Cpu)
{
    Cpu->OpValue = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
                
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpValue);
#endif
    
    Cpu->OpValue += Cpu->X;
    Cpu->OpLowByte = readCpu8(Cpu->OpValue, Cpu, 4); // Cycle 4
    uint8 Temp = (Cpu->OpValue+1) & 0xFF;
    Cpu->OpHighByte = readCpu8(Temp, Cpu, 5); // Cycle 5
    pollInterrupts(Cpu, 5); // Cycle 5
    uint8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 6); // Cycle 6
    operations[Cpu->OpCode](Value, Cpu);
}

// 8 Cycles
void idxXReadWrite(cpu *Cpu)
{
    Cpu->OpValue = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
                
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpValue);
#endif
    
    Cpu->OpValue += Cpu->X;
    Cpu->OpLowByte = readCpu8(Cpu->OpValue, Cpu, 4); // Cycle 4
    uint8 Temp = (Cpu->OpValue+1) & 0xFF;
    Cpu->OpHighByte = readCpu8(Temp, Cpu, 5); // Cycle 5
    Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 6); // Cycle 6
    Cpu->OpValue = operations[Cpu->OpCode](Cpu->OpValue, Cpu);
    pollInterrupts(Cpu, 7); // Cycle 7
    writeCpu8(Cpu->OpValue, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 8); // Cycle 8
}

// 6 Cycles
void idxXWrite(cpu *Cpu)
{
    Cpu->OpValue = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2
                
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpValue);
#endif
    
    Cpu->OpValue += Cpu->X;
    Cpu->OpLowByte = readCpu8(Cpu->OpValue, Cpu, 4); // Cycle 4
    uint8 Temp = (Cpu->OpValue+1) & 0xFF;
    Cpu->OpHighByte = readCpu8(Temp, Cpu, 5); // Cycle 5
    pollInterrupts(Cpu, 5); // Cycle 5
    uint8 Value = operations[Cpu->OpCode](0, Cpu);
    writeCpu8(Value, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 6); // Cycle 6
}

///////////////////////////////
//// INDIRECT INDEXED MODE ////
///////////////////////////////

// 5 or 6 cycles
void idxYRead(cpu *Cpu)
{
    Cpu->OpValue = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2

#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpValue);
#endif
    
    Cpu->OpLowByte = readCpu8(Cpu->OpValue, Cpu, 3); // Cycle 3
    uint8 Temp = (Cpu->OpValue + 1) & 0xFF;
    Cpu->OpHighByte = readCpu8(Temp, Cpu, 4); // Cycle 4
    Cpu->OpTemp = Cpu->OpLowByte;
    Cpu->OpLowByte += Cpu->Y;
    uint8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 5); // Cycle 5
        
    if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
    {
        Cpu->OpClockTotal += 1;
        ++Cpu->OpHighByte;
        pollInterrupts(Cpu, 5); // Cycle 5
            
        uint8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 6); // Cycle 6
        operations[Cpu->OpCode](Value, Cpu);
    }
    else
    {
        pollInterrupts(Cpu, 4); // Cycle 4
        operations[Cpu->OpCode](Value, Cpu);
    }

#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " ($%s),Y @ $%04X = #$%02X",
            Cpu->LogData1, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Value);
#endif
}

// 8 cycles
void idxYReadWrite(cpu *Cpu)
{
    Cpu->OpValue = readCpu8(Cpu->PrgCounter++, Cpu, 2); // Cycle 2

#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpValue);
#endif
    
    Cpu->OpLowByte = readCpu8(Cpu->OpValue, Cpu, 3); // Cycle 3
    uint8 Temp = (Cpu->OpValue + 1) & 0xFF;
    Cpu->OpHighByte = readCpu8(Temp, Cpu, 4); // Cycle 4
    Cpu->OpTemp = Cpu->OpLowByte;
    Cpu->OpLowByte += Cpu->Y;
    Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 5); // Cycle 5
    
    if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
    {
        ++Cpu->OpHighByte; 
    }
    Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 6); // Cycle 6
        
#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " ($%s),Y @ $%04X = #$%02X",
            Cpu->LogData1, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu->OpValue);
#endif
    
    Cpu->OpValue = operations[Cpu->OpCode](Cpu->OpValue, Cpu);
    pollInterrupts(Cpu, 7); // Cycle 7
    writeCpu8(Cpu->OpValue, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu, 8); // Cycle 8
}

// 6 cycles
void idxYWrite(cpu *Cpu)
{
    Cpu->OpValue = readCpu8(Cpu->PrgCounter++, Cpu); // Cycle 2

#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpValue);
#endif
    
    Cpu->OpLowByte = readCpu8(Cpu->OpValue, Cpu); // Cycle 3
    uint8 Temp = (Cpu->OpValue + 1) & 0xFF;
    Cpu->OpHighByte = readCpu8(Temp, Cpu); // Cycle 4
    Cpu->OpTemp = Cpu->OpLowByte;
    Cpu->OpLowByte += Cpu->Y;
    Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu); // Cycle 5
        
    if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
    {
        ++Cpu->OpHighByte; 
    }
                  
    pollInterrupts(Cpu); // Cycle 5
    
#if CPU_LOG
    sprintf(Cpu->LogExtraInfo, " ($%s),Y @ $%04X = #$%02X",
            Cpu->LogData1, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu->OpValue);
#endif
    
    uint8 Value = operations[Cpu->OpCode](0, Cpu); // Cycle 6
    writeCpu8(Value, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu); // Cycle 6
}

///////////////////////////
//// ABSOLUTE INDIRECT ////
///////////////////////////

// 5 cycles
void absIndJmp(cpu *Cpu)
{
    Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu); // Cycle 2
    Cpu->OpHighByte = readCpu8(Cpu->PrgCounter++, Cpu); // Cycle 3
    
#if CPU_LOG
    sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
    sprintf(Cpu->LogData2, "%02X", Cpu->OpHighByte);
#endif

    Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);  // Cycle 4
    pollInterrupts(Cpu); // Cycle 4
    uint8 IndLowByte = (uint8)(Cpu->OpLowByte + 1);
    Cpu->OpHighByte = readCpu8((Cpu->OpHighByte << 8) | IndLowByte, Cpu); // Cycle 5
    Cpu->OpLowByte = Cpu->OpValue;
    operations[Cpu->OpCode](0, Cpu); // Cycle 5
}

#define ADDRESS_MODE_COUNT 30

void (*operationAddressModes[ADDRESS_MODE_COUNT])(cpu *Cpu) =
{
    accumulator, implied, immediate, relative,
    zeroRead, zeroReadWrite, zeroWrite,
    zeroXIndexRead, zeroXIndexReadWrite, zeroXIndexWrite,
    zeroYIndexRead, zeroYIndexReadWrite, zeroYIndexWrite,
    absRead, absReadWrite, absWrite,
    absXIndexRead, absXIndexReadWrite, absXIndexWrite,
    absYIndexRead, absYIndexReadWrite, absYIndexWrite,
    idxXRead, idxXReadWrite, idxXWrite,
    idxYRead, idxYReadWrite, idxYWrite,
    absJmp, absIndJmp
};
