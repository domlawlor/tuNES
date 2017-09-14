/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#include "cpu.h"

//////////////////////
// Stack Operations //
//////////////////////

uint8 brk(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
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
    if(Cpu->Cycle == 2)
    {
        // Nothing
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
        Cpu->PrgCounter = Cpu->PrgCounter & 0xFF00 | readStack(Cpu);
        incrementStack(Cpu);
    }
    else if(Cpu->Cycle == 6)
    {
        Cpu->PrgCounter = Cpu->PrgCounter & 0x00FF | readStack(Cpu) << 8;
        Cpu->NextCycle = 1;
    }
    return(0);
}

uint8 rts(uint8 Value, cpu *Cpu)
{
    if(Cpu->Cycle == 2)
    {
        // Nothing
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
        Cpu->PrgCounter = Cpu->PrgCounter & 0x00FF | readStack(Cpu) << 8;
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
    if(Cpu->Cycle == 2)
    {
// Nothing
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
    if(Cpu->Cycle == 2)
    {
        // NOTHING
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
    if(Cpu->Cycle == 2)
    {
        // Nothing
    }
    else if(Cpu->Cycle == 3)
    {
        incrementStack(Cpu);
    }
    else if(Cpu->Cycle == 3)
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
    if(Cpu->Cycle == 2)
    {
        // Nothing
    }
    else if(Cpu->Cycle == 3)
    {
        incrementStack(Cpu);
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
    if(Cpu->Cycle == 2)
    {
        uint8 LowByte = readCpu8(Cpu->PrgCounter++, Cpu);
    }
    else if(Cpu->Cycle == 3)
    {
        // NOTHING??
    }
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
        Cpu->PrgCounter = (Cpu->PrgCounter & 0xFF00) | LowByte;
        Cpu->PrgCounter = readCpu8(Cpu->PrgCounter, Cpu) << 8 | (Cpu->PrgCounter & 0x00FF);
        Cpu->NextCycle = 1;
    }

    return(0);
}

/////////////////////////
// Absolute Operations //
/////////////////////////

uint8 jmp(uint8 Value, cpu *Cpu)
{
// Cycle 2
    uint8 LowByte = readCpu8(Cpu->PrgCounter++, Cpu);
// Cycle 3
    Cpu->PrgCounter = (Cpu->PrgCounter & 0xFF00) | LowByte;
    Cpu->PrgCounter = readCpu8(Cpu->PrgCounter, Cpu) << 8 | (Cpu->PrgCounter & 0x00FF);

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
}

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


// Implied

uint8 clc(uint8 Value, cpu *Cpu)
{
    clearCarry(&Cpu->Flags);
    return(0);
}
uint8 cld(uint8 Value, cpu *Cpu)
{
    clearDecimal(&Cpu->Flags);
    return(0);
}
uint8 cli(uint8 Value, cpu *Cpu)
{
    clearInterrupt(&Cpu->Flags);
    return(0);
}
uint8 clv(uint8 Value, cpu *Cpu)
{
    clearOverflow(&Cpu->Flags);
    return(0);
}
uint8 dex(uint8 Value, cpu *Cpu)
{
    --Cpu->X;
    setZero(Cpu->X, &Cpu->Flags);
    setNegative(Cpu->X, &Cpu->Flags);
    return(0);
}
uint8 dey(uint8 Value, cpu *Cpu)
{
    --Cpu->Y;
    setZero(Cpu->Y, &Cpu->Flags);
    setNegative(Cpu->Y, &Cpu->Flags);
    return(0);
}
uint8 inx(uint8 Value, cpu *Cpu)
{
    ++Cpu->X;
    setZero(Cpu->X, &Cpu->Flags);
    setNegative(Cpu->X, &Cpu->Flags);
    return(0);
}
uint8 iny(uint8 Value, cpu *Cpu)
{
    ++Cpu->Y;
    setZero(Cpu->Y, &Cpu->Flags);
    setNegative(Cpu->Y, &Cpu->Flags);
    return(0);
}
uint8 sec(uint8 Value, cpu *Cpu)
{
    setCarry(&Cpu->Flags);
    return(0);
}
uint8 sed(uint8 Value, cpu *Cpu)
{
    setDecimal(&Cpu->Flags);
    return(0);
}
uint8 sei(uint8 Value, cpu *Cpu)
{
    setInterrupt(&Cpu->Flags);
    return(0);
}
uint8 tax(uint8 Value, cpu *Cpu)
{
    Cpu->X = Cpu->A;
    setZero(Cpu->X, &Cpu->Flags);
    setNegative(Cpu->X, &Cpu->Flags);
    return(0);
}
uint8 tay(uint8 Value, cpu *Cpu)
{    
    Cpu->Y = Cpu->A;
    setZero(Cpu->Y, &Cpu->Flags);
    setNegative(Cpu->Y, &Cpu->Flags);
    return(0);
}
uint8 tsx(uint8 Value, cpu *Cpu)
{
    Cpu->X = Cpu->StackPtr;
    setZero(Cpu->X, &Cpu->Flags);
    setNegative(Cpu->X, &Cpu->Flags);
    return(0);
}
uint8 txa(uint8 Value, cpu *Cpu)
{
    Cpu->A = Cpu->X;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(0);
}
uint8 tya(uint8 Value, cpu *Cpu)
{   
    Cpu->A = Cpu->Y;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(0);
}
uint8 txs(uint8 Value, cpu *Cpu)
{
    Cpu->StackPtr = Cpu->X;
    return(0);
}

/////////



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

/*
// 9E: // shx $1234,y 
Array 
( 
[1005] => addr = PB(); 
[1020] => d = Y; 
[1050] => addr = u8(addr) + 256 * PB(); 
[1080] => RB(wrap(addr, addr+d)); 
[1262] => WB(wrap(addr, addr+d), X & ((addr+d) >> 8)); 
) 

// 9C: // shy $1234,x 
Array 
( 
[1005] => addr = PB(); 
[1010] => d = X; 
[1050] => addr = u8(addr) + 256 * PB(); 
[1080] => RB(wrap(addr, addr+d)); 
[1263] => WB(wrap(addr, addr+d), Y & ((addr+d) >> 8)); 
) 

Where PB() is equal to RB(PC++), 
RB(addr) reads a byte from given address, 
and WB(addr, value) writes a byte into the given address; 
and wrap(addr, addr2) is equal to (addr & 0xFF00) + (addr2 & 0xFF). 

This passes Blargg's test. Note that changing "X & ((addr+d) >> 8)" into "X & (((addr+d) >> 8) + 1)" or "X & ((addr) >> 8)" or into a combination thereof made no difference to the fact. 
*/

uint8 shy(uint8 Value, cpu *Cpu)
{
    if((Cpu->X + Value) <= 0xFF)
        //writeCpu8(Byte, Address, Cpu); // TODO: This shouldn't happen
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
