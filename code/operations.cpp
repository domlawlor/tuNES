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
        setBreak(&Cpu->Flags);
        writeStack(Cpu->Flags, Cpu);
        decrementStack(Cpu);
    }
    else if(Cpu->Cycle == 6)
    {
        setInterrupt(&Cpu->Flags);
        Cpu->PrgCounter = (Cpu->PrgCounter & 0xFF00) | readCpu8(IRQ_BRK_VEC, Cpu);
    }
    else if(Cpu->Cycle == 7)
    {
        Cpu->PrgCounter = (Cpu->PrgCounter & 0x00FF) | readCpu8(IRQ_BRK_VEC + 1, Cpu) << 8;
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
        writeStack(Cpu->Flags, Cpu);
        decrementStack(Cpu);
        Cpu->NextCycle = 1;
    }
//    setBreak(&Cpu->Flags);
//    setBlank(&Cpu->Flags);
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
    return(0);
}
uint8 AND(uint8 Value, cpu *Cpu)
{
    Cpu->A = Cpu->A & Value;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(0);
}

uint8 ora(uint8 Value, cpu *Cpu)
{
    Cpu->A = Cpu->A | Value;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(0);
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
    // TODO: Unsure on exact timing
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
    Value = adc(Value, Cpu);
    return(Value);
}

uint8 isc(uint8 Value, cpu *Cpu)
{
    Value = inc(Value, Cpu);
    Value = sbc(Value, Cpu);
    return(Value);
}

uint8 dcp(uint8 Value, cpu *Cpu)
{
    Value = dec(Value, Cpu);
    Value = cmp(Value, Cpu);
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
    return(isBitSet(ZERO_BIT, Cpu->Flags));
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

uint8 asl_acm(uint8 Value, cpu *Cpu)
{
    uint8 Byte = 0;

    if(Cpu->A & (1 << 7))
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
    Byte = Cpu->A << 1;
    Cpu->A = Byte;

    setZero(Byte, &Cpu->Flags);
    setNegative(Byte, &Cpu->Flags);

    return(0);
}

uint8 lsr_acm(uint8 Value, cpu *Cpu)
{
    if(Cpu->A & 1)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
    Cpu->A = Cpu->A >> 1;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);

    return(0);
}

uint8 rol_acm(uint8 Value, cpu *Cpu)
{
    uint8 CarrySet = isBitSet(CARRY_BIT, Cpu->Flags);

    if(Cpu->A & (1 << 7))
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
        
    uint8 Byte = Cpu->A << 1;
        
    if(CarrySet)
        Byte = Byte | 1;
        
    Cpu->A = Byte;

    setZero(Byte, &Cpu->Flags);
    setNegative(Byte, &Cpu->Flags);

    return(0);
}

uint8 ror_acm(uint8 Value, cpu *Cpu)
{
    uint8 CarrySet = isBitSet(CARRY_BIT, Cpu->Flags);
    
    if(Cpu->A & 1)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
        
    uint8 Byte = Cpu->A >> 1;
        
    if(CarrySet)
        Byte = Byte | (1 << 7);
        
    Cpu->A = Byte;

    setZero(Byte, &Cpu->Flags);
    setNegative(Byte, &Cpu->Flags);

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
    AND(Value, Cpu);
    lsr(Value, Cpu);
    return(0);
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
        instrOps[Cpu->OpInstruction](0, Cpu);
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
        pollInterrupts();
    }
    if(Cpu->Cycle == 2)
    {
        Cpu->OpValue = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        uint8 NextOp = readCpu8(Cpu->PrgCounter, Cpu);

        uint8 Branch = instrOps[Cpu->OpInstruction](0, Cpu);
        if(Branch)
        {
            Cpu->OpLowByte = Cpu->OpTemp = (Cpu->PrgCounter & 0x00FF);
            Cpu->OpHighByte = (Cpu->PrgCounter & 0xFF00) >> 8;
            Cpu->OpLowByte += Cpu->OpValue;
        }
        else
        {
            Cpu->OpInstruction = NextOp;
            ++Cpu->PrgCounter;
            Cpu->NextCycle = 2;
        }
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 NextOp = readCpu8(Cpu->PrgCounter, Cpu);

        if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
        {
            ++Cpu->OpHighByte;
            pollInterrupts();
        }
        else
        {
            Cpu->OpInstruction = NextOp;
            ++Cpu->PrgCounter;
            Cpu->NextCycle = 2;
        }
    }
    else if(Cpu->Cycle == 5)
    {
        Cpu->OpInstruction = readCpu8(Cpu->PrgCounter++, Cpu);
        Cpu->NextCycle = 2;
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
        Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu);
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
        instrOps[Cpu->OpInstruction](Value, Cpu);// Do Op pass in value
        Cpu->NextCycle = 1;
    }   
}

