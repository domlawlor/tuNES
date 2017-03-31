/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

uint8 adc(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 A = Cpu->A;
    uint8 B = readCpu8(Address, Cpu->MemoryOffset);
    uint8 C = isBitSet(CARRY_BIT, Cpu->Flags);

    Cpu->A = A + B + C;

    uint16 CarryTest = (uint16)A + (uint16)B + (uint16)C;
    if(CarryTest > 0xFF)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);

    // Overflow check, taken from the web. One day find out how this works
    if(((A ^ B) & 0x80 == 0) && ((A ^ Cpu->A) & 0x80 != 0))
        setOverflow(&Cpu->Flags);
    else
        clearOverflow(&Cpu->Flags);

    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(0);
}

uint8 AND(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = readCpu8(Address, Cpu->MemoryOffset);
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
        Value = readCpu8(Address, Cpu->MemoryOffset);
        if(Value & (1 << 7))
            setCarry(&Cpu->Flags);
        else
            clearCarry(&Cpu->Flags);
        Value = Value << 1;
        writeCpu8(Value, Address, Cpu->MemoryOffset);
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
    uint8 Value = readCpu8(Address, Cpu->MemoryOffset);
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
    push(Cpu->Flags, Cpu);

    Cpu->PrgCounter = readCpu16(IRQ_BRK_VEC, Cpu->MemoryOffset);
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
    uint8 Value = readCpu8(Address, Cpu->MemoryOffset);

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
    uint8 Value = readCpu8(Address, Cpu->MemoryOffset);

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
    uint8 Value = readCpu8(Address, Cpu->MemoryOffset);

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
    uint8 Value = readCpu8(Address, Cpu->MemoryOffset) - 1;
    writeCpu8(Value, Address, Cpu->MemoryOffset);
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
    uint8 Value = readCpu8(Address, Cpu->MemoryOffset);
    Cpu->A = Cpu->A ^ Value;
    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
    return(0);
}
uint8 inc(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = readCpu8(Address, Cpu->MemoryOffset) + 1;
    writeCpu8(Value, Address, Cpu->MemoryOffset);
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
    uint8 Value = readCpu8(Address, Cpu->MemoryOffset);
    Cpu->A = Value;
    setZero(Value, &Cpu->Flags);
    setNegative(Value, &Cpu->Flags);
    return(0);
}
uint8 ldx(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = readCpu8(Address, Cpu->MemoryOffset);
    Cpu->X = Value;
    setZero(Value, &Cpu->Flags);
    setNegative(Value, &Cpu->Flags);
    return(0);
}
uint8 ldy(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    uint8 Value = readCpu8(Address, Cpu->MemoryOffset);
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
        Value = readCpu8(Address, Cpu->MemoryOffset);
        if(Value & 1)
            setCarry(&Cpu->Flags);
        else
            clearCarry(&Cpu->Flags);
        Value = Value >> 1;
        writeCpu8(Value, Address, Cpu->MemoryOffset);
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
    uint8 Value = readCpu8(Address, Cpu->MemoryOffset);
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
            Value = Value & 1;
        
        Cpu->A = Value;
    }
    else
    {
        Value = readCpu8(Address, Cpu->MemoryOffset);
        if(Value & (1 << 7))
            setCarry(&Cpu->Flags);
        else
            clearCarry(&Cpu->Flags);
        
        Value = Value << 1;
        
        if(CarrySet)
            Value = Value & 1;
        
        writeCpu8(Value, Address, Cpu->MemoryOffset);
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
            Value = Value & (1 << 7);
        
        Cpu->A = Value;
    }
    else
    {
        Value = readCpu8(Address, Cpu->MemoryOffset);
        if(Value & 1)
            setCarry(&Cpu->Flags);
        else
            clearCarry(&Cpu->Flags);
        
        Value = Value >> 1;
        
        if(CarrySet)
            Value = Value & (1 << 7);
        
        writeCpu8(Value, Address, Cpu->MemoryOffset);
    }
    setZero(Value, &Cpu->Flags);
    setNegative(Value, &Cpu->Flags);
    return(0);
}
uint8 rti(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    // TODO: Double check if problems arise
    uint8 Flags = pop(Cpu);
    clearBreak(&Flags);
    setInterrupt(&Flags);
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
    // TODO: Revisit this to double check if it is correct
    uint8 A = Cpu->A;
    uint8 B = readCpu8(Address, Cpu->MemoryOffset);
    uint8 C = isBitSet(CARRY_BIT, Cpu->Flags);

    Cpu->A = A - B - (1-C);

    uint16 CarryTest = (int16)A - (int16)B - (int16)(1-C);
    if(CarryTest >= 0)
        setCarry(&Cpu->Flags);
    else
        clearCarry(&Cpu->Flags);

    // Overflow check, taken from the web. One day find out how this works
    if(((A ^ B) & 0x80 != 0) && ((A ^ Cpu->A) & 0x80 != 0))
        setOverflow(&Cpu->Flags);
    else
        clearOverflow(&Cpu->Flags);

    setZero(Cpu->A, &Cpu->Flags);
    setNegative(Cpu->A, &Cpu->Flags);
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
    writeCpu8(Cpu->A, Address, Cpu->MemoryOffset);
    return(0);
}
uint8 stx(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    writeCpu8(Cpu->X, Address, Cpu->MemoryOffset);
    return(0);
}
uint8 sty(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    writeCpu8(Cpu->Y, Address, Cpu->MemoryOffset);
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
    setZero(Cpu->StackPtr, &Cpu->Flags);
    setNegative(Cpu->StackPtr, &Cpu->Flags);
    return(0);
}



uint8 ahx(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Assert(0);
    return(0);
}
uint8 alr(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Assert(0);
    return(0);
}
uint8 anc(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Assert(0);
    return(0);
}
uint8 arr(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Assert(0);
    return(0);
}
uint8 axs(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Assert(0);
    return(0);
}
uint8 dcp(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Assert(0);
    return(0);
}
uint8 isc(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Assert(0);
    return(0);
}
uint8 kil(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Assert(0);
    return(0);
}
uint8 las(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    return(0); 
}
uint8 lax(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Assert(0);
    return(0);
}
uint8 rla(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    /*
    Rotate one bit left in memory, then AND accumulator with memory. Status
    flags: N,Z,C
    */
        
    return(0);
}
uint8 rra(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Assert(0);
    return(0);
}
uint8 sax(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Assert(0);
    return(0);
}
uint8 shx(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Assert(0);
    return(0);
}
uint8 shy(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Assert(0);
    return(0);
}
uint8 slo(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
//    Assert(0);
    return(0);
}
uint8 sre(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Assert(0);
    return(0);
}
uint8 tas(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Assert(0);
    return(0);
}

uint8 xaa(uint16 Address, cpu *Cpu, uint8 AddressMode)
{
    Assert(0);
    return(0);
}



