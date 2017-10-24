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
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts();
    }
    else if(Cpu->Cycle == 2)
    {
        clearCarry(&Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
uint8 cld(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts();
    }
    else if(Cpu->Cycle == 2)
    {
        clearDecimal(&Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
uint8 cli(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts();
    }
    else if(Cpu->Cycle == 2)
    {
        clearInterrupt(&Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
uint8 clv(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts();
    }
    else if(Cpu->Cycle == 2)
    {
        clearOverflow(&Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
uint8 dex(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts();
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
uint8 dey(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts();
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
uint8 inx(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts();
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
uint8 iny(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts();
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
uint8 sec(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts();
    }
    else if(Cpu->Cycle == 2)
    {
        setCarry(&Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
uint8 sed(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts();
    }
    else if(Cpu->Cycle == 2)
    {
        setDecimal(&Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
uint8 sei(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts();
    }
    else if(Cpu->Cycle == 2)
    {
        setInterrupt(&Cpu->Flags);
        Cpu->NextCycle = 1;
    }
    return(0);
}
uint8 tax(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts();
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
uint8 tay(uint8 Value, cpu *Cpu)
{   
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts();
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
uint8 tsx(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts();
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
uint8 txa(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts();
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
uint8 tya(uint8 Value, cpu *Cpu)
{   
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts();
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
uint8 txs(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts();
    }
    else if(Cpu->Cycle == 2)
    {
        Cpu->StackPtr = Cpu->X;
        Cpu->NextCycle = 1;
    }
    return(0);
}


uint8 nmi(uint8 Value, cpu *Cpu)
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

uint8 brk(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1 || Cpu->Cycle == 2 )
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 3)
    {
        uint8 HighByte = (uint8)(Cpu->PrgCounter >> 8);
        writeStack(HighByte, Cpu);
        decrementStack(Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 LowByte = (uint8)Cpu->PrgCounter; 
        writeStack(LowByte, Cpu);
        decrementStack(Cpu);
        pollInterrupts();
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

uint8 rti(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
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
        pollInterrupts();
    }
    else if(Cpu->Cycle == 6)
    {
        Cpu->PrgCounter = (readStack(Cpu) << 8) | (Cpu->PrgCounter & 0x00FF);
        Cpu->NextCycle = 1;
    }
    return(0);
}

uint8 rts(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
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
        pollInterrupts();
    }
    else if(Cpu->Cycle == 6)
    {
        ++Cpu->PrgCounter;
        Cpu->NextCycle = 1;
    }
    
    return(0);
}

uint8 pha(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 2)
    {
        pollInterrupts();
    }
    else if(Cpu->Cycle == 3)
    {
        writeStack(Cpu->A, Cpu);
        decrementStack(Cpu);
        Cpu->NextCycle = 1;
    }
    return(0);
}

uint8 php(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 2)
    {
        pollInterrupts();
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

uint8 pla(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 3)
    {
        incrementStack(Cpu);
        pollInterrupts();
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

uint8 plp(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 3)
    {
        incrementStack(Cpu);
        pollInterrupts();
    }
    else if(Cpu->Cycle == 4)
    {
        Cpu->Flags = readStack(Cpu);
        Cpu->NextCycle = 1;
    }
    
    return(0);
}

uint8 jsr(uint8 Value, cpu *Cpu)
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

    setZero(Sum, &Cpu->Flags);
    setNegative(Sum, &Cpu->Flags);

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

    setZero(Sum, &Cpu->Flags);
    setNegative(Sum, &Cpu->Flags);

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

uint8 nop(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
    }
    else if(Cpu->Cycle == 2)
    {
        Cpu->NextCycle = 1;
    }
    
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
uint8 bcc(uint8 Value, cpu *Cpu)
{
    return(!isBitSet(CARRY_BIT, Cpu->Flags));
}
uint8 bcs(uint8 Value, cpu *Cpu)
{
    return(isBitSet(CARRY_BIT, Cpu->Flags));    
}
uint8 beq(uint8 Value, cpu *Cpu)
{
    uint8 temp = ZERO_BIT;
    return(isBitSet(temp, Cpu->Flags));
}
uint8 bmi(uint8 Value, cpu *Cpu)
{
    return(isBitSet(NEGATIVE_BIT, Cpu->Flags));    
}
uint8 bne(uint8 Value, cpu *Cpu)
{
    return(!isBitSet(ZERO_BIT, Cpu->Flags));    
}
uint8 bpl(uint8 Value, cpu *Cpu)
{
    return(!isBitSet(NEGATIVE_BIT, Cpu->Flags));    
}
uint8 bvc(uint8 Value, cpu *Cpu)
{
    return(!isBitSet(OVERFLOW_BIT, Cpu->Flags));    
}
uint8 bvs(uint8 Value, cpu *Cpu)
{
    return(isBitSet(OVERFLOW_BIT, Cpu->Flags));    
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
    //Assert(0);
    return(0);
}
uint8 las(uint8 Value, cpu *Cpu)
{
    return(0); 
}
uint8 shx(uint8 Value, cpu *Cpu)
{
    return(0);
}

uint8 shy(uint8 Value, cpu *Cpu)
{
//    if((Cpu->X + Value) <= 0xFF)
    //writeCpu8(Byte, Address, Cpu); // TODO: This shouldn't happen, need refactoring
    return(0);
}
uint8 tas(uint8 Value, cpu *Cpu)
{
    ///  Assert(0);
    return(0);
}

uint8 xaa(uint8 Value, cpu *Cpu)
{
//    Assert(0);
    return(0);
}


uint8 (*instrOps[INSTRUCTION_COUNT])(uint8 InByte, cpu *Cpu) =
{
    /*         0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F        */
    /*0*/    brk,ora,nmi,slo,nop,ora,asl,slo,php,ora,asl,anc,nop,ora,asl,slo,
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


void implied(cpu *Cpu)
{
    // NOTE: Implied instructions handle how many cycles they take
    instrOps[Cpu->OpInstruction](0, Cpu);
}

//////////////////////////////////////
//// ACCUMULATOR AND IMPLIED MODE ////
//////////////////////////////////////

void accumulator(cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts();
    }
    else if(Cpu->Cycle == 2)
    {
        Cpu->A = instrOps[Cpu->OpInstruction](Cpu->A, Cpu);
        Cpu->NextCycle = 1;
    }
}

////////////////////////
//// IMMEDIATE MODE ////
////////////////////////

void immediate(cpu *Cpu)
{
    if(Cpu->Cycle == 1)
    {
        ++Cpu->PrgCounter;
        pollInterrupts();
    }
    else if(Cpu->Cycle == 2)
    {
        uint8 Value = readCpu8(Cpu->PrgCounter++, Cpu);
        instrOps[Cpu->OpInstruction](Value, Cpu);
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
        //pollInterrupts();
    }
    if(Cpu->Cycle == 2)
    {
        Cpu->OpValue = readCpu8(Cpu->PrgCounter, Cpu);
        ++Cpu->PrgCounter;
        pollInterrupts();
#if CPU_LOG
        sprintf(Cpu->LogData1, "%02X", Cpu->OpValue);
#endif
    }
    else if(Cpu->Cycle == 3)
    {
        // The Correct Program Counter. Will be saved into OpHigh and Low Bytes
        uint16 TempPC = Cpu->PrgCounter + (int8)Cpu->OpValue;
        Cpu->OpHighByte = (TempPC & 0xFF00) >> 8;
        Cpu->OpLowByte = TempPC & 0x00FF;

        // The potentiall wrong high byte after branch
        uint8 UnFixedHighByte = (Cpu->PrgCounter & 0xFF00) >> 8;

#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%04X", TempPC);
#endif
        
        uint8 Branch = instrOps[Cpu->OpInstruction](0, Cpu);
        
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

            pollInterrupts();
        }
        else
        {
            Cpu->Branched = true; // NOTE: technically haven't 'branched' here. But still does pipelining
        }
    }
    else if(Cpu->Cycle == 4)
    {
        if(Cpu->OpTemp) // If the page was crossed then fix.
        {
            Cpu->PrgCounter = (Cpu->OpHighByte << 8) | (Cpu->PrgCounter & 0xFF);
            pollInterrupts();
        }
        else
        {
            Cpu->Branched = true;
        }
    }
    else if(Cpu->Cycle == 5)
    {
        Cpu->Branched = true;
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
        pollInterrupts();
    }
    else if(Cpu->Cycle == 3)
    {
        uint8 Value = readCpu8(Cpu->OpLowByte, Cpu);
        instrOps[Cpu->OpInstruction](Value, Cpu);
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
        Cpu->OpValue = instrOps[Cpu->OpInstruction](Cpu->OpValue, Cpu);
        pollInterrupts();
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
        pollInterrupts();
    }
    else if(Cpu->Cycle == 3)
    {
#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%04X = #$%02X", Cpu->OpLowByte, readCpu8(Cpu->OpLowByte, Cpu));
#endif
        
        uint8 Value = instrOps[Cpu->OpInstruction](0, Cpu);
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
        pollInterrupts();
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 Value = readCpu8(Cpu->OpLowByte, Cpu);
        instrOps[Cpu->OpInstruction](Value, Cpu);
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
        Cpu->OpValue = instrOps[Cpu->OpInstruction](Cpu->OpValue, Cpu);
        pollInterrupts();
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
        pollInterrupts();
    }
    else if(Cpu->Cycle == 4)
    {  
#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%02X,X @ $%04X = #$%02X", Cpu->OpLowByte - Cpu->X, Cpu->OpLowByte, readCpu8(Cpu->OpLowByte, Cpu));
#endif

        uint8 Value = instrOps[Cpu->OpInstruction](0, Cpu);
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
        pollInterrupts();
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 Value = readCpu8(Cpu->OpLowByte, Cpu);
        instrOps[Cpu->OpInstruction](Value, Cpu);
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
        Cpu->OpValue = instrOps[Cpu->OpInstruction](Cpu->OpValue, Cpu);
        pollInterrupts();
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
        pollInterrupts();
    }
    else if(Cpu->Cycle == 4)
    {      
#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%02X,Y @ $%04X = #$%02X", Cpu->OpLowByte - Cpu->X, Cpu->OpLowByte, readCpu8(Cpu->OpLowByte, Cpu));
#endif

        uint8 Value = instrOps[Cpu->OpInstruction](0, Cpu);
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
        pollInterrupts();
    }
    if(Cpu->Cycle == 3)
    {   
#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%04X", (Cpu->OpHighByte << 8) | Cpu->OpLowByte);
#endif
        
        instrOps[Cpu->OpInstruction](0, Cpu);
        Cpu->NextCycle = 1;
    }
}

void absRead(cpu *Cpu)
{
    absCommon(Cpu);

    if(Cpu->Cycle == 3)
    {
        pollInterrupts();
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 Value = readCpu8(((Cpu->OpHighByte << 8) | Cpu->OpLowByte), Cpu);
        instrOps[Cpu->OpInstruction](Value, Cpu);
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
        Cpu->OpValue = instrOps[Cpu->OpInstruction](Cpu->OpValue, Cpu);
        pollInterrupts();
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
        pollInterrupts();
    }
    else if(Cpu->Cycle == 4)
    {
#if CPU_LOG
        sprintf(Cpu->LogExtraInfo, " $%04X = #$%02X", (Cpu->OpHighByte << 8) | Cpu->OpLowByte, readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu));
#endif

        uint8 Value = instrOps[Cpu->OpInstruction](0, Cpu);
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
        pollInterrupts();
    }
    else if(Cpu->Cycle == 4)
    {        
        uint8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
   
        if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
        {
            ++Cpu->OpHighByte;
            pollInterrupts();
        }
        else // else read was fine, execute instruction, end op
        {
            instrOps[Cpu->OpInstruction](Value, Cpu);
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
        uint8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        instrOps[Cpu->OpInstruction](Value, Cpu);
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
        Cpu->OpValue = instrOps[Cpu->OpInstruction](Cpu->OpValue, Cpu);
        pollInterrupts();
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

        
        pollInterrupts();
    }
    else if(Cpu->Cycle == 5)
    {        
        uint8 Value = instrOps[Cpu->OpInstruction](0, Cpu);
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
        pollInterrupts();
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        
        if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
        {
            ++Cpu->OpHighByte;
            pollInterrupts();
        }
        else // else read was fine, execute instruction, end op
        {
            instrOps[Cpu->OpInstruction](Value, Cpu);
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
        uint8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        instrOps[Cpu->OpInstruction](Value, Cpu);
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
        Cpu->OpValue = instrOps[Cpu->OpInstruction](Cpu->OpValue, Cpu);
        pollInterrupts();
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

        pollInterrupts();
    }
    else if(Cpu->Cycle == 5)
    {
        uint8 Value = instrOps[Cpu->OpInstruction](0, Cpu);
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
        uint8 Temp = (Cpu->OpValue+1) & 0xFF;
        Cpu->OpHighByte = readCpu8(Temp, Cpu);
    }
}

void idxXRead(cpu *Cpu)
{
    idxXCommon(Cpu);

    if(Cpu->Cycle == 5)
    {
        pollInterrupts();
    }
    else if(Cpu->Cycle == 6)
    {
        uint8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        instrOps[Cpu->OpInstruction](Value, Cpu);
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
        Cpu->OpValue = instrOps[Cpu->OpInstruction](Cpu->OpValue, Cpu);
        pollInterrupts();
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
        pollInterrupts();
    }
    else if(Cpu->Cycle == 6)
    {
        uint8 Value = instrOps[Cpu->OpInstruction](0, Cpu);
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
        uint8 Temp = (Cpu->OpValue + 1) & 0xFF;
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
        pollInterrupts();
    }
    else if(Cpu->Cycle == 5)
    {
        uint8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        
        if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
        {
            ++Cpu->OpHighByte;
            pollInterrupts();
        }
        else
        {
#if CPU_LOG
            sprintf(Cpu->LogExtraInfo, " ($%s),Y @ $%04X = #$%02X",
                    Cpu->LogData1, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Value);
#endif
            instrOps[Cpu->OpInstruction](Value, Cpu);
            Cpu->NextCycle = 1;
        }
    }
    else if(Cpu->Cycle == 6)
    {
        uint8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        instrOps[Cpu->OpInstruction](Value, Cpu);
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
        Cpu->OpValue = instrOps[Cpu->OpInstruction](Cpu->OpValue, Cpu);
        pollInterrupts();
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
                
        pollInterrupts();
    }
    else if(Cpu->Cycle == 6)
    {
        uint8 Value = instrOps[Cpu->OpInstruction](0, Cpu);
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
        pollInterrupts();
    }
    else if(Cpu->Cycle == 5)
    {        
        uint8 IndLowByte = (uint8)(Cpu->OpLowByte + 1);
        Cpu->OpHighByte = readCpu8((Cpu->OpHighByte << 8) | IndLowByte, Cpu);


        Cpu->OpLowByte = Cpu->OpValue;

        instrOps[Cpu->OpInstruction](0, Cpu);
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
