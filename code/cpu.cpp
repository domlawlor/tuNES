/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#define NMI_VEC     0xFFFA
#define RESET_VEC   0xFFFC
#define IRQ_BRK_VEC 0xFFFE 

struct cpuRegisters
{
    uint8 A;
    uint8 X;
    uint8 Y; 
    uint8 Flags;
    uint8 StackPtr;
    uint16 PrgCounter;
};

struct cpu
{
    cpuRegisters Registers;
    uint64 MemoryOffset;
    uint8 LastTickCycles;
};

struct cpuLog
{
    uint16 ProgramLine;
    uint8 Instr;
    uint8 InstrValue1;
    uint8 InstrValue2;
    char *InstrName;
    uint16 InstrAddress;
    uint8 AddressValue;
    uint8 BytesRead;
};                                 

global cpuLog Log;

inline void setCarry(uint8 *Flags)
{
    *Flags = *Flags | 1;
}
inline void clearCarry(uint8 *Flags)
{
    *Flags = *Flags & ~1;
}
inline void setZero(uint8 Value, uint8 *Flags)
{
    if(Value == 0x00)
        *Flags = *Flags | (1 << 1); // Set zero flag
    else
        *Flags = *Flags & ~(1 << 1);
}
inline void setInterrupt(uint8 *Flags)
{
    *Flags = *Flags | (1 << 2);
}
inline void clearInterrupt(uint8 *Flags)
{
    *Flags = *Flags & ~(1 << 2);
}
inline void clearDecimal(uint8 *Flags)
{
    *Flags = *Flags & ~(1 << 3);
}
inline void setBreak(uint8 *Flags)
{
    *Flags = *Flags | (1 << 4);
}
inline void clearBreak(uint8 *Flags)
{
    *Flags = *Flags & ~(1 << 4);
}
inline void setOverflow(uint8 *Flags)
{
    *Flags = *Flags | (1 << 6);
}
inline void clearOverflow(uint8 *Flags)
{
    *Flags = *Flags & ~(1 << 6);
}
inline void setNegative(uint8 Value, uint8 *Flags)
{  
    if(Value >= 0x00 && Value <= 0x7F)
        *Flags = *Flags & ~(1 << 7); // clear negative flag
    else
        *Flags = *Flags | (1 << 7); // set negative flag
}


enum STATUS_BITS
{
    CARRY_BIT = 0,
    ZERO_BIT,
    INTERRUPT_BIT,
    DECIMAL_BIT,
    BREAK_BIT,
    BLANK_BIT,
    OVERFLOW_BIT,
    NEGATIVE_BIT
};

internal bool32 isBitSet(STATUS_BITS Bit, uint8 Flags)
{
    if(Bit == CARRY_BIT)
        return(Flags & 1);
    if(Bit == ZERO_BIT)
        return(Flags & (1 << 1));
    if(Bit == INTERRUPT_BIT)
        return(Flags & (1 << 2));
    if(Bit == DECIMAL_BIT)
        return(Flags & (1 << 3));
    if(Bit == BREAK_BIT)
        return(Flags & (1 << 4));
    if(Bit == BLANK_BIT)
        return(Flags & (1 << 5));
    if(Bit == OVERFLOW_BIT)
        return(Flags & (1 << 6));
    if(Bit == NEGATIVE_BIT)
        return(Flags & (1 << 7));

    Assert(0);
    return(0);
}


internal uint8 readCpuMemory8(uint16 Address, uint64 MemoryOffset)
{
    // NOTE: Mirrors the address for the 2kb ram 
    if(0x800 <= Address && Address < 0x2000)
        Address = (Address % 0x800);
    // NOTE: Mirror for PPU Registers
    if(0x2008 <= Address && Address < 0x4000)
        Address = (Address % (0x2008 - 0x2000)) + 0x2000;
    
    if(Address == 0x2007) // Reading from the IO of ppu. First read is junk, unless its the colour palette
    {
        // 0x0000 - 0x3EFF    First read will be junk, second read will return actual value
        // 0x3F00 - 0x3FFF    This read will always return straight away.
        uint8 temp= 1;
    }
        
    uint8 Value = readMemory8(Address, MemoryOffset);
            
    if(Address == 0x2002) // Read status will reset the IO registers
    {
        // Will reset 2005 and 2006 registers, and turn off bit 7 of 0x2002
        ResetScrollIOAdrs = true;
        ResetVRamIOAdrs = true;
        
        uint8 ResetValue = Value & ~(1 << 7);
        writeMemory8(ResetValue, Address, MemoryOffset);
    }
    
    return(Value);
}


internal uint16 readCpuMemory16(uint16 Address, uint64 MemoryOffset)
{
    // NOTE: Little Endian
    uint8 LowByte = readCpuMemory8(Address, MemoryOffset);
    uint8 HighByte = readCpuMemory8(Address+1, MemoryOffset);
        
    uint16 NewAddress = (HighByte << 8) | LowByte;
    return(NewAddress);
}


internal void writeCpuMemory8(uint8 Byte, uint16 Address, uint64 MemoryOffset)
{
    // NOTE: Mirrors the address for the 2kb ram 
    if(0x800 <= Address && Address < 0x2000)
        Address = (Address % 0x800);
    // NOTE: Mirror for PPU Registers
    if(0x2008 <= Address && Address < 0x4000)
        Address = (Address % (0x2008 - 0x2000)) + 0x2000;
    if(0x8000 < Address || Address == 0x2002)
        Assert(0); // Writing to Program ROM, bank switching?     

    
    writeMemory8(Byte, Address, MemoryOffset);
    
    if(Address == 0x2005) // Scroll address
    {
//        Assert(0);
    }
    
    if(Address == 0x2006) // Writing to ppu io address register
    {
        VRamIOAdrsCount++;
    }
    if(Address == 0x2007) // Write to IO for ppu. Happens after two writes to 0x2006
    {
        VRamIOWriteCount++; 
    }
}

