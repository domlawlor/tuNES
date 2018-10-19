/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#include "cpu.h"

// Implied Operations

u8 clc(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 2)
    {
        clearCarry(&Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
u8 cld(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 2)
    {
        clearDecimal(&Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
u8 cli(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 2)
    {
        clearInterrupt(&Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
u8 clv(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 2)
    {
        clearOverflow(&Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
u8 dex(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 2)
    {
        --Cpu->X;
        setZero(Cpu->X, &Cpu->Flags);
        setNegative(Cpu->X, &Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
u8 dey(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 2)
    {
        --Cpu->Y;
        setZero(Cpu->Y, &Cpu->Flags);
        setNegative(Cpu->Y, &Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
u8 inx(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 2)
    {
        ++Cpu->X;
        setZero(Cpu->X, &Cpu->Flags);
        setNegative(Cpu->X, &Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
u8 iny(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 2)
    {
        ++Cpu->Y;
        setZero(Cpu->Y, &Cpu->Flags);
        setNegative(Cpu->Y, &Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
u8 sec(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 2)
    {
        setCarry(&Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
u8 sed(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 2)
    {
        setDecimal(&Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
u8 sei(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 2)
    {
        setInterrupt(&Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
u8 tax(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 2)
    {
        Cpu->X = Cpu->A;
        setZero(Cpu->X, &Cpu->Flags);
        setNegative(Cpu->X, &Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    
    return(0);
}
u8 tay(u8 Value, cpu *Cpu)
{   
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 2)
    { 
        Cpu->Y = Cpu->A;
        setZero(Cpu->Y, &Cpu->Flags);
        setNegative(Cpu->Y, &Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
u8 tsx(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 2)
    {
        Cpu->X = Cpu->StackPtr;
        setZero(Cpu->X, &Cpu->Flags);
        setNegative(Cpu->X, &Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
u8 txa(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 2)
    {
        Cpu->A = Cpu->X;
        setZero(Cpu->A, &Cpu->Flags);
        setNegative(Cpu->A, &Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
u8 tya(u8 Value, cpu *Cpu)
{   
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 2)
    {
        Cpu->A = Cpu->Y;
        setZero(Cpu->A, &Cpu->Flags);
        setNegative(Cpu->A, &Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
u8 txs(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 2)
    {
        Cpu->StackPtr = Cpu->X;
        Cpu->NextCycle = 1;
    }
    return(0);
}

// TODO: Condense irq and nmi in future
u8 irq(u8 Value, cpu *Cpu)
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
        Cpu->PrgCounter = (Cpu->PrgCounter & 0xFF00) | read8(IRQ_BRK_VEC, Cpu->MemoryBase);        
    }
    else if(Cpu->Cycle == 7)
    {
        Cpu->PrgCounter = (read8(IRQ_BRK_VEC+1, Cpu->MemoryBase) << 8) | (Cpu->PrgCounter & 0xFF);
        clearBreak(&Cpu->Flags);
        setInterrupt(&Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}


u8 nmi(u8 Value, cpu *Cpu)
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
    }
    return(0);
}

u8 brk(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 2)
    {
        readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        u8 HighByte = (u8)(Cpu->PrgCounter >> 8);
        writeStack(HighByte, Cpu);
        decrementStack(Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        u8 LowByte = (u8)Cpu->PrgCounter; 
        writeStack(LowByte, Cpu);
        decrementStack(Cpu);
        pollInterrupts(Cpu);
        // TODO: NMI Hijack
    }
    else if(Cpu->Cycle == 5)
    {
        setBlank(&Cpu->Flags);
        setBreak(&Cpu->Flags);
        writeStack(Cpu->Flags, Cpu);
        decrementStack(Cpu);
        setInterrupt(&Cpu->Flags);
    }
    else if(Cpu->Cycle == 6)
    {        
        Cpu->PrgCounter = (Cpu->PrgCounter & 0xFF00) | read8(IRQ_BRK_VEC, Cpu->MemoryBase);
    }
    else if(Cpu->Cycle == 7)
    {
        Cpu->PrgCounter = (read8(IRQ_BRK_VEC + 1, Cpu->MemoryBase) << 8) | (Cpu->PrgCounter & 0x00FF);
        Cpu->NextCycle = 1;
    }
    return(0);
}

u8 rti(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 2)
    {
        readCpu8(Cpu->PrgCounter, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        incrementStack(Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        Cpu->Flags = readStack(Cpu);
        incrementStack(Cpu);
    }
    else if(Cpu->Cycle == 5)
    {
        Cpu->PrgCounter = (Cpu->PrgCounter & 0xFF00) | readStack(Cpu);
        incrementStack(Cpu);
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 6)
    {
        Cpu->PrgCounter = (readStack(Cpu) << 8) | (Cpu->PrgCounter & 0x00FF);
        Cpu->NextCycle = 1;
    }
    return(0);
}

u8 rts(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 2)
    {
        readCpu8(Cpu->PrgCounter, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        incrementStack(Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        Cpu->PrgCounter = Cpu->PrgCounter & 0xFF00 | readStack(Cpu);
        incrementStack(Cpu);
    }
    else if(Cpu->Cycle == 5)
    {
        Cpu->PrgCounter =  (readStack(Cpu) << 8) | (Cpu->PrgCounter & 0x00FF);
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 6)
    {
        ++Cpu->PrgCounter;
        Cpu->NextCycle = 1;
    }
    
    return(0);
}

u8 pha(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 2)
    {
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        writeStack(Cpu->A, Cpu);
        decrementStack(Cpu);
        Cpu->NextCycle = 1;
    }
    return(0);
}

u8 php(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 2)
    {
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        setBreak(&Cpu->Flags);
        setBlank(&Cpu->Flags);
        writeStack(Cpu->Flags, Cpu);
        decrementStack(Cpu);
        Cpu->NextCycle = 1;
    }
    return(0);
}

u8 pla(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 3)
    {
        incrementStack(Cpu);
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        Cpu->A = readStack(Cpu);
        setZero(Cpu->A, &Cpu->Flags);
        setNegative(Cpu->A, &Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}

u8 plp(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 3)
    {
        incrementStack(Cpu);
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        Cpu->Flags = readStack(Cpu);
        Cpu->NextCycle = 1;
    }
    
    return(0);
}

u8 jsr(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 2)
    {
        Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu);
            
#if CPU_LOG
        sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
        sprintf(Cpu->LogData2, "%02X", readCpu8(Cpu->PrgCounter, Cpu));
#endif
    }
    // Nothing on cycle 3
    else if(Cpu->Cycle == 4)
    {
        writeStack((Cpu->PrgCounter >> 8), Cpu);
        decrementStack(Cpu);
    }
    else if(Cpu->Cycle == 5)
    {
        writeStack(Cpu->PrgCounter & 0xFF, Cpu);
        decrementStack(Cpu);
    }
    else if(Cpu->Cycle == 6)
    {
        Cpu->PrgCounter = (readCpu8(Cpu->PrgCounter, Cpu) << 8) | Cpu->OpLowByte;
        Cpu->NextCycle = 1;

#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%04X", Cpu->PrgCounter);
#endif
    }

    return(0);
}

// Read
u8 lda(u8 Value, cpu *Cpu)
{    
    Cpu->A = Value;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(0);
}
u8 ldx(u8 Value, cpu *Cpu)
{
    Cpu->X = Value;
    setZero(Cpu->X, &Cpu->Flags);
    setNegative(Cpu->X, &Cpu->Flags);
    return(0);
}
u8 ldy(u8 Value, cpu *Cpu)
{
    Cpu->Y = Value;
    setZero(Cpu->Y, &Cpu->Flags);
    setNegative(Cpu->Y, &Cpu->Flags);
    return(0);
}
u8 eor(u8 Value, cpu *Cpu)
{
    Cpu->A = Cpu->A ^ Value;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(Value);
}
u8 AND(u8 Value, cpu *Cpu)
{
    Cpu->A = Cpu->A & Value;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(Value);
}

u8 ora(u8 Value, cpu *Cpu)
{
    Cpu->A = Cpu->A | Value;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(Value);
}

u8 adc(u8 Value, cpu *Cpu)
{    
    u8 A = Cpu->A;
    u8 B = Value;
    u8 C = isBitSet(CARRY_BIT, Cpu->Flags);

    u16 Sum = (u16)A + (u16)B + (u16)C;

    // Overflow check, taken from the web. One day find out how this works
    if(((A ^ Sum) & (B ^ Sum) & 0x80) == 0x80)
        setOverflow(&Cpu->Flags);
    else
        clearOverflow(&Cpu->Flags);

    if(Sum & 0x100)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);

    setZero(Sum, &Cpu->Flags);
    setNegative(Sum, &Cpu->Flags);

    Cpu->A = (u8)Sum;
    return(0);
}

u8 sbc(u8 Value, cpu *Cpu)
{   
    u8 A = Cpu->A;
    u8 B = ~Value; // NOTE: Using the inverse
    u8 C = isBitSet(CARRY_BIT, Cpu->Flags);

    u16 Sum = (u16)A + (u16)B + (u16)C;

    // Overflow check, taken from the web. One day find out how this works
    if(((A ^ Sum) & (B ^ Sum) & 0x80) == 0x80)
        setOverflow(&Cpu->Flags);
    else
        clearOverflow(&Cpu->Flags);

    if(Sum & 0x100)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);

    setZero(Sum, &Cpu->Flags);
    setNegative(Sum, &Cpu->Flags);

    Cpu->A = (u8)Sum;

    return(0);
}

u8 cmp(u8 Value, cpu *Cpu)
{
    if(Cpu->A >= Value)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
    
    u8 CmpValue = Cpu->A - Value;
    setZero(CmpValue, &Cpu->Flags);
    setNegative(CmpValue, &Cpu->Flags);
    return(0);
}

u8 bit(u8 Value, cpu *Cpu)
{    
    if(Value & (1 << 6))
        setOverflow(&Cpu->Flags);
    else
        clearOverflow(&Cpu->Flags);
    setNegative(Value, &Cpu->Flags);
    setZero(Cpu->A & Value, &Cpu->Flags);
    return(0);
}

u8 lax(u8 Value, cpu *Cpu)
{
    lda(Value, Cpu);
    ldx(Value, Cpu);
    return(0);
}

u8 nop(u8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 2)
    {
        Cpu->NextCycle = 1;
    }
    
    return(0);
}


// Read modify write

u8 asl(u8 Value, cpu *Cpu)
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

u8 lsr(u8 Value, cpu *Cpu)
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

u8 rol(u8 Value, cpu *Cpu)
{    
    u8 CarrySet = isBitSet(CARRY_BIT, Cpu->Flags);

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

u8 ror(u8 Value, cpu *Cpu)
{
    u8 CarrySet = isBitSet(CARRY_BIT, Cpu->Flags);

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

u8 inc(u8 Value, cpu *Cpu)
{
    ++Value;
    setZero(Value, &Cpu->Flags);
    setNegative(Value, &Cpu->Flags);
    return(Value);
}

u8 dec(u8 Value, cpu *Cpu)
{
    --Value;
    setZero(Value, &Cpu->Flags);
    setNegative(Value, &Cpu->Flags);
    return(Value);
}

u8 slo(u8 Value, cpu *Cpu)
{
    Value = asl(Value, Cpu);
    Value = ora(Value, Cpu);
    return(Value);
}
u8 sre(u8 Value, cpu *Cpu)
{
    Value = lsr(Value, Cpu);
    Value = eor(Value, Cpu);
    return(Value);
}

u8 rla(u8 Value, cpu *Cpu)
{
    Value = rol(Value, Cpu);
    Value = AND(Value, Cpu);
    return(Value);
}
u8 rra(u8 Value, cpu *Cpu)
{
    Value = ror(Value, Cpu);
    adc(Value, Cpu);
    return(Value);
}

u8 isc(u8 Value, cpu *Cpu)
{
    Value = inc(Value, Cpu);
    sbc(Value, Cpu);
    return(Value);
}

u8 dcp(u8 Value, cpu *Cpu)
{
    Value = dec(Value, Cpu);
    cmp(Value, Cpu);
    return(Value);
}

// Write instructions

u8 sta(u8 Value, cpu *Cpu)
{
    return(Cpu->A);
}
u8 stx(u8 Value, cpu *Cpu)
{
    return(Cpu->X);
}
u8 sty(u8 Value, cpu *Cpu)
{
    return(Cpu->Y);
}
u8 sax(u8 Value, cpu *Cpu)
{
    return(Cpu->A & Cpu->X);
}


/////////////////////////
// Relative Operations //
/////////////////////////
u8 bcc(u8 Value, cpu *Cpu)
{
    return(!isBitSet(CARRY_BIT, Cpu->Flags));
}
u8 bcs(u8 Value, cpu *Cpu)
{
    return(isBitSet(CARRY_BIT, Cpu->Flags));    
}
u8 beq(u8 Value, cpu *Cpu)
{
    u8 temp = ZERO_BIT;
    return(isBitSet(temp, Cpu->Flags));
}
u8 bmi(u8 Value, cpu *Cpu)
{
    return(isBitSet(NEGATIVE_BIT, Cpu->Flags));    
}
u8 bne(u8 Value, cpu *Cpu)
{
    return(!isBitSet(ZERO_BIT, Cpu->Flags));    
}
u8 bpl(u8 Value, cpu *Cpu)
{
    return(!isBitSet(NEGATIVE_BIT, Cpu->Flags));    
}
u8 bvc(u8 Value, cpu *Cpu)
{
    return(!isBitSet(OVERFLOW_BIT, Cpu->Flags));    
}
u8 bvs(u8 Value, cpu *Cpu)
{
    return(isBitSet(OVERFLOW_BIT, Cpu->Flags));    
}
/////////////////////


u8 jmp(u8 Value, cpu *Cpu)
{
    Cpu->PrgCounter = (Cpu->OpHighByte << 8) | Cpu->OpLowByte;
    return(0);
}

// Compares

u8 cpx(u8 Value, cpu *Cpu)
{
    if(Cpu->X >= Value)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
    
    u8 CmpValue = Cpu->X - Value;
    setZero(CmpValue, &Cpu->Flags);
    setNegative(CmpValue, &Cpu->Flags);
    return(0);
}
u8 cpy(u8 Value, cpu *Cpu)
{
    if(Cpu->Y >= Value)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
    
    u8 CmpValue = Cpu->Y - Value;
    setZero(CmpValue, &Cpu->Flags);
    setNegative(CmpValue, &Cpu->Flags);
    return(0);
}




u8 ahx(u8 Value, cpu *Cpu)
{
    //Assert(0);
    return(0);
}
u8 alr(u8 Value, cpu *Cpu)
{
    Value = AND(Value, Cpu);
    Value = lsr(Value, Cpu);
    return(Value);
}
u8 anc(u8 Value, cpu *Cpu)
{
    AND(Value, Cpu);
    
    if(isBitSet(NEGATIVE_BIT, Cpu->Flags))
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
    
    return(0);
}
u8 arr(u8 Value, cpu *Cpu)
{
    AND(Value, Cpu);    
    ror(Value, Cpu);

    u8 bit5 = Cpu->A & (1<<5);
    u8 bit6 = Cpu->A & (1<<6);

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
u8 axs(u8 Value, cpu *Cpu)
{
    u8 ANDValue = (Cpu->A & Cpu->X);
    Cpu->X = ANDValue - Value;

    if(ANDValue >= Value)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
    
    setZero(Cpu->X, &Cpu->Flags);
    setNegative(Cpu->X, &Cpu->Flags);
    
    return(0);
}
u8 kil(u8 Value, cpu *Cpu)
{
    Assert(0);
    return(0);
}
u8 las(u8 Value, cpu *Cpu)
{
    Assert(0);
    return(0); 
}
u8 shx(u8 Value, cpu *Cpu)
{
Assert(0);
    return(0);
}

u8 shy(u8 Value, cpu *Cpu)
{
    Assert(0);
//    if((Cpu->X + Value) <= 0xFF)
    //writeCpu8(Byte, Address, Cpu); // TODO: This shouldn't happen, need refactoring
    return(0);
}
u8 tas(u8 Value, cpu *Cpu)
{
    Assert(0);
    return(0);
}

u8 xaa(u8 Value, cpu *Cpu)
{
    Assert(0);
    return(0);
}



global u8 opAddressType[INSTRUCTION_COUNT] =
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

global char * opName[INSTRUCTION_COUNT] =
{
    /*         0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F        */
    /*0*/  "BRK","ORA","NMI","SLO","NOP","ORA","ASL","SLO","PHP","ORA","ASL","ANC","NOP","ORA","ASL","SLO",
    /*1*/  "BPL","ORA","IRQ","SLO","NOP","ORA","ASL","SLO","CLC","ORA","NOP","SLO","NOP","ORA","ASL","SLO",
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

u8 (*operations[INSTRUCTION_COUNT])(u8 InByte, cpu *Cpu) =
{
    /*         0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F        */
    /*0*/    brk,ora,nmi,slo,nop,ora,asl,slo,php,ora,asl,anc,nop,ora,asl,slo,
    /*1*/    bpl,ora,irq,slo,nop,ora,asl,slo,clc,ora,nop,slo,nop,ora,asl,slo,
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


void implied(cpu *Cpu)
{
    // NOTE: Implied instructions handle how many cycles they take
    operations[Cpu->OpCode](0, Cpu);
}

//////////////////////////////////////
//// ACCUMULATOR AND IMPLIED MODE ////
//////////////////////////////////////

void accumulator(cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 2)
    {
        Cpu->A = operations[Cpu->OpCode](Cpu->A, Cpu);
        Cpu->NextCycle = 1;
    }
    // Two cycles 
}

////////////////////////
//// IMMEDIATE MODE ////
////////////////////////

void immediate(cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 2)
    {
        u8 Value = readCpu8(Cpu->PrgCounter++, Cpu);
        operations[Cpu->OpCode](Value, Cpu);
        Cpu->NextCycle = 1;
        
#if CPU_LOG
        sprintf(Cpu->LogData1, "%02X", Value);
        sprintf(Cpu->LogExtraInfo, " #$%02X", Value);
#endif
    }
}


///////////////////////
//// RELATIVE MODE ////
///////////////////////

void relative(cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        //pollInterrupts(Cpu);
    }
    if(Cpu->Cycle == 2)
    {
        Cpu->OpValue = readCpu8(Cpu->PrgCounter, Cpu);
        ++Cpu->PrgCounter;
        pollInterrupts(Cpu);
#if CPU_LOG
        sprintf(Cpu->LogData1, "%02X", Cpu->OpValue);
#endif
    }
    else if(Cpu->Cycle == 3)
    {
        // The Correct Program Counter. Will be saved into OpHigh and Low Bytes
        u16 TempPC = Cpu->PrgCounter + (int8)Cpu->OpValue;
        Cpu->OpHighByte = (TempPC & 0xFF00) >> 8;
        Cpu->OpLowByte = TempPC & 0x00FF;

        // The potentiall wrong high byte after branch
        u8 UnFixedHighByte = (Cpu->PrgCounter & 0xFF00) >> 8;

#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%04X", TempPC);
#endif
        
        u8 Branch = operations[Cpu->OpCode](0, Cpu);
        
        if(Branch)
        {            
            // Update prgcounter, could have unfixed page
            Cpu->PrgCounter = (Cpu->PrgCounter & 0xFF00) | Cpu->OpLowByte;

            // If OpHighByte isn't the same as the unfixed version, then we crossed page
            if(Cpu->OpHighByte != UnFixedHighByte)
            {
                // Page crossed
                Cpu->OpTemp = 1; //Boolean if we page crossed
            }
            else
            {
                Cpu->OpTemp = 0; //Boolean if we page crossed
            }

            pollInterrupts(Cpu);
        }
        else
        {
            Cpu->OpBranched = true; // NOTE: technically haven't 'branched' here. But still does pipelining
        }
    }
    else if(Cpu->Cycle == 4)
    {
        if(Cpu->OpTemp) // If the page was crossed then fix.
        {
            Cpu->PrgCounter = (Cpu->OpHighByte << 8) | (Cpu->PrgCounter & 0xFF);
            pollInterrupts(Cpu);
        }
        else
        {
            Cpu->OpBranched = true;
        }
    }
    else if(Cpu->Cycle == 5)
    {
        Cpu->OpBranched = true;
    }
}

////////////////////////
//// ZERO PAGE MODE ////
////////////////////////

void zeroCommon(cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 2)
    {
        Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu);
        
#if CPU_LOG
        sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
#endif
    }
}

void zeroRead(cpu *Cpu)
{
    zeroCommon(Cpu);

    if(Cpu->Cycle == 2)
    {
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        u8 Value = readCpu8(Cpu->OpLowByte, Cpu);
        operations[Cpu->OpCode](Value, Cpu);
        Cpu->NextCycle = 1;
       
#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%04X = #$%02X", Cpu->OpLowByte, Value);
#endif
    }
}

void zeroReadWrite(cpu *Cpu)
{
    zeroCommon(Cpu);
    
    if(Cpu->Cycle == 3)
    {
        Cpu->OpValue = readCpu8(Cpu->OpLowByte, Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        Cpu->OpValue = operations[Cpu->OpCode](Cpu->OpValue, Cpu);
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 5)
    {
        writeCpu8(Cpu->OpValue, Cpu->OpLowByte, Cpu);
        Cpu->NextCycle = 1;
  
#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%04X = #$%02X", Cpu->OpLowByte, Cpu->OpValue);
#endif
    }    
}

void zeroWrite(cpu *Cpu)
{
    zeroCommon(Cpu);

    if(Cpu->Cycle == 2)
    {
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%04X = #$%02X", Cpu->OpLowByte, readCpu8(Cpu->OpLowByte, Cpu));
#endif
        
        u8 Value = operations[Cpu->OpCode](0, Cpu);
        writeCpu8(Value, Cpu->OpLowByte, Cpu);
        Cpu->NextCycle = 1;
    }
}

//////////////////////////
//// ZERO PAGE X MODE ////
//////////////////////////

void zeroXCommon(cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 2)
    {
        Cpu->OpLowByte = readCpu8(Cpu->PrgCounter, Cpu);
        ++Cpu->PrgCounter;
          
#if CPU_LOG
        sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
#endif
    }
    else if(Cpu->Cycle == 3)
    {
        Cpu->OpLowByte += Cpu->X;        
    }
}

void zeroXIndexRead(cpu *Cpu)
{
    zeroXCommon(Cpu);

    if(Cpu->Cycle == 3)
    {
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        u8 Value = readCpu8(Cpu->OpLowByte, Cpu);
        operations[Cpu->OpCode](Value, Cpu);
        Cpu->NextCycle = 1;
        
#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%02X,X @ $%04X = #$%02X", Cpu->OpLowByte - Cpu->X, Cpu->OpLowByte, Value);
#endif
    }
}

void zeroXIndexReadWrite(cpu *Cpu)
{
    zeroXCommon(Cpu);

    if(Cpu->Cycle == 4)
    {
        Cpu->OpValue = readCpu8(Cpu->OpLowByte, Cpu);

#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%02X,X @ $%04X = #$%02X", Cpu->OpLowByte - Cpu->X, Cpu->OpLowByte, Cpu->OpValue);
#endif
    }
    else if(Cpu->Cycle == 5)
    {
        Cpu->OpValue = operations[Cpu->OpCode](Cpu->OpValue, Cpu);
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 6)
    {
        writeCpu8(Cpu->OpValue, Cpu->OpLowByte, Cpu);
        Cpu->NextCycle = 1;
    }
}

void zeroXIndexWrite(cpu *Cpu)
{
    zeroXCommon(Cpu);
    
    if(Cpu->Cycle == 3)
    {
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 4)
    {  
#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%02X,X @ $%04X = #$%02X", Cpu->OpLowByte - Cpu->X, Cpu->OpLowByte, readCpu8(Cpu->OpLowByte, Cpu));
#endif

        u8 Value = operations[Cpu->OpCode](0, Cpu);
        writeCpu8(Value, Cpu->OpLowByte, Cpu);
        Cpu->NextCycle = 1;      
    }
}

//////////////////////////
//// ZERO PAGE Y MODE ////
//////////////////////////

void zeroYCommon(cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 2)
    {
        Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu);
          
#if CPU_LOG
        sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
#endif
    }
    else if(Cpu->Cycle == 3)
    {
        Cpu->OpLowByte += Cpu->Y;        
    }
}

void zeroYIndexRead(cpu *Cpu)
{
    zeroYCommon(Cpu);

    if(Cpu->Cycle == 3)
    {
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        u8 Value = readCpu8(Cpu->OpLowByte, Cpu);
        operations[Cpu->OpCode](Value, Cpu);
        Cpu->NextCycle = 1;
            
#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%02X,Y @ $%04X = #$%02X", Cpu->OpLowByte - Cpu->X, Cpu->OpLowByte, Value);
#endif
    }   
}

void zeroYIndexReadWrite(cpu *Cpu)
{
    zeroYCommon(Cpu);
    
    if(Cpu->Cycle == 4)
    {
        Cpu->OpValue = readCpu8(Cpu->OpLowByte, Cpu);

#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%02X,Y @ $%04X = #$%02X", Cpu->OpLowByte - Cpu->X, Cpu->OpLowByte, Cpu->OpValue);
#endif
    }
    else if(Cpu->Cycle == 5)
    {
        Cpu->OpValue = operations[Cpu->OpCode](Cpu->OpValue, Cpu);
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 6)
    {
        writeCpu8(Cpu->OpValue, Cpu->OpLowByte, Cpu);
        Cpu->NextCycle = 1;
    }
}

void zeroYIndexWrite(cpu *Cpu)
{
    zeroYCommon(Cpu);
    
    if(Cpu->Cycle == 3)
    {
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 4)
    {      
#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%02X,Y @ $%04X = #$%02X", Cpu->OpLowByte - Cpu->X, Cpu->OpLowByte, readCpu8(Cpu->OpLowByte, Cpu));
#endif

        u8 Value = operations[Cpu->OpCode](0, Cpu);
        writeCpu8(Value, Cpu->OpLowByte, Cpu);
        Cpu->NextCycle = 1;
    }
}

///////////////////////
//// ABSOLUTE MODE ////
///////////////////////

void absCommon(cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 2)
    {
        Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu);
          
#if CPU_LOG
        sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
#endif
    }
    else if(Cpu->Cycle == 3)
    {
        Cpu->OpHighByte = readCpu8(Cpu->PrgCounter++, Cpu);
          
#if CPU_LOG
        sprintf(Cpu->LogData2, "%02X", Cpu->OpHighByte);
#endif
    }
}
    
void absJmp(cpu *Cpu)
{
    absCommon(Cpu);
    
    if(Cpu->Cycle == 2)
    {
        pollInterrupts(Cpu);
    }
    if(Cpu->Cycle == 3)
    {   
#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%04X", (Cpu->OpHighByte << 8) | Cpu->OpLowByte);
#endif
        
        operations[Cpu->OpCode](0, Cpu);
        Cpu->NextCycle = 1;
    }
}

void absRead(cpu *Cpu)
{
    absCommon(Cpu);

    if(Cpu->Cycle == 3)
    {
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        u8 Value = readCpu8(((Cpu->OpHighByte << 8) | Cpu->OpLowByte), Cpu);
        operations[Cpu->OpCode](Value, Cpu);
        Cpu->NextCycle = 1;
               
#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%04X = #$%02X", (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Value);
#endif
    }
}

void absReadWrite(cpu *Cpu)
{
    absCommon(Cpu);
    
    if(Cpu->Cycle == 4)
    {
        Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);

#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%04X = #$%02X", (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu->OpValue);
#endif
    }
    else if(Cpu->Cycle == 5)
    {
        Cpu->OpValue = operations[Cpu->OpCode](Cpu->OpValue, Cpu);
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 6)
    {
        writeCpu8(Cpu->OpValue, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        Cpu->NextCycle = 1;
    }
}

void absWrite(cpu *Cpu)
{
    absCommon(Cpu);
    
    if(Cpu->Cycle == 3)
    {
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%04X = #$%02X", (Cpu->OpHighByte << 8) | Cpu->OpLowByte, readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu));
#endif

        u8 Value = operations[Cpu->OpCode](0, Cpu);
        writeCpu8(Value, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        Cpu->NextCycle = 1;
    }
}

///////////////////////////////
//// ABSOLUTE X INDEX MODE ////
///////////////////////////////

void absXIndexCommon(cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 2)
    {
        Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu);
            
#if CPU_LOG
        sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
#endif
    }
    else if(Cpu->Cycle == 3)
    {
        Cpu->OpHighByte = readCpu8(Cpu->PrgCounter++, Cpu);
        Cpu->OpTemp = Cpu->OpLowByte;
        Cpu->OpLowByte += Cpu->X;
          
#if CPU_LOG
        sprintf(Cpu->LogData2, "%02X", Cpu->OpHighByte);
#endif
    }
}

void absXIndexRead(cpu *Cpu)
{
    absXIndexCommon(Cpu);

    if(Cpu->Cycle == 3 && Cpu->OpLowByte >= Cpu->OpTemp)
    {
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 4)
    {        
        u8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
   
        if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
        {
            ++Cpu->OpHighByte;
            pollInterrupts(Cpu);
        }
        else // else read was fine, execute instruction, end op
        {
            operations[Cpu->OpCode](Value, Cpu);
            Cpu->NextCycle = 1;

#if CPU_LOG
            sprintf(Cpu->LogExtraInfo, " $%04X,X @ $%04X = #$%02X",
                    ((Cpu->OpHighByte << 8) | Cpu->OpLowByte) - Cpu->X, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Value);
#endif

        }
    }
    else if(Cpu->Cycle == 5)
    {
// NOTE: Only reaches here if the page was crossed
        u8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        operations[Cpu->OpCode](Value, Cpu);
        Cpu->NextCycle = 1;

#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%04X,X @ $%04X = #$%02X",
                ((Cpu->OpHighByte << 8) | Cpu->OpLowByte) - Cpu->X, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Value);
#endif
    }
}

void absXIndexReadWrite(cpu *Cpu)
{
    absXIndexCommon(Cpu);
    
    if(Cpu->Cycle == 4)
    {
        Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);

        if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
        {
            ++Cpu->OpHighByte; 
        }
    }
    else if(Cpu->Cycle == 5)
    {
        Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);

#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%04X,X @ $%04X = #$%02X",
                ((Cpu->OpHighByte << 8) | Cpu->OpLowByte) - Cpu->X, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu->OpValue);
#endif
    }
    else if(Cpu->Cycle == 6)
    {
        Cpu->OpValue = operations[Cpu->OpCode](Cpu->OpValue, Cpu);
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 7)
    {
        writeCpu8(Cpu->OpValue, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        Cpu->NextCycle = 1;
    }
}

void absXIndexWrite(cpu *Cpu)
{
    absXIndexCommon(Cpu);
    
    if(Cpu->Cycle == 4)
    {
        Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);

        if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
        {
            ++Cpu->OpHighByte; 
        }

#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%04X,X @ $%04X = #$%02X",
                ((Cpu->OpHighByte << 8) | Cpu->OpLowByte) - Cpu->X, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu->OpValue);
#endif

        
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 5)
    {        
        u8 Value = operations[Cpu->OpCode](0, Cpu);
        writeCpu8(Value, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        Cpu->NextCycle = 1;      
    }
}

///////////////////////////////
//// ABSOLUTE Y INDEX MODE ////
///////////////////////////////

void absYIndexCommon(cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 2)
    {
        Cpu->OpLowByte = readCpu8(Cpu->PrgCounter, Cpu);
        ++Cpu->PrgCounter;
            
#if CPU_LOG
        sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
#endif
    }
    else if(Cpu->Cycle == 3)
    {
        Cpu->OpHighByte = readCpu8(Cpu->PrgCounter++, Cpu);
        Cpu->OpTemp = Cpu->OpLowByte;
        Cpu->OpLowByte += Cpu->Y;
            
#if CPU_LOG
        sprintf(Cpu->LogData2, "%02X", Cpu->OpHighByte);
#endif
    }
}

void absYIndexRead(cpu *Cpu)
{
    absYIndexCommon(Cpu);

    if(Cpu->Cycle == 3 && Cpu->OpLowByte >= Cpu->OpTemp)
    {
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        u8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        
        if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
        {
            ++Cpu->OpHighByte;
            pollInterrupts(Cpu);
        }
        else // else read was fine, execute instruction, end op
        {
            operations[Cpu->OpCode](Value, Cpu);
            Cpu->NextCycle = 1;

#if CPU_LOG
            sprintf(Cpu->LogExtraInfo, " $%04X,Y @ $%04X = #$%02X",
                    ((Cpu->OpHighByte << 8) | Cpu->OpLowByte) - Cpu->Y, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Value);
#endif
        }
    }
    else if(Cpu->Cycle == 5)
    {
        // NOTE: Only reaches here if the page was crossed
        u8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        operations[Cpu->OpCode](Value, Cpu);
        Cpu->NextCycle = 1;

#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%04X,Y @ $%04X = #$%02X",
                ((Cpu->OpHighByte << 8) | Cpu->OpLowByte) - Cpu->Y, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Value);
#endif

    }
}

void absYIndexReadWrite(cpu *Cpu)
{
    //Assert(0); // NOTE: Not ment to get here??
    absYIndexCommon(Cpu);
    
    if(Cpu->Cycle == 4)
    {
        Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);

        if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
        {
            ++Cpu->OpHighByte; 
        }
    }
    else if(Cpu->Cycle == 5)
    {
        Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);

#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%04X,Y @ $%04X = #$%02X",
                ((Cpu->OpHighByte << 8) | Cpu->OpLowByte) - Cpu->Y, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu->OpValue);
#endif
    }
    else if(Cpu->Cycle == 6)
    {
        Cpu->OpValue = operations[Cpu->OpCode](Cpu->OpValue, Cpu);
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 7)
    {
        writeCpu8(Cpu->OpValue, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        Cpu->NextCycle = 1;
    }
}

void absYIndexWrite(cpu *Cpu)
{
    absYIndexCommon(Cpu);
    
    if(Cpu->Cycle == 4)
    {
        Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        
        if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
        {
            ++Cpu->OpHighByte; 
        }
        
#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%04X,Y @ $%04X = #$%02X",
                ((Cpu->OpHighByte << 8) | Cpu->OpLowByte) - Cpu->Y, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu->OpValue);
#endif

        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 5)
    {
        u8 Value = operations[Cpu->OpCode](0, Cpu);
        writeCpu8(Value, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        Cpu->NextCycle = 1;
    }
}

///////////////////////////////
//// INDEXED INDIRECT MODE ////
///////////////////////////////

void idxXCommon(cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 2)
    {
        Cpu->OpValue = readCpu8(Cpu->PrgCounter, Cpu);
        ++Cpu->PrgCounter;
            
#if CPU_LOG
        sprintf(Cpu->LogData1, "%02X", Cpu->OpValue);
#endif
    }
    else if(Cpu->Cycle == 3)
    {
        Cpu->OpValue += Cpu->X;
    }
    else if(Cpu->Cycle == 4)
    {
        Cpu->OpLowByte = readCpu8(Cpu->OpValue, Cpu);
    }
    else if(Cpu->Cycle == 5)
    {
        u8 Temp = (Cpu->OpValue+1) & 0xFF;
        Cpu->OpHighByte = readCpu8(Temp, Cpu);
    }
}

void idxXRead(cpu *Cpu)
{
    idxXCommon(Cpu);

    if(Cpu->Cycle == 5)
    {
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 6)
    {
        u8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        operations[Cpu->OpCode](Value, Cpu);
        Cpu->NextCycle = 1;
    }
}

void idxXReadWrite(cpu *Cpu)
{
    idxXCommon(Cpu);
    
    if(Cpu->Cycle == 6)
    {
        Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
    }
    else if(Cpu->Cycle == 7)
    {
        Cpu->OpValue = operations[Cpu->OpCode](Cpu->OpValue, Cpu);
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 8)
    {
        writeCpu8(Cpu->OpValue, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        Cpu->NextCycle = 1;
    }
}


void idxXWrite(cpu *Cpu)
{
    idxXCommon(Cpu);

    if(Cpu->Cycle == 5)
    {
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 6)
    {
        u8 Value = operations[Cpu->OpCode](0, Cpu);
        writeCpu8(Value, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        Cpu->NextCycle = 1;
    }
}

///////////////////////////////
//// INDIRECT INDEXED MODE ////
///////////////////////////////

void idxYCommon(cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 2)
    {
        Cpu->OpValue = readCpu8(Cpu->PrgCounter, Cpu);
        ++Cpu->PrgCounter;
        
#if CPU_LOG
        sprintf(Cpu->LogData1, "%02X", Cpu->OpValue);
#endif
    }
    else if(Cpu->Cycle == 3)
    {
        Cpu->OpLowByte = readCpu8(Cpu->OpValue, Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        u8 Temp = (Cpu->OpValue + 1) & 0xFF;
        Cpu->OpHighByte = readCpu8(Temp, Cpu);
        Cpu->OpTemp = Cpu->OpLowByte;
        Cpu->OpLowByte += Cpu->Y;
    }
}

void idxYRead(cpu *Cpu)
{
    idxYCommon(Cpu);

    if(Cpu->Cycle == 4 && Cpu->OpLowByte >= Cpu->OpTemp)
    {
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 5)
    {
        u8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        
        if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
        {
            ++Cpu->OpHighByte;
            pollInterrupts(Cpu);
        }
        else
        {
#if CPU_LOG
            sprintf(Cpu->LogExtraInfo, " ($%s),Y @ $%04X = #$%02X",
                    Cpu->LogData1, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Value);
#endif
            operations[Cpu->OpCode](Value, Cpu);
            Cpu->NextCycle = 1;
        }
    }
    else if(Cpu->Cycle == 6)
    {
        u8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        operations[Cpu->OpCode](Value, Cpu);
        Cpu->NextCycle = 1;

#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " ($%s),Y @ $%04X = #$%02X",
                Cpu->LogData1, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Value);
#endif
    }
}

void idxYReadWrite(cpu *Cpu)
{
    idxYCommon(Cpu);

    if(Cpu->Cycle == 5)
    {
        Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);

        if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
        {
            ++Cpu->OpHighByte; 
        }
    }
    else if(Cpu->Cycle == 6)
    {
        Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        
#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " ($%s),Y @ $%04X = #$%02X",
                Cpu->LogData1, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu->OpValue);
#endif
    }
    else if(Cpu->Cycle == 7)
    {
        Cpu->OpValue = operations[Cpu->OpCode](Cpu->OpValue, Cpu);
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 8)
    {
        writeCpu8(Cpu->OpValue, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        Cpu->NextCycle = 1;
    }
}

void idxYWrite(cpu *Cpu)
{
    idxYCommon(Cpu);

    if(Cpu->Cycle == 5)
    {
        Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        
        if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
        {
            ++Cpu->OpHighByte; 
        }
  
#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " ($%s),Y @ $%04X = #$%02X",
                Cpu->LogData1, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu->OpValue);
#endif
                
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 6)
    {
        u8 Value = operations[Cpu->OpCode](0, Cpu);
        writeCpu8(Value, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        Cpu->NextCycle = 1;
    }
}

///////////////////////////
//// ABSOLUTE INDIRECT ////
///////////////////////////

void absIndJmp(cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 2)
    {
        Cpu->OpLowByte = readCpu8(Cpu->PrgCounter, Cpu);
        ++Cpu->PrgCounter;
        
#if CPU_LOG
        sprintf(Cpu->LogData1, "%02X", Cpu->OpLowByte);
#endif
    }
    else if(Cpu->Cycle == 3)
    {
        Cpu->OpHighByte = readCpu8(Cpu->PrgCounter, Cpu);
        ++Cpu->PrgCounter;
        
#if CPU_LOG
        sprintf(Cpu->LogData2, "%02X", Cpu->OpHighByte);
#endif
    }
    else if(Cpu->Cycle == 4)
    {
        Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        pollInterrupts(Cpu);
    }
    else if(Cpu->Cycle == 5)
    {        
        u8 IndLowByte = (u8)(Cpu->OpLowByte + 1);
        Cpu->OpHighByte = readCpu8((Cpu->OpHighByte << 8) | IndLowByte, Cpu);

        Cpu->OpLowByte = Cpu->OpValue;

        operations[Cpu->OpCode](0, Cpu);
        Cpu->NextCycle = 1;
    }   
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
