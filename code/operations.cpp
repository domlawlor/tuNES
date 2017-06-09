/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

uint8 adc(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 A = Cpu->A;
    uint8 B = readCpu8(Address, Cpu);
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

uint8 AND(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = readCpu8(Address, Cpu);
    Cpu->A = Cpu->A & Value;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(0);
}

uint8 asl(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = 0;
    if(AddressMode == ACM)
    {        
        if(Cpu->A & (1 << 7))
            setCarry(&Cpu->Flags);
        else
            clearCarry(&Cpu->Flags);
        Value = Cpu->A << 1;
        Cpu->A = Value;
    }
    else
    {
        Value = readCpu8(Address, Cpu);
        if(Value & (1 << 7))
            setCarry(&Cpu->Flags);
        else
            clearCarry(&Cpu->Flags);
        Value = Value << 1;
        writeCpu8(Value, Address, Cpu);
    }
    setZero(Value, &Cpu->Flags);
    setNegative(Value, &Cpu->Flags);
    return(0);
}

uint8 bcc(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 AddCycles = 0;
    
    if(!isBitSet(CARRY_BIT, Cpu->Flags))
    {
        ++AddCycles;
        if(crossedPageCheck(Cpu->PrgCounter, Address))
            ++AddCycles;
        Cpu->PrgCounter = Address;
    }
    else
        uint8 Test =0;
    return(AddCycles);
}

uint8 bcs(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 AddCycles = 0;
    if(isBitSet(CARRY_BIT, Cpu->Flags))
    {
        ++AddCycles;
        if(crossedPageCheck(Cpu->PrgCounter, Address))
            ++AddCycles;
        Cpu->PrgCounter = Address;
    }   
    return(AddCycles);    
}
uint8 beq(uint16 Address, cpu *Cpu, uint8 AddressMode)
{   
    uint8 AddCycles = 0;
    if(isBitSet(ZERO_BIT, Cpu->Flags))
    {
        ++AddCycles;
        if(crossedPageCheck(Cpu->PrgCounter, Address))
            ++AddCycles;
        Cpu->PrgCounter = Address;
    }   
    return(AddCycles);    
}

uint8 bit(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = readCpu8(Address, Cpu);
    if(Value & (1 << 6))
        setOverflow(&Cpu->Flags);
    else
        clearOverflow(&Cpu->Flags);
    setNegative(Value, &Cpu->Flags);
    setZero(Cpu->A & Value, &Cpu->Flags);
    return(0);
}

uint8 bmi(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 AddCycles = 0;
    if(isBitSet(NEGATIVE_BIT, Cpu->Flags))
    {
        ++AddCycles;
        if(crossedPageCheck(Cpu->PrgCounter, Address))
            ++AddCycles;
        Cpu->PrgCounter = Address;
    }   
    return(AddCycles);    
}

uint8 bne(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 AddCycles = 0;
    if(!isBitSet(ZERO_BIT, Cpu->Flags))
    {
        ++AddCycles;
        if(crossedPageCheck(Cpu->PrgCounter, Address))
            ++AddCycles;
        Cpu->PrgCounter = Address;
    }   
    return(AddCycles);    
}

uint8 bpl(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 AddCycles = 0;
    if(!isBitSet(NEGATIVE_BIT, Cpu->Flags))
    {
        ++AddCycles;
        if(crossedPageCheck(Cpu->PrgCounter, Address))
            ++AddCycles;
        Cpu->PrgCounter = Address;
    }   
    return(AddCycles);    
}

uint8 brk(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 HighByte = (uint8)(Cpu->PrgCounter >> 8);
    uint8 LowByte = (uint8)Cpu->PrgCounter; 
    push(HighByte, Cpu);
    push(LowByte, Cpu);

    setBlank(&Cpu->Flags);
    setBreak(&Cpu->Flags);
    push(Cpu->Flags, Cpu);
    setInterrupt(&Cpu->Flags);

    Cpu->PrgCounter = readCpu16(IRQ_BRK_VEC, Cpu);
    return(0);
}

uint8 bvc(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 AddCycles = 0;
    if(!isBitSet(OVERFLOW_BIT, Cpu->Flags))
    {
        ++AddCycles;
        if(crossedPageCheck(Cpu->PrgCounter, Address))
            ++AddCycles;
        Cpu->PrgCounter = Address;
    }   
    return(AddCycles);    
}

uint8 bvs(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 AddCycles = 0;
    if(isBitSet(OVERFLOW_BIT, Cpu->Flags))
    {
        ++AddCycles;
        if(crossedPageCheck(Cpu->PrgCounter, Address))
            ++AddCycles;
        Cpu->PrgCounter = Address;
    }   
    return(AddCycles);    
}

uint8 clc(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    clearCarry(&Cpu->Flags);
    return(0);
}
uint8 cld(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    clearDecimal(&Cpu->Flags);
    return(0);
}
uint8 cli(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    clearInterrupt(&Cpu->Flags);
    return(0);
}
uint8 clv(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    clearOverflow(&Cpu->Flags);
    return(0);
}
uint8 cmp(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = readCpu8(Address, Cpu);

    if(Cpu->A >= Value)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
    
    uint8 CmpValue = Cpu->A - Value;
    setZero(CmpValue, &Cpu->Flags);
    setNegative(CmpValue, &Cpu->Flags);
    return(0);
}
uint8 cpx(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = readCpu8(Address, Cpu);

    if(Cpu->X >= Value)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
    
    uint8 CmpValue = Cpu->X - Value;
    setZero(CmpValue, &Cpu->Flags);
    setNegative(CmpValue, &Cpu->Flags);
    return(0);
}
uint8 cpy(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = readCpu8(Address, Cpu);

    if(Cpu->Y >= Value)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
    
    uint8 CmpValue = Cpu->Y - Value;
    setZero(CmpValue, &Cpu->Flags);
    setNegative(CmpValue, &Cpu->Flags);
    return(0);
}
uint8 dec(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = readCpu8(Address, Cpu) - 1;
    writeCpu8(Value, Address, Cpu);
    setZero(Value, &Cpu->Flags);
    setNegative(Value, &Cpu->Flags);
    return(0);
}
uint8 dex(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    --Cpu->X;
    setZero(Cpu->X, &Cpu->Flags);
    setNegative(Cpu->X, &Cpu->Flags);
    return(0);
}
uint8 dey(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    --Cpu->Y;
    setZero(Cpu->Y, &Cpu->Flags);
    setNegative(Cpu->Y, &Cpu->Flags);
    return(0);
}
uint8 eor(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = readCpu8(Address, Cpu);
    Cpu->A = Cpu->A ^ Value;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(0);
}
uint8 inc(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = readCpu8(Address, Cpu) + 1;
    writeCpu8(Value, Address, Cpu);
    setZero(Value, &Cpu->Flags);
    setNegative(Value, &Cpu->Flags);
    return(0);
}
uint8 inx(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    ++Cpu->X;
    setZero(Cpu->X, &Cpu->Flags);
    setNegative(Cpu->X, &Cpu->Flags);
    return(0);
}
uint8 iny(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    ++Cpu->Y;
    setZero(Cpu->Y, &Cpu->Flags);
    setNegative(Cpu->Y, &Cpu->Flags);
    return(0);
}
uint8 jmp(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Cpu->PrgCounter = Address;
    return(0);
}
uint8 jsr(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint16 ReturnAddress = Cpu->PrgCounter - 1;
    uint8 HighByte = (uint8)(ReturnAddress >> 8);
    uint8 LowByte = (uint8)ReturnAddress; 

    push(HighByte, Cpu);
    push(LowByte, Cpu);

    Cpu->PrgCounter = Address;
    return(0);
}
uint8 lda(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = readCpu8(Address, Cpu);
    Cpu->A = Value;
    setZero(Value, &Cpu->Flags);
    setNegative(Value, &Cpu->Flags);
    return(0);
}
uint8 ldx(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = readCpu8(Address, Cpu);
    Cpu->X = Value;
    setZero(Value, &Cpu->Flags);
    setNegative(Value, &Cpu->Flags);
    return(0);
}
uint8 ldy(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = readCpu8(Address, Cpu);
    Cpu->Y = Value;
    setZero(Value, &Cpu->Flags);
    setNegative(Value, &Cpu->Flags);
    return(0);
}
uint8 lsr(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = 0;
    if(AddressMode == ACM)
    {        
        if(Cpu->A & 1)
            setCarry(&Cpu->Flags);
        else
            clearCarry(&Cpu->Flags);
        Cpu->A = Cpu->A >> 1;
        Value = Cpu->A;
    }
    else
    {
        Value = readCpu8(Address, Cpu);
        if(Value & 1)
            setCarry(&Cpu->Flags);
        else
            clearCarry(&Cpu->Flags);
        Value = Value >> 1;
        writeCpu8(Value, Address, Cpu);
    }
    setZero(Value, &Cpu->Flags);
    setNegative(Value, &Cpu->Flags);
    return(0);
}
uint8 nop(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    return(0);
}
uint8 ora(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = readCpu8(Address, Cpu);
    Cpu->A = Cpu->A | Value;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(0);
}
uint8 pha(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    push(Cpu->A, Cpu);
    return(0);
}
uint8 php(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    setBreak(&Cpu->Flags);
    setBlank(&Cpu->Flags);
    push(Cpu->Flags, Cpu);
    return(0);
}
uint8 pla(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Cpu->A = pop(Cpu); 
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(0);
}
uint8 plp(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Cpu->Flags = pop(Cpu);
    return(0);
}
uint8 rol(uint16 Address, cpu *Cpu, uint8 AddressMode)
{    
    uint8 Value = 0;
    uint8 CarrySet = isBitSet(CARRY_BIT, Cpu->Flags);
    
    if(AddressMode == ACM)
    {        
        if(Cpu->A & (1 << 7))
            setCarry(&Cpu->Flags);
        else
            clearCarry(&Cpu->Flags);
        
        Value = Cpu->A << 1;
        
        if(CarrySet)
            Value = Value | 1;
        
        Cpu->A = Value;
    }
    else
    {
        Value = readCpu8(Address, Cpu);
        if(Value & (1 << 7))
            setCarry(&Cpu->Flags);
        else
            clearCarry(&Cpu->Flags);
        
        Value = Value << 1;
        
        if(CarrySet)
            Value = Value | 1;
        
        writeCpu8(Value, Address, Cpu);
    }
    setZero(Value, &Cpu->Flags);
    setNegative(Value, &Cpu->Flags);
    return(0);
}

uint8 ror(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = 0;
    uint8 CarrySet = isBitSet(CARRY_BIT, Cpu->Flags);
    
    if(AddressMode == ACM)
    {        
        if(Cpu->A & 1)
            setCarry(&Cpu->Flags);
        else
            clearCarry(&Cpu->Flags);
        
        Value = Cpu->A >> 1;
        
        if(CarrySet)
            Value = Value | (1 << 7);
        
        Cpu->A = Value;
    }
    else
    {
        Value = readCpu8(Address, Cpu);
        if(Value & 1)
            setCarry(&Cpu->Flags);
        else
            clearCarry(&Cpu->Flags);
        
        Value = Value >> 1;
        
        if(CarrySet)
            Value = Value | (1 << 7);
        
        writeCpu8(Value, Address, Cpu);
    }
    setZero(Value, &Cpu->Flags);
    setNegative(Value, &Cpu->Flags);
    return(0);
}

uint8 rti(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Flags = pop(Cpu);
    Cpu->Flags = Flags;
    
    uint8 LowByte = pop(Cpu);
    uint8 HighByte = pop(Cpu);
    Cpu->PrgCounter = ((uint16)HighByte << 8) | (uint16)LowByte;
    return(0);
}
uint8 rts(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 LowByte = pop(Cpu);
    uint8 HighByte = pop(Cpu);
    uint16 ReturnAddress = ((uint16)HighByte << 8) | (uint16)LowByte;
    Cpu->PrgCounter = ReturnAddress + 1;
    return(0);
}
uint8 sbc(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 A = Cpu->A;
    uint8 B = ~readCpu8(Address, Cpu); // NOTE: Using the inverse
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
uint8 sec(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    setCarry(&Cpu->Flags);
    return(0);
}
uint8 sed(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    setDecimal(&Cpu->Flags);
    return(0);
}
uint8 sei(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    setInterrupt(&Cpu->Flags);
    return(0);
}
uint8 sta(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    writeCpu8(Cpu->A, Address, Cpu);
    return(0);
}
uint8 stx(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    writeCpu8(Cpu->X, Address, Cpu);
    return(0);
}
uint8 sty(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    writeCpu8(Cpu->Y, Address, Cpu);
    return(0);
}
uint8 tax(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Cpu->X = Cpu->A;
    setZero(Cpu->X, &Cpu->Flags);
    setNegative(Cpu->X, &Cpu->Flags);
    return(0);
}
uint8 tay(uint16 Address, cpu *Cpu, uint8 AddressMode)
{    
    Cpu->Y = Cpu->A;
    setZero(Cpu->Y, &Cpu->Flags);
    setNegative(Cpu->Y, &Cpu->Flags);
    return(0);
}
uint8 tsx(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Cpu->X = Cpu->StackPtr;
    setZero(Cpu->X, &Cpu->Flags);
    setNegative(Cpu->X, &Cpu->Flags);
    return(0);
}
uint8 txa(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Cpu->A = Cpu->X;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(0);
}
uint8 tya(uint16 Address, cpu *Cpu, uint8 AddressMode)
{   
    Cpu->A = Cpu->Y;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(0);
}
uint8 txs(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Cpu->StackPtr = Cpu->X;
    return(0);
}



uint8 ahx(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    //Assert(0);
    return(0);
}
uint8 alr(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    AND(Address, Cpu, AddressMode);
    lsr(Address, Cpu, ACM);
    return(0);
}
uint8 anc(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    AND(Address, Cpu, AddressMode);
    
    if(isBitSet(NEGATIVE_BIT, Cpu->Flags))
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);
    
    return(0);
}
uint8 arr(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    AND(Address, Cpu, AddressMode);    
    ror(Address, Cpu, ACM);

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
uint8 axs(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = readCpu8(Address, Cpu);

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
uint8 dcp(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    dec(Address, Cpu, AddressMode);
    cmp(Address, Cpu, AddressMode);
    return(0);
}
uint8 isc(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    inc(Address, Cpu, AddressMode);
    sbc(Address, Cpu, AddressMode);
    return(0);
}
uint8 kil(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    //Assert(0);
    return(0);
}
uint8 las(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    return(0); 
}
uint8 lax(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    lda(Address, Cpu, AddressMode);
    ldx(Address, Cpu, AddressMode);
    return(0);
}
uint8 rla(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    rol(Address, Cpu, AddressMode);
    AND(Address, Cpu, AddressMode);
    return(0);
}
uint8 rra(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    ror(Address, Cpu, AddressMode);
    adc(Address, Cpu, AddressMode);
    return(0);
}
uint8 sax(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = Cpu->A & Cpu->X;
    writeCpu8(Value, Address, Cpu);    
    return(0);
}
uint8 shx(uint16 Address, cpu *Cpu, uint8 AddressMode)
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

uint8 shy(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = (Cpu->Y & ((Address >> 8) + 1)) & 0xFF;
    uint8 ReadValue = readCpu8((Cpu->PrgCounter + 1), Cpu);
    
    if((Cpu->X + ReadValue) <= 0xFF)
        writeCpu8(Value, Address, Cpu);
    return(0);
}
uint8 slo(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    asl(Address, Cpu, AddressMode);
    ora(Address, Cpu, AddressMode);
    return(0);
}
uint8 sre(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    lsr(Address, Cpu, AddressMode);
    eor(Address, Cpu, AddressMode);
    return(0);
}
uint8 tas(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    ///  Assert(0);
    return(0);
}

uint8 xaa(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
//    Assert(0);
    return(0);
}