void adc(uint8 Value, cpuRegisters* Registers)
{    
    uint8 CarryIn = (Registers->Flags & 1);            
    uint8 AddedValue = Registers->A + Value + CarryIn;

    bool32 ValueBit7 = Value & (1 << 7);
    bool32 RegABit7 = Registers->A & (1 << 7);
    bool32 ResultBit6 = AddedValue & (1 << 6);

    if((!ValueBit7 && !RegABit7 && ResultBit6) || (ValueBit7 && RegABit7 && !ResultBit6))
        setOverflow(&Registers->Flags);
    else
        clearOverflow(&Registers->Flags);

    if((!ValueBit7 && RegABit7 && ResultBit6)  || (ValueBit7 && !RegABit7 && ResultBit6) ||
       (ValueBit7 &&  RegABit7 && !ResultBit6) || (ValueBit7 &&  RegABit7 && ResultBit6))
        setCarry(&Registers->Flags);
    else
        clearCarry(&Registers->Flags);
    
    setNegative(AddedValue, &Registers->Flags);
    setZero(AddedValue, &Registers->Flags);
}
void sbc(uint8 Value, cpuRegisters* Registers)
{
    adc(~Value, Registers);
}
            
void cmp(uint8 Value, uint8 Register, uint8 *Flags)
{
    uint8 CmpValue = Register - Value;
                        
    setNegative(CmpValue, Flags);
    setZero(CmpValue, Flags);
                        
    if(Register < Value)
    {
        clearCarry(Flags);
    }
    else
        setCarry(Flags);
}

inline uint8 immediate(cpuRegisters *Registers, uint64 MemoryOffset)
{
    uint8 Value = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);

    Log.InstrValue1 = Value;
    Log.InstrAddress = Value;
    
    return(Value);
}
inline uint8 zeroPage(cpuRegisters *Registers, uint64 MemoryOffset)
{
    uint8 Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
    uint8 Value = readCpuMemory8(Address, MemoryOffset);

    Log.InstrAddress = Log.InstrValue1 = Address;
    Log.AddressValue = Value;
    
    return(Value);
}
inline uint8 zeroPageX(cpuRegisters *Registers, uint64 MemoryOffset)
{
    uint8 Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
    uint8 AddressBefore = Address;
    Address += Registers->X;
    uint8 Value = readCpuMemory8(Address, MemoryOffset);

    Log.InstrValue1 = AddressBefore;
    Log.InstrAddress = Address;
    Log.AddressValue = Value;
    
    return(Value);
}
inline uint8 zeroPageY(cpuRegisters *Registers, uint64 MemoryOffset)
{
    uint8 Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
    uint8 AddressBefore = Address;
    Address += Registers->Y;
    uint8 Value = readCpuMemory8(Address, MemoryOffset);

    Log.InstrValue1 = AddressBefore;
    Log.InstrAddress = Address;
    Log.AddressValue = Value;
    
    return(Value);
}
inline uint8 abs(cpuRegisters *Registers, uint64 MemoryOffset)
{
    uint16 Address = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset);
    uint8 Value = readCpuMemory8(Address, MemoryOffset);

    Log.InstrValue1 = Address;
    Log.InstrValue2 = Address >> 8;
    Log.InstrAddress = Address;
    Log.AddressValue = Value;
    
    return(Value);
}
inline uint8 absX(cpuRegisters *Registers, uint64 MemoryOffset)
{
    uint16 Address = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset);
    uint16 NewAdrs = Address + Registers->X;
    uint8 Value = readCpuMemory8(NewAdrs, MemoryOffset);

    
    Log.InstrValue1 = Address;
    Log.InstrValue2 = Address >> 8;
    Log.InstrAddress = NewAdrs;
    Log.AddressValue = Value;
    
    return(Value);
}
inline uint8 absY(cpuRegisters *Registers, uint64 MemoryOffset)
{
    uint16 Address = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset);
    uint16 NewAdrs = Address + Registers->Y;
    uint8 Value = readCpuMemory8(NewAdrs, MemoryOffset);

    
    Log.InstrValue1 = Address;
    Log.InstrValue2 = Address >> 8;
    Log.InstrAddress = NewAdrs;
    Log.AddressValue = Value;
    
    return(Value);
}
inline uint8 indirectX(cpuRegisters *Registers, uint64 MemoryOffset)
{
    uint8 ZeroAddress = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
    uint8 NewAddress = ZeroAddress + Registers->X;
    uint16 IndirectAddress = readCpuMemory16(NewAddress, MemoryOffset);
    uint8 Value = readCpuMemory8(IndirectAddress, MemoryOffset);

    Log.InstrValue1 = ZeroAddress;
    Log.InstrAddress = IndirectAddress;
    Log.AddressValue = Value;
    
    return(Value);
}
inline uint8 indirectY(cpuRegisters *Registers, uint64 MemoryOffset)
{
    uint8 ZeroAddress = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset); 
    uint16 IndirectAddress = readCpuMemory16(ZeroAddress, MemoryOffset);
    uint16 FinalAddress = IndirectAddress + Registers->Y;
    uint8 Value = readCpuMemory8(FinalAddress, MemoryOffset);
   
    Log.InstrValue1 = ZeroAddress;
    Log.InstrAddress = FinalAddress;
    Log.AddressValue = Value;
    
    return(Value);
}
inline int8 relative(cpuRegisters *Registers, uint64 MemoryOffset)
{
    int8 Value = (int8)readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);

    Log.InstrValue1 = Value;
    Log.InstrAddress = Value;
    
    return(Value);
}


#define STACK_ADRS 0x100