void zeroXIndexReadWrite(cpu *Cpu)
{
    zeroXCommon(Cpu);

    if(Cpu->Cycle == 4)
    {
        Cpu->OpValue = readCpu8(Cpu->OpLowByte, Cpu);
    }
    else if(Cpu->Cycle == 5)
    {
        Cpu->OpValue = instrOps[Cpu->OpInstruction](Cpu->OpValue, Cpu);// Do op on value and return
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
    }
    else if(Cpu->Cycle == 3)
    {
        Cpu->OpLowByte += Cpu->Y;        
    }
}

void zeroYIndexRead(cpu *Cpu)
{
    if(Cpu->Cycle == 3)
    {
        pollInterrupts();
    }
    else if(Cpu->Cycle == 4)
    {
        uint8 Value = readCpu8(Cpu->OpLowByte, Cpu);
        instrOps[Cpu->OpInstruction](Value, Cpu);
        Cpu->NextCycle = 1;
    }   
}

void zeroYIndexReadWrite(cpu *Cpu)
{
    zeroYCommon(Cpu);
    
    if(Cpu->Cycle == 4)
    {
        Cpu->OpValue = readCpu8(Cpu->OpLowByte, Cpu);
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
    }
    else if(Cpu->Cycle == 3)
    {
        Cpu->OpHighByte = readCpu8(Cpu->PrgCounter++, Cpu);
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
    }
}

void absReadWrite(cpu *Cpu)
{
    absCommon(Cpu);
    
    if(Cpu->Cycle == 4)
    {
        Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
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
        uint8 Byte = instrOps[Cpu->OpInstruction](0, Cpu);
        writeCpu8(Byte, (Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
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
    }
    else if(Cpu->Cycle == 3)
    {
        Cpu->OpHighByte = readCpu8(Cpu->PrgCounter++, Cpu);
        Cpu->OpTemp = Cpu->OpLowByte;
        Cpu->OpLowByte += Cpu->X;
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
        }
    }
    else if(Cpu->Cycle == 5)
    {
        // NOTE: Only reaches here if the page was crossed
        uint8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        instrOps[Cpu->OpInstruction](Value, Cpu);
        Cpu->NextCycle = 1;
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
        Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        Cpu->OpHighByte = readCpu8(Cpu->PrgCounter++, Cpu);
        Cpu->OpTemp = Cpu->OpLowByte;
        Cpu->OpLowByte += Cpu->Y;
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
        }
    }
    else if(Cpu->Cycle == 5)
    {
        // NOTE: Only reaches here if the page was crossed
        uint8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        instrOps[Cpu->OpInstruction](Value, Cpu);
        Cpu->NextCycle = 1;
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
    absXIndexCommon(Cpu);
    
    if(Cpu->Cycle == 4)
    {
        Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);

        if(Cpu->OpLowByte < Cpu->OpTemp) // If the page was crossed then fix.
        {
            ++Cpu->OpHighByte; 
        }

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
        Cpu->OpValue = readCpu8(Cpu->PrgCounter++, Cpu);
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
        Cpu->OpHighByte = readCpu8(Cpu->OpValue+1, Cpu);
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
        Cpu->OpValue = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        Cpu->OpLowByte = readCpu8(Cpu->OpValue, Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        Cpu->OpHighByte = readCpu8(Cpu->OpValue+1, Cpu);
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
            instrOps[Cpu->OpInstruction](Value, Cpu);
            Cpu->NextCycle = 1;
        }
    }
    else if(Cpu->Cycle == 6)
    {
        // Will not enter if boundary not crossed
        uint8 Value = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        Cpu->NextCycle = 1;
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
        Cpu->OpLowByte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        Cpu->OpHighByte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 4)
    {
        Cpu->OpValue = readCpu8((Cpu->OpHighByte << 8) | Cpu->OpLowByte, Cpu);
        // TODO: POLL INTERRUPT???
    }
    else if(Cpu->Cycle == 5)
    {
        uint8 TempHighByte = readCpu8((Cpu->OpHighByte << 8) | (Cpu->OpLowByte+1), Cpu);
        Cpu->OpLowByte = Cpu->OpValue;
        Cpu->OpHighByte = TempHighByte;
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