internal void pushStack(uint8 Byte, uint8 *StackPointer, uint64 MemoryOffset)
{
    uint8 *Address = (uint8 *)((*StackPointer + STACK_ADRS) + MemoryOffset);
    *Address = Byte;
    *StackPointer -= 1;  
}
internal uint8 popStack(uint8 *StackPointer, uint64 MemoryOffset)
{
    *StackPointer += 1;    
    uint8 *Address = (uint8 *)((*StackPointer + STACK_ADRS) + MemoryOffset);
    uint8 Value = *Address;
    *Address = 0;
    return(Value);
}




internal void cpuTick(cpu *CpuData)
{
    cpuRegisters *Registers = &CpuData->Registers;
    uint8 *CyclesElapsed = &CpuData->LastTickCycles;
    uint64 MemoryOffset = CpuData->MemoryOffset; 
    
    Log = {};
    Log.ProgramLine = Registers->PrgCounter;   
    
    uint8 BytesRead = 0;
    
    uint8 CurrentInstr = readMemory8(Registers->PrgCounter, MemoryOffset);                
    Log.Instr = CurrentInstr;

    static uint64 linecount = 0;
    uint64 WantedDebugLine = 330;
    if(linecount == WantedDebugLine)
        uint8 Break = 1;
    
    switch(CurrentInstr)
    {
        case 0x00: // BRK - Break
        {
            Assert(0); // NOTE: Very few games call break, if hit here, then likely to be a bug
            Registers->PrgCounter += 2;
            uint8 HighByte = (uint8)(Registers->PrgCounter >> 8);
            uint8 LowByte = (uint8)Registers->PrgCounter;

            pushStack(HighByte, &Registers->StackPtr, MemoryOffset);
            pushStack(LowByte, &Registers->StackPtr, MemoryOffset);

            setBreak(&Registers->Flags);
            setInterrupt(&Registers->Flags);
            pushStack(Registers->Flags, &Registers->StackPtr, MemoryOffset);
            
            Registers->PrgCounter = readCpuMemory16(IRQ_BRK_VEC, MemoryOffset);
           
            BytesRead = 0; // TODO: Check this
            *CyclesElapsed = 7;

            Log.InstrValue1 = (uint8) Registers->PrgCounter;
            Log.InstrValue2 = (uint8) (Registers->PrgCounter >> 8);
            Log.InstrName = "BRK";
            Log.InstrAddress = Registers->PrgCounter;
            Log.BytesRead = 3;
            break;
        }
        case 0x40: // RTI - Return from Interrupt
        {
            Registers->Flags = popStack(&Registers->StackPtr, MemoryOffset);
            clearInterrupt(&Registers->Flags);
            
            uint8 LowBytes = popStack(&Registers->StackPtr, MemoryOffset);
            uint8 HighBytes = popStack(&Registers->StackPtr, MemoryOffset);
            Registers->PrgCounter = (HighBytes << 8) | LowBytes;
            
            BytesRead = 0;
            *CyclesElapsed = 6;

            Log.BytesRead = 1;
            Log.InstrName = "RTI";
            break;
        }               
                    
        case 0x20: // JSR - Jump to subroutine
        {
            uint16 NewAddress = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset);
                        
            uint16 PrevAdrs = Registers->PrgCounter + 2; // Push the next opcode onto the stack                            
            uint8 HighByte = (uint8)(PrevAdrs >> 8);
            uint8 LowByte = (uint8)PrevAdrs;
      
            // Push onto stack, little endian
            pushStack(HighByte, &Registers->StackPtr, MemoryOffset);
            pushStack(LowByte, &Registers->StackPtr, MemoryOffset);

            Registers->PrgCounter = NewAddress;
            BytesRead = 0; // Moved to the next instruction already
            *CyclesElapsed = 6;

            
            Log.InstrValue1 = (uint8) NewAddress;
            Log.InstrValue2 = (uint8) (NewAddress >> 8);
            Log.InstrAddress = Registers->PrgCounter;
            Log.BytesRead = 3;
            Log.InstrName = "JSR";
            break;
        }
        case 0x60: // RTS - Return from subroutine
        {
            uint8 LowByte = popStack(&Registers->StackPtr, MemoryOffset);
            uint8 HighByte = popStack(&Registers->StackPtr, MemoryOffset);
            Registers->PrgCounter = ((uint16)HighByte << 8) | (uint16)LowByte;
            
            BytesRead = 1;
            *CyclesElapsed = 6;
            
            Log.InstrName = "RTS";
            break;
        }
                    
        case 0x4C: // JMP(Absolute) - Jump
        {
            uint16 NewAddress = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset);
            Registers->PrgCounter = NewAddress;
            BytesRead = 0;
            *CyclesElapsed = 3;

            Log.InstrValue1 = (uint8) NewAddress;
            Log.InstrValue2 = (uint8) (NewAddress >> 8);
            Log.InstrAddress = Registers->PrgCounter;
            Log.BytesRead = 3;
            Log.InstrName = "JMP";
            break;
        }
        case 0x6C: // JMP(Indirect)
        {
            uint16 IndirectAddress = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset);
            if((IndirectAddress & 0x00FF) == 0xFF)
                Assert(0); // TODO: If this ever hits then was on boundary of page. Fix how nes does this
            uint16 NewAddress = readCpuMemory16(IndirectAddress, MemoryOffset);
            
            Registers->PrgCounter = NewAddress;
            
            BytesRead = 0;
            *CyclesElapsed = 5;
            
            Log.InstrValue1 = (uint8) IndirectAddress;
            Log.InstrValue2 = (uint8) (IndirectAddress >> 8);
            Log.InstrAddress = Registers->PrgCounter;
            Log.BytesRead = 3;
            Log.InstrName = "JMP";
            break;
        }
        
        // Branch Instructions 
        case 0x10: case 0x30: case 0x50: case 0x70: case 0x90: case 0xB0: case 0xD0: case 0xF0:
        {
            BytesRead = 2;
            *CyclesElapsed = 2;

            uint8 StatusFlag = Registers->Flags;
            
            if( (CurrentInstr == 0x10 && !isBitSet(NEGATIVE_BIT, StatusFlag)) || // BPL - Negative is clear
                (CurrentInstr == 0x30 &&  isBitSet(NEGATIVE_BIT, StatusFlag)) || // BMI - Negative is set
                (CurrentInstr == 0x50 && !isBitSet(OVERFLOW_BIT, StatusFlag)) || // BVC - Overflow is clear
                (CurrentInstr == 0x70 &&  isBitSet(OVERFLOW_BIT, StatusFlag)) || // BVS - Overflow is set
                (CurrentInstr == 0x90 && !isBitSet(CARRY_BIT, StatusFlag))    || // BCC - Carry is clear
                (CurrentInstr == 0xB0 &&  isBitSet(CARRY_BIT, StatusFlag))    || // BCS - Carry is set
                (CurrentInstr == 0xD0 && !isBitSet(ZERO_BIT, StatusFlag))     || // BNE - Zero is clear
                (CurrentInstr == 0xF0 &&  isBitSet(ZERO_BIT, StatusFlag)) )      // BEQ - Zero is set
            {
                int8 RelAddress = relative(Registers, MemoryOffset);
                Registers->PrgCounter += RelAddress; // Plus two to next instruction
                *CyclesElapsed += 1; // TODO: Add cycles for crossings boundaries
            }
            Log.InstrName = "BRC";
            break;
        }
        
        // NOTE: Load Memory Operations
             
        case 0xA9: case 0xA5: case 0xB5: case 0xAD: case 0xBD:   // LDA
        case 0xB9: case 0xA1: case 0xB1: 
        case 0xA2: case 0xA6: case 0xB6: case 0xAE: case 0xBE:   // LDX
        case 0xA0: case 0xA4: case 0xB4: case 0xAC: case 0xBC:   // LDY   
        {
            uint8 Value;
            if(CurrentInstr == 0xA9 || CurrentInstr == 0xA2 || CurrentInstr == 0xA0) // Immediate
            {
                Value = immediate(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 2;
            }           
            if(CurrentInstr == 0xA5 || CurrentInstr == 0xA6 || CurrentInstr == 0xA4) // Zero Page
            {
                Value = zeroPage(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 3;
            }
            if(CurrentInstr == 0xB5 || CurrentInstr == 0xB4) // (Zero Page, X)
            {
                Value = zeroPageX(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 4;
            }
            if(CurrentInstr == 0xB6)
            {
                Value = zeroPageY(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 4;
            }
            if(CurrentInstr == 0xAD || CurrentInstr == 0xAE || CurrentInstr == 0xAC) // Absolute
            {
                Value = abs(Registers, MemoryOffset);
                BytesRead = 3;
                *CyclesElapsed = 4;                        
            }
            if(CurrentInstr == 0xBD || CurrentInstr == 0xBC) // Absolute, X
            {
                Value = absX(Registers, MemoryOffset);
                BytesRead = 3;
                *CyclesElapsed = 4; // TODO: Boundary fix                         
            }
            if(CurrentInstr == 0xB9 || CurrentInstr == 0xBE) // Absolute, Y
            {
                Value = absY(Registers, MemoryOffset);
                BytesRead = 3;
                *CyclesElapsed = 4; // TODO: Boundary fix                         
            }
            if(CurrentInstr == 0xA1) // Indirect, X
            {
                Value = indirectX(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 6;
            }
            if(CurrentInstr == 0xB1) // (Indirect), Y
            {
                Value = indirectY(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 5; // TODO: Boundary fix
            }

            uint8 RegisterToFill;
            switch(CurrentInstr)
            {
                case 0xA9: case 0xA5: case 0xB5: case 0xAD: // Register A
                case 0xBD: case 0xB9: case 0xA1: case 0xB1: 
                    Log.InstrName = "LDA";
                    Registers->A = Value;
                    break;
                case 0xA2: case 0xA6: case 0xB6: case 0xAE: case 0xBE: // Register X
                    Log.InstrName = "LDX";
                    Registers->X = Value;
                    break;
                case 0xA0: case 0xA4: case 0xB4: case 0xAC: case 0xBC: // Register Y
                    Log.InstrName = "LDY";
                    Registers->Y = Value;
                    break;
                default:
                    Assert(0);
                    break;
            }
            
            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);                        
            break;
        }

        // NOTE: Store Memory Operations
        
        case 0x85: case 0x95: case 0x8D: case 0x9D: // STA
        case 0x99: case 0x81: case 0x91:
        case 0x86: case 0x96: case 0x8E:            // STX
        case 0x84: case 0x94: case 0x8C:            // STY
        {
            uint16 Address = {};
            
            if(CurrentInstr == 0x85 || CurrentInstr == 0x86 || CurrentInstr == 0x84) // Zero Page
            {
                Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 3;
            }
            
            if(CurrentInstr == 0x95 || CurrentInstr == 0x94) // ZeroPage, X
            {
                Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
                Address += Registers->X;
                BytesRead = 2;
                *CyclesElapsed = 4;
            }
            if(CurrentInstr == 0x96) // ZeroPage, Y
            {
                Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
                Address += Registers->Y;
                BytesRead = 2;
                *CyclesElapsed = 4;
            }
            
            if(CurrentInstr == 0x8D || CurrentInstr == 0x8E || CurrentInstr == 0x8C) // Absolute
            {
                Address = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset);
                BytesRead = 3;
                *CyclesElapsed = 4;
            }

            if(CurrentInstr == 0x9D) // Absolute, X
            {
                Address = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset); 
                Address += Registers->X;
                BytesRead = 3;
                *CyclesElapsed = 5; 
            }            
            if(CurrentInstr == 0x99) // Absolute, Y
            {
                Address = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset); 
                Address += Registers->Y;
                BytesRead = 3;
                *CyclesElapsed = 5; 
            }

            if(CurrentInstr == 0x91) // (Indirect, X)
            {
                uint8 ZeroAddress = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset); 
                Address = readCpuMemory16(ZeroAddress + Registers->X, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 6; // TODO: Check timing on this           
            }
            
            if(CurrentInstr == 0x91) // (Indirect), Y
            {
                uint8 ZeroAddress = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset); 
                uint16 IndirectAddress = readCpuMemory16(ZeroAddress, MemoryOffset);
                Address = IndirectAddress + Registers->Y;
                BytesRead = 2;
                *CyclesElapsed = 6; // TODO: Check timing on this           
            }


            
            Log.InstrValue1 = (uint8) Address;
            Log.InstrValue2 = (uint8) (Address >> 8);
            Log.InstrAddress = Address;
            
            uint8 RegisterValue;
            switch(CurrentInstr)
            {
                case 0x85: case 0x95: case 0x8D: case 0x9D: // Register A
                case 0x99: case 0x81: case 0x91:
                    Log.InstrName = "STA";
                    Log.AddressValue = Registers->A;
                    RegisterValue = Registers->A;
                    break;
                case 0x86: case 0x96: case 0x8E: // Register X
                    Log.InstrName = "STX";
                    Log.AddressValue = Registers->X;
                    RegisterValue = Registers->X;
                    break;
                case 0x84: case 0x94: case 0x8C: // Register Y
                    Log.InstrName = "STY";
                    Log.AddressValue = Registers->Y;
                    RegisterValue = Registers->Y;
                    break;
                default:
                    Assert(0);
                    break;
            }
            
            writeCpuMemory8(RegisterValue, Address, MemoryOffset);
            break;
        }
      
        
        // NOTE: Bit Shifts
        
        case 0x06: // ASL(ZeroPage) - Shift Mem Left 1 bit
        {
            uint8 Address = readCpuMemory8(Registers->PrgCounter+1, MemoryOffset);
            uint8 Value = readCpuMemory8(Address, MemoryOffset);
            
            if(Value & (1 << 7))
                setCarry(&Registers->Flags);
            else
                clearCarry(&Registers->Flags);
          
            Value = Value << 1;
            writeCpuMemory8(Value, Address, MemoryOffset);
                        
            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);
            BytesRead = 2;
            *CyclesElapsed = 5;

            Log.InstrValue1 = Log.InstrAddress = Address;
            Log.AddressValue = Value;
            Log.InstrName = "ASL";
            break;
        }
        case 0x0A: // ASL(Accumulator)
        {
            uint8 Value = Registers->A;
            
            if(Value & (1 << 7))
                setCarry(&Registers->Flags);
            else
                clearCarry(&Registers->Flags);
          
            Registers->A = Value << 1;
                        
            setNegative(Registers->A, &Registers->Flags);
            setZero(Registers->A, &Registers->Flags);
            BytesRead = 1;
            *CyclesElapsed = 2;

            Log.InstrName = "ASL";
            break;
        }
        case 0x4A: // LSR(Accumulator) - logical shift right
        {
            uint8 Value = Registers->A;
            if(Value & 1)
                setCarry(&Registers->Flags);
            else
                clearCarry(&Registers->Flags);
            
            Registers->A = Value >> 1;
            
            setNegative(Registers->A, &Registers->Flags);
            setZero(Registers->A, &Registers->Flags);
            BytesRead = 1;
            *CyclesElapsed = 2;
            
            Log.InstrName = "LSR";
            break;
        }
        case 0x46: // LSR(ZeroPage)
        {
            uint8 Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
            uint8 Value = zeroPage(Registers, MemoryOffset);
            if(Value & 1)
                setCarry(&Registers->Flags);
            else
                clearCarry(&Registers->Flags);
            
            Value = Value >> 1;
            
            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);
            
            writeCpuMemory8(Value, Address, MemoryOffset);
            BytesRead = 2;
            *CyclesElapsed = 5;
            
            Log.InstrName = "LSR";
            break;
        }

        case 0x2A: // ROL - Accumulator
        {
            uint8 Value = Registers->A;
            
            bool32 CarryIsSet = Registers->Flags & 1;
            if(Value & (1 << 7))
                setCarry(&Registers->Flags);
            else
                clearCarry(&Registers->Flags);

            Value = Value << 1;
            
            if(CarryIsSet)
                Value = Value & 1;

            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);

            Registers->A = Value;

            BytesRead = 1;
            *CyclesElapsed = 2;
            
            Log.InstrName = "ROL";
            break;
        }
        
        case 0x26: case 0x36: // ROL
        {
            uint16 Address;
            uint8 Value;
            if(CurrentInstr == 0x26) // ZeroPage
            {
                Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
                Value = zeroPage(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 5;
            }
            if(CurrentInstr == 0x36) // ZeroPage, X
            {
                Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset) + Registers->X;
                Value = zeroPageX(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 6;
            }
            
            bool32 CarryIsSet = Registers->Flags & 1;
            if(Value & (1 << 7))
                setCarry(&Registers->Flags);
            else
                clearCarry(&Registers->Flags);

            Value = Value << 1;
            
            if(CarryIsSet)
                Value = Value & 1;

            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);

            writeCpuMemory8(Value, Address, MemoryOffset);

            Log.InstrName = "ROL";
            break;
        }

        
        case 0x6A: // ROR - Accumulator
        {
            uint8 Value = Registers->A;

            bool32 CarryIsSet = Registers->Flags & 1;
            if(Value & 1)
                setCarry(&Registers->Flags);
            else
                clearCarry(&Registers->Flags);

            Value = Value >> 1;
            
            if(CarryIsSet)
                Value = Value & (1 << 7);

            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);
            Registers->A = Value;

            BytesRead = 1;
            *CyclesElapsed = 2;
            Log.InstrName = "ROR";
            break;
        }
        
        case 0x66: case 0x6E: // ROR
        {
            uint16 Address;
            uint8 Value;

            if(CurrentInstr == 0x66) // ZeroPage
            {
                Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
                Value = zeroPage(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 5;
            }
            if(CurrentInstr == 0x6E) // Absolute
            {
                Address = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset);
                Value = abs(Registers, MemoryOffset);
                BytesRead = 3;
                *CyclesElapsed = 6;
            }
            
            bool32 CarryIsSet = Registers->Flags & 1;
            if(Value & 1)
                setCarry(&Registers->Flags);
            else
                clearCarry(&Registers->Flags);

            Value = Value >> 1;
            
            if(CarryIsSet)
                Value = Value & (1 << 7);

            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);

            writeCpuMemory8(Value, Address, MemoryOffset);

            Log.InstrName = "ROR";
            break;
        }
        
        case 0x24: // BIT - zeropage
        {
            uint8 Value = zeroPage(Registers, MemoryOffset);

            setNegative(Value, &Registers->Flags);
            setZero(Registers->A & Value, &Registers->Flags);

            if(Value & (1 << 6))
                setCarry(&Registers->Flags);
            else
                clearCarry(&Registers->Flags);

            BytesRead = 2;
            *CyclesElapsed = 3;
            Log.InstrName = "BIT";
            break;
        }

        
        // NOTE: Logic Operations          
                    
        case 0x29: case 0x25: case 0x35: case 0x2D: // AND  
        case 0x3D: case 0x39: case 0x21: case 0x31:
        case 0x49: case 0x45: case 0x55: case 0x4D: // EOR
        case 0x5D: case 0x59: case 0x41: case 0x51:
        case 0x09: case 0x05: case 0x15: case 0x0D: // ORA
        case 0x1D: case 0x19: case 0x01: case 0x11: 
        {
            uint8 Value;
            if(CurrentInstr == 0x29 || CurrentInstr == 0x49 || CurrentInstr == 0x09) // Immediate 
            {
                Value = immediate(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 2;
            }
            if(CurrentInstr == 0x25 || CurrentInstr == 0x45 || CurrentInstr == 0x05) // Zeropage
            {
                Value = zeroPage(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 3;
            }                   
            if(CurrentInstr == 0x35 || CurrentInstr == 0x55 || CurrentInstr == 0x15) // ZeroPage, X
            {
                Value = zeroPageX(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 4;
            }
            if(CurrentInstr == 0x2D || CurrentInstr == 0x4D || CurrentInstr == 0x0D) // Absolute
            {
                Value = abs(Registers, MemoryOffset);
                BytesRead = 3;
                *CyclesElapsed = 4;
            }          
            if(CurrentInstr == 0x3D  || CurrentInstr == 0x5D || CurrentInstr == 0x1D) // Absolute, X
            {
                Value = absX(Registers, MemoryOffset);
                BytesRead = 3;
                *CyclesElapsed = 4; // TODO: Boundary
            }
            if(CurrentInstr == 0x39 || CurrentInstr == 0x59 || CurrentInstr == 0x19) // Absolute, Y
            {
                Value = absY(Registers, MemoryOffset);
                BytesRead = 3;
                *CyclesElapsed = 4; // TODO: Boundary
            }
            if(CurrentInstr == 0x21 || CurrentInstr == 0x41 || CurrentInstr == 0x01) // (Indirect, X)
            {
                Value = indirectX(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 6;
            }
            if(CurrentInstr == 0x31 || CurrentInstr == 0x51 || CurrentInstr == 0x11) // Indirect, Y
            {
                Value = indirectY(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 5; //TODO: Boundary 
            }

            switch(CurrentInstr)
            {
                case 0x29: case 0x25: case 0x35: case 0x2D: // AND  
                case 0x3D: case 0x39: case 0x21: case 0x31:
                    Log.InstrName = "AND";
                    Registers->A = Registers->A & Value;
                    break;
                case 0x49: case 0x45: case 0x55: case 0x4D: // EOR
                case 0x5D: case 0x59: case 0x41: case 0x51:
                    Log.InstrName = "EOR";
                    Registers->A = Registers->A ^ Value;
                    break;
                case 0x09: case 0x05: case 0x15: case 0x0D: // ORA
                case 0x1D: case 0x19: case 0x01: case 0x11:
                    Log.InstrName = "ORA";
                    Registers->A = Registers->A | Value;
                    break;
            }            
            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);
            break;
        }
                
        // NOTE: Transfering Register Values 
        
        case 0x8A: case 0xAA: case 0xA8: case 0x98: case 0x9A:
        {
            uint8 Value;
            if(CurrentInstr == 0x8A) // TXA
            {
                Log.InstrName = "TXA";
                Registers->A = Value = Registers->X;
            }
            if(CurrentInstr == 0xAA) // TAX
            {
                Log.InstrName = "TAX";
                Registers->X = Value = Registers->A;
            }
            if(CurrentInstr == 0xA8) // TAY
            {
                Log.InstrName = "TAY";
                Registers->Y = Value = Registers->A;
            }
            if(CurrentInstr == 0x98) // TYA
            {
                Log.InstrName = "TYA";
                Registers->A = Value = Registers->Y;
            }
            if(CurrentInstr == 0x9A) // TXS
            {
                Log.InstrName = "TXS";
                Registers->StackPtr = Value = Registers->X;
            }
            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);
            BytesRead = 1;
            *CyclesElapsed = 2;
            break;
        }

        // NOTE: Stack Operations
        
        case 0x48: // PHA - Push accumulator on stack
        {
            pushStack(Registers->A, &Registers->StackPtr, MemoryOffset);
            BytesRead = 1;
            *CyclesElapsed = 3;
                   
            Log.InstrName = "PHA";
            break;
        }
        case 0x68: // PLA - Pop to accumulator
        {
            Registers->A = popStack(&Registers->StackPtr, MemoryOffset);
            BytesRead = 1;
            *CyclesElapsed = 4;

            Log.InstrName = "PLA";
            break;
        }

        // NOTE: Increment and Decrement
        
        case 0xCA: case 0x88: case 0xE8: case 0xC8:
        {
            uint8 *Register;
            if(CurrentInstr == 0xCA || CurrentInstr == 0xE8)
                Register = &Registers->X;
            if(CurrentInstr == 0x88 || CurrentInstr == 0xC8)
                Register = &Registers->Y;

            if(CurrentInstr == 0xE8 || CurrentInstr == 0xC8) // Increment
            {
                Log.InstrName = "INC";
                *Register += 1;
            }
            if(CurrentInstr == 0xCA || CurrentInstr == 0x88) // Decrement
            {
                Log.InstrName = "DEC";
                *Register -= 1;
            }
                        
            setNegative(*Register, &Registers->Flags);
            setZero(*Register, &Registers->Flags);
            BytesRead = 1;
            *CyclesElapsed = 2;
            break;
        }

        case 0xC6: case 0xD6: case 0xCE: // DEC
        {
            uint16 Address;
            uint8 Value;
            if(CurrentInstr == 0xC6) // ZeroPage
            {
                Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
                Value = zeroPage(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 5;
            }
            if(CurrentInstr == 0xD6) // ZeroPage, X
            {
                Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset) + Registers->X;
                Value = zeroPageX(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 6;
            }
            if(CurrentInstr == 0xCE) // Absolute
            {
                Address = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset);
                Value = abs(Registers, MemoryOffset);
                BytesRead = 3;
                *CyclesElapsed = 6;
            }
           
            --Value;
            
            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);

            writeCpuMemory8(Value, Address, MemoryOffset);

            Log.InstrName = "DEC";    
            break;
        }
        
        case 0xE6: case 0xEE: // INC
        {
            uint16 Address;
            uint8 Value;
            if(CurrentInstr == 0xE6) // Zero Page
            {
                Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
                Value = zeroPage(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 5;
            }
            if(CurrentInstr == 0xEE) // Absolute
            {
                Address = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset);
                Value = abs(Registers, MemoryOffset);
                BytesRead = 3;
                *CyclesElapsed = 6;
            }

            ++Value;
            
            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);
            writeCpuMemory8(Value, Address, MemoryOffset);

            Log.InstrName = "INC";
            break;
        }
        
        // NOTE: Add and Subtract with Carry
        
        case 0x69: case 0x65: case 0x6D: case 0x7D:// ADC - add with carry
        {
            uint8 Value;
            if(CurrentInstr == 0x69)// Immediate
            {
                Value = immediate(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 2;
            }
            if(CurrentInstr == 0x65) // zeropage
            {
                Value = zeroPage(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 3;
            }
            if(CurrentInstr == 0x6D) // Absolute
            {
                Value = abs(Registers, MemoryOffset);
                BytesRead = 3;
                *CyclesElapsed = 4;
            }
            if(CurrentInstr == 0x7D) // Absolute, X
            {
                Value = absX(Registers, MemoryOffset);
                BytesRead = 3;
                *CyclesElapsed = 4; // TODO: Boundary
            }
            adc(Value, Registers);
            Log.InstrName = "ADC";
            break;
        }
        case 0xE1: case 0xF1: // SBC - Subtract with Carry
        {
            uint8 Value;
            if(CurrentInstr == 0xE1) // (Indirect, X)
            {
                Value = indirectX(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 6;
            }
            if(CurrentInstr == 0xF1) // (Indirect), Y
            {
                Value = indirectY(Registers, MemoryOffset); 
                BytesRead = 2;
                *CyclesElapsed = 5; // TODO: Boundary cross is +1
            }
            sbc(Value, Registers);
            Log.InstrName = "SBC";
            break;
        }
        
        // NOTE: Compare instructions       
        case 0xC9: case 0xC5: case 0xD5: case 0xCD: // CMP
        case 0xDD: case 0xD9: case 0xC1: case 0xD1:
        case 0xE0: case 0xE4: case 0xEC:            // CPX
        case 0xC0: case 0xC4: case 0xCC:            // CPY
        {
            uint8 Value = {};
            
            if(CurrentInstr == 0xC9 || CurrentInstr == 0xE0 || CurrentInstr == 0xC0) // Immediate
            {
                Value = immediate(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 2;
            }
            if(CurrentInstr == 0xC5 || CurrentInstr == 0xE4 || CurrentInstr == 0xC4) // Zero Page
            {
                Value = zeroPage(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 3;
            }
            if(CurrentInstr == 0xD5) // ZeroPage, X
            {
                Value = zeroPageX(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 4;
            }
            if(CurrentInstr == 0xCD || CurrentInstr == 0xEC || CurrentInstr == 0xCC) // Absolute
            {
                Value = abs(Registers, MemoryOffset);
                BytesRead = 3;
                *CyclesElapsed = 4;
            }
            if(CurrentInstr == 0xDD) // Absolute, X
            {
                Value = absX(Registers, MemoryOffset);
                BytesRead = 3;
                *CyclesElapsed = 4; // TODO: **
            }
            if(CurrentInstr == 0xD9) // Absolute, Y
            {
                Value = absY(Registers, MemoryOffset);
                BytesRead = 3;
                *CyclesElapsed = 4; // TODO: **
            }
            if(CurrentInstr == 0xC1) // Indirect, X
            {
                Value = indirectX(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 6;
            }
            if(CurrentInstr == 0xD1) // Indirect, Y
            {
                Value = indirectY(Registers, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 5; // TODO: Boundary
            }            

            uint8 ToCompareAgainst;
            switch(CurrentInstr)
            {
                case 0xC9: case 0xC5: case 0xD5: case 0xCD: // CMP
                case 0xDD: case 0xD9: case 0xC1: case 0xD1:
                    Log.InstrName = "CMP";
                    ToCompareAgainst = Registers->A;
                    break;
                case 0xE0: case 0xE4: case 0xEC: // CPX
                    Log.InstrName = "CPX";
                    ToCompareAgainst = Registers->X;
                    break;
                case 0xC0: case 0xC4: case 0xCC: // CPY
                    Log.InstrName = "CPY";
                    ToCompareAgainst = Registers->Y;
                    break;
            }
            cmp(Value, ToCompareAgainst, &Registers->Flags);
            break;
        }
       
        // NOTE: Status Flag set and clear opcodes
        case 0x78: case 0xD8: case 0x38: case 0x18: // Set Interrupt Disable Flag
        {
            if(CurrentInstr == 0x78)
                setInterrupt(&Registers->Flags);
            if(CurrentInstr == 0xD8)
                clearDecimal(&Registers->Flags);
            if(CurrentInstr == 0x38)
                setCarry(&Registers->Flags);
            if(CurrentInstr == 0x18)
                clearCarry(&Registers->Flags);
            BytesRead = 1;
            *CyclesElapsed = 2;
            Log.InstrName = "FLG";
            break;
        }

        case 0xEA: // NOP
        {
            BytesRead = 1;
            *CyclesElapsed = 2;
            Log.InstrName = "NOP";
            break;
        }

        default:
        {
            uint8 MissingValue = CurrentInstr;
            char Buffer[8];
            sprintf(Buffer, "%X\n", MissingValue);
            OutputDebugString(Buffer);
            Assert(0);
            break;
        }
    }

#if 1
    
    char LogBuffer[512];

    if(BytesRead != 0)
        Log.BytesRead = BytesRead;

    
    
    switch(Log.BytesRead)
    {
        case 1:
        {
            if(Log.AddressValue)
                sprintf(LogBuffer, "$%.4X:%.2X        %s $%.4X = $%.2X        A:%.2X X:%.2X, Y:%.2X S:%.2X P:%X\n",
                        Log.ProgramLine, Log.Instr, Log.InstrName, Log.InstrAddress, Log.AddressValue,
                        Registers->A, Registers->X, Registers->Y, Registers->StackPtr, Registers->Flags);
            else
                sprintf(LogBuffer, "$%.4X:%.2X        %s                      A:%.2X X:%2X, Y:%.2X S:%.2X P:%X\n",
                        Log.ProgramLine, Log.Instr, Log.InstrName,
                        Registers->A, Registers->X, Registers->Y, Registers->StackPtr, Registers->Flags);
            

            break;
        }
        case 2:
        {
            if(Log.AddressValue)
                sprintf(LogBuffer, "$%.4X:%2X %2X     %s $%.4X = $%.2X          A:%.2X X:%.2X, Y:%.2X S:%.2X P:%X\n",
                        Log.ProgramLine, Log.Instr, Log.InstrValue1, Log.InstrName, Log.InstrAddress, Log.AddressValue,
                        Registers->A, Registers->X, Registers->Y, Registers->StackPtr, Registers->Flags);
            else
                sprintf(LogBuffer, "$%.4X:%2X %2X     %s $%.4X                A:%.2X X:%.2X, Y:%.2X S:%.2X P:%X\n",
                        Log.ProgramLine, Log.Instr, Log.InstrValue1, Log.InstrName, Log.InstrAddress, 
                        Registers->A, Registers->X, Registers->Y, Registers->StackPtr, Registers->Flags);
            
                break;
        }
        case 3:
        {
            if(Log.AddressValue)
                sprintf(LogBuffer, "$%.4X:%.2X %.2X %.2X  %s $%.4X = $%.2X          A:%.2X X:%.2X, Y:%.2X S:%.2X P:%X\n",
                    Log.ProgramLine, Log.Instr, Log.InstrValue1, Log.InstrValue2, Log.InstrName, Log.InstrAddress, Log.AddressValue,
                        Registers->A, Registers->X, Registers->Y, Registers->StackPtr, Registers->Flags);
            
            else
                sprintf(LogBuffer, "$%.4X:%.2X %.2X %.2X  %s $%.4X                A:%.2X X:%.2X, Y:%.2X S:%.2X P:%X\n",
                        Log.ProgramLine, Log.Instr, Log.InstrValue1, Log.InstrValue2, Log.InstrName, Log.InstrAddress,
                        Registers->A, Registers->X, Registers->Y, Registers->StackPtr, Registers->Flags);
                
            break;
        }
    }
    

    OutputDebugString(LogBuffer);


#endif
   
    Registers->PrgCounter += BytesRead;
    linecount++;
    /*
    if(NMICalled)
    {
        NMICalled = false;

        uint8 HighByte = (uint8)(Registers->PrgCounter >> 8);
        uint8 LowByte = (uint8)Registers->PrgCounter;
                        
        pushStack(HighByte, &Registers->StackPtr, MemoryOffset);
        pushStack(LowByte, &Registers->StackPtr, MemoryOffset);

        setInterrupt(&Registers->Flags);
        pushStack(Registers->Flags, &Registers->StackPtr, MemoryOffset);

        Registers->PrgCounter = readCpuMemory16(NMI_VEC, MemoryOffset);
           
        BytesRead = 0; // TODO: Check this
        *CyclesElapsed = *CyclesElapsed + 7;        
    }
    */
}




// NOTE: DEBUG FUNCTIONS IF NEEDED LATER

void debugPrintStack(uint8 StackPointer, uint64 MemoryOffset)
{
    for(uint8 Element = StackPointer; Element != 0; Element++)
    {
        uint8 *Address = (uint8 *)((Element + STACK_ADRS) + MemoryOffset);
        char StackBuffer[512];
        sprintf(StackBuffer, "%X: %X, ", (uint8)Element, *Address);
        OutputDebugString(StackBuffer);
    }
    OutputDebugString("\n");
}
