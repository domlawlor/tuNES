/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#define NMI_VEC     0xFFFA
#define RESET_VEC   0xFFFC
#define IRQ_BRK_VEC 0xFFFE 
    

/*
  CPU Memory Map - taken from nesdev.com
  $0000-$07FF   $0800   2KB internal RAM
  $0800-$0FFF   $0800   Mirrors of $0000-$07FF
  $1000-$17FF   $0800
  $1800-$1FFF   $0800
  $2000-$2007   $0008   NES PPU registers
  $2008-$3FFF   $1FF8   Mirrors of $2000-2007 (repeats every 8 bytes)
  $4000-$4017   $0018   NES APU and I/O registers
  $4018-$401F   $0008   APU and I/O functionality that is normally disabled. See CPU Test Mode.
  $4020-$FFFF   $BFE0   Cartridge space: PRG ROM, PRG RAM, and mapper registers (See Note)
      
*/

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


inline void setNegative(uint8 Value, uint8 *Flags)
{  
    if(Value >= 0x00 && Value <= 0x7F)
        *Flags = *Flags & ~(1 << 7); // clear negative flag
    else
        *Flags = *Flags | (1 << 7); // set negative flag
}
inline void setZero(uint8 Value, uint8 *Flags)
{
    if(Value == 0x00)
        *Flags = *Flags | (1 << 1); // Set zero flag
    else
        *Flags = *Flags & ~(1 << 1);
}
inline void setCarry(uint8 *Flags)
{
    *Flags = *Flags | 1;
}
inline void clearCarry(uint8 *Flags)
{
    *Flags = *Flags & ~1;
}
inline void setOverflow(uint8 *Flags)
{
    *Flags = *Flags | (1 << 6);
}
inline void clearOverflow(uint8 *Flags)
{
    *Flags = *Flags & ~(1 << 6);
}


void adc(uint8 Value, cpuRegisters* Registers)
{
    uint8 Carry = (Registers->Flags & 1);            
    int32 Result = (int8)Registers->A + (int8)Value + Carry;
            
    if(Result < -128 || Result > 127)
    {
        setOverflow(&Registers->Flags);
    }
    else
        clearOverflow(&Registers->Flags);
                        
    Registers->A = (uint8)Result;

    if(Registers->A < (Value + Carry))
    {
        clearCarry(&Registers->Flags);
    }
    else
        setCarry(&Registers->Flags);
            
    setNegative((uint8)Result, &Registers->Flags);
    setZero((uint8)Result, &Registers->Flags);
}
void sbc(uint8 Value, cpuRegisters* Registers)
{
    uint8 Carry = 1 - (Registers->Flags & 1);
    int32 Result = (int8)Registers->A - (int8)Value - Carry;
            
    if(Result < -128 || Result > 127)
    {
        setOverflow(&Registers->Flags);
    }
    else
        clearOverflow(&Registers->Flags);
                        
    Registers->A = (uint8)Result;

    if(Registers->A < (Value - Carry))
    {
        clearCarry(&Registers->Flags);
    }
    else
        setCarry(&Registers->Flags);
            
    setNegative((uint8)Result, &Registers->Flags);
    setZero((uint8)Result, &Registers->Flags);
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

// NOTE:
// Stack pointer is commonly initialised to 0xFF.
// Adding a byte will decrease the pointer.
// Poping increases the pointer.
// The pointer does wrap if under 0x100    
#define STACK_ADRS 0x100


void PushStack(uint8 Byte, uint8 *StackPointer, uint64 MemoryOffset)
{
    uint8 *Address = (uint8 *)((*StackPointer + STACK_ADRS) + MemoryOffset);
    *Address = Byte;
    --(*StackPointer);  
}
uint8 PopStack(uint8 *StackPointer, uint64 MemoryOffset)
{
    uint8 *Address = (uint8 *)((*StackPointer + STACK_ADRS) + MemoryOffset);
    uint8 Value = *Address;
    ++(*StackPointer);    
    return(Value);
}

void cpuTick(cpu *CpuData)
{
    cpuRegisters *Registers = &CpuData->Registers;
    uint8 *CyclesElapsed = &CpuData->LastTickCycles;
    uint64 MemoryOffset = CpuData->MemoryOffset; 

    uint8 BytesRead = 0;
    
    uint8 CurrentInstr = readMemory8(Registers->PrgCounter, MemoryOffset);                
                
    switch(CurrentInstr)
    {
        case 0x00: // BRK - Break
        {
            Registers->PrgCounter += 2;
            uint8 HighByte = (uint8)(Registers->PrgCounter >> 8);
            uint8 LowByte = (uint8)Registers->PrgCounter;
                        
            PushStack(HighByte, &Registers->StackPtr, MemoryOffset);
            PushStack(LowByte, &Registers->StackPtr, MemoryOffset);
            PushStack(Registers->Flags, &Registers->StackPtr, MemoryOffset);

            Registers->Flags = Registers->Flags | (1 << 2);
            
            // Read the break location from the IRQ/BRK Vector
#define IRQ_VEC 0xFFFE
            Registers->PrgCounter = readCpuMemory16(IRQ_VEC, MemoryOffset);
           
            BytesRead = 0; // TODO: Check this
            *CyclesElapsed = 7;
            break;
        }
        case 0x40: // RTI - Return from Interrupt
        {
            Registers->Flags = PopStack(&Registers->StackPtr, MemoryOffset);
            uint8 LowBytes = PopStack(&Registers->StackPtr, MemoryOffset);
            uint8 HighBytes = PopStack(&Registers->StackPtr, MemoryOffset);
            Registers->PrgCounter = (HighBytes << 8) | LowBytes;
            
            BytesRead = 0;
            *CyclesElapsed = 6;
            break;
        }
        
        // Branch Instructions 
        case 0x10: case 0x90: case 0xB0: case 0xD0: case 0xF0:
        {
            BytesRead = 2;
            *CyclesElapsed = 2;
                        
            int8 RelAddress = readMemory8(Registers->PrgCounter + 1, MemoryOffset);
                        
            if( (CurrentInstr == 0x10 && !(Registers->Flags & (1 << 7))) ||  // BPL - Branch if plus.
                (CurrentInstr == 0xB0 && !(Registers->Flags & (1)))      ||  // BCC - Branch on carry clear
                (CurrentInstr == 0xB0 &&   Registers->Flags & (1))       ||  // BCS - Branch if Carry is set
                (CurrentInstr == 0xD0 && !(Registers->Flags & (1 << 1))) ||  // BNE - Branch if not equal
                (CurrentInstr == 0xF0 &&   Registers->Flags & (1 << 1)))     // BEQ - Brach if Equal
            {
                Registers->PrgCounter += (RelAddress + 2); // Plus two to next instruction
                BytesRead = 0;
                *CyclesElapsed += 1;
                // TODO: Add cycles for crossings boundaries
            }                       
            break;
        }

        // LDA - Load Accumulator Register with value
        case 0xA5: case 0xA9: case 0xAD: case 0xB1: case 0xBD: case 0xB9:
        {
            uint8 Value;
                        
            if(CurrentInstr == 0xA5) // Zero Page
            {
                uint8 Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
                Value = readCpuMemory8(Address, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 3;
            }
            else if(CurrentInstr == 0xA9) // Immediate
            {
                Value = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 2;
            }
            else if(CurrentInstr == 0xAD) // Absolute
            {
                uint16 Address = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset);
                Value = readCpuMemory8(Address, MemoryOffset);
                BytesRead = 3;
                *CyclesElapsed = 4;                        
            }
            else if(CurrentInstr == 0xB1) // (Indirect), Y
            {
                uint8 ZeroAddress = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset); 
                uint16 IndirectAddress = readCpuMemory16(ZeroAddress, MemoryOffset);
                uint16 FinalAddress = IndirectAddress + Registers->Y;
                Value = readCpuMemory8(FinalAddress, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 5;
            }
            else if(CurrentInstr == 0xBD) // Absolute, X
            {
                uint16 Address = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset);
                uint16 NewAdrs = Address + Registers->X;
                Value = readCpuMemory8(NewAdrs, MemoryOffset);
                BytesRead = 3;
                *CyclesElapsed = 4; // TODO: Boundary fix                         
            }
            else if(CurrentInstr == 0xB9) // Absolute, Y
            {
                uint16 Address = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset);
                uint16 NewAdrs = Address + Registers->Y;
                Value = readCpuMemory8(NewAdrs, MemoryOffset);
                BytesRead = 3;
                *CyclesElapsed = 4; // TODO: Boundary fix                         
            }
                        
            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);                        
            Registers->A = Value;
            break;
        }

        case 0x06: // ASL(ZeroPage) - Shift Mem Left 1 bit
        {
            uint8 Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
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
            break;
        }
        case 0x0A: // ASL(Accumulator)
        {
            uint8 Value = Registers->A;
            
            if(Value & (1 << 7))
                setCarry(&Registers->Flags);
            else
                clearCarry(&Registers->Flags);
          
            Value = Value << 1;

            Registers->A = Value;
                        
            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);
            BytesRead = 1;
            *CyclesElapsed = 2;
            
            break;
        }
        case 0x4A: // LSR(Accumulator) - logical shift right
        {
            if(Registers->A & 1)
                setCarry(&Registers->Flags);
            else
                clearCarry(&Registers->Flags);

            Registers->A = Registers->A >> 1;
            BytesRead = 1;
            *CyclesElapsed = 2;
            break;
        }
        case 0x26: // ROL(Zeropage)
        {
            uint16 Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
            uint8 Value = readCpuMemory8(Address, MemoryOffset);

            bool32 CarryIsSet = Registers->Flags & 1;
            if(Registers->Flags & (1 << 7))
                setCarry(&Registers->Flags);
            else
                clearCarry(&Registers->Flags);

            Value = Value << 1;
            if(CarryIsSet)
                Value = Value & 1;
            
            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);
            
            writeCpuMemory8(Value, Address, MemoryOffset);
                                  
            BytesRead = 2;
            *CyclesElapsed = 6;
            break;
        }
        case 0x36: // ROL(Zeropage, X) - Rotate left one bit
        {
            uint16 Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
            Address += Registers->X;
            uint8 Value = readCpuMemory8(Address, MemoryOffset);

            bool32 CarryIsSet = Registers->Flags & 1;
            if(Registers->Flags & (1 << 7))
                setCarry(&Registers->Flags);
            else
                clearCarry(&Registers->Flags);

            Value = Value << 1;
            if(CarryIsSet)
                Value = Value & 1;

            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);
            writeCpuMemory8(Value, Address, MemoryOffset);
                                  
            BytesRead = 2;
            *CyclesElapsed = 6;
            
            break;
        }
        case 0x0D: // ORA(Absolute) - OR memory with Accumlator
        {
            uint16 Address = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset);
            uint8 Value = readCpuMemory8(Address, MemoryOffset);

            Registers->A = Registers->A | Value;
            
            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);

            BytesRead = 3;
            *CyclesElapsed = 4;
            break;
        }
        
        case 0xA2: // LDX(Immediate) - Load X index with memory
        {
            uint8 Value = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);

            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);

            Registers->X = Value;
                        
            BytesRead = 2;
            *CyclesElapsed = 2;
            break;
        }
                
                    
        case 0x20: // JSR - Jump to subroutine
        {
            // last byte of jrs (next instruction minus 1) is pushed onto stack
            // program counter jumps to address

            uint16 NewAddress = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset);
                        
            uint16 PrevAdrs = Registers->PrgCounter + 2;                              
            uint8 HighByte = (uint8)(PrevAdrs >> 8);
            uint8 LowByte = (uint8)PrevAdrs;
      
            // Push onto stack, little endian
            PushStack(HighByte, &Registers->StackPtr, MemoryOffset);
            PushStack(LowByte, &Registers->StackPtr, MemoryOffset);

            Registers->PrgCounter = NewAddress;

            BytesRead = 0; // Moved to the next instruction already
            *CyclesElapsed = 6;
            break;
        }
        case 0x60: // RTS - Return from subroutine
        {
            uint8 LowByte = PopStack(&Registers->StackPtr, MemoryOffset);
            uint8 HighByte = PopStack(&Registers->StackPtr, MemoryOffset);
            Registers->PrgCounter = (HighByte << 8) | LowByte; 

            BytesRead = 1;
            *CyclesElapsed = 6;
            break;
        }
                    
        case 0x4C: // JMP(Absolute) - Jump
        {
            uint16 NewAddress = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset);
            Registers->PrgCounter = NewAddress;
            BytesRead = 0;
            *CyclesElapsed = 3;
            break;
        }
                    
        case 0x29: // AND(Immediate) - Logical AND with value and A, stores in A
        {
            uint8 ByteValue = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);

            uint8 ANDValue = Registers->A & ByteValue;

            setNegative(ANDValue, &Registers->Flags);
            setZero(ANDValue, &Registers->Flags);

            Registers->A = ANDValue;

            BytesRead = 2;
            *CyclesElapsed = 2;
            break;
        }
        case 0x25: // AND(Zeropage)
        {
            uint8 Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
            uint8 ByteValue = readCpuMemory8(Address, MemoryOffset);
            uint8 ANDValue = Registers->A & ByteValue;

            setNegative(ANDValue, &Registers->Flags);
            setZero(ANDValue, &Registers->Flags);

            Registers->A = ANDValue;

            BytesRead = 2;
            *CyclesElapsed = 3;
            break;
        }
                    
        case 0xA8: // TAY - Transfer A to Y
        {
            Registers->Y = Registers->A;
            
            setNegative(Registers->A, &Registers->Flags);
            setZero(Registers->A, &Registers->Flags);
            
            BytesRead = 1;
            *CyclesElapsed = 2;
            break;
        }
        case 0x98: // TYA - Transfer Y to A
        {
            Registers->A = Registers->Y;
            
            setNegative(Registers->A, &Registers->Flags);
            setZero(Registers->A, &Registers->Flags);
            
            BytesRead = 1;
            *CyclesElapsed = 2;
            break;
        }

        case 0x8D: case 0x91: case 0x99: case 0x85: // STA - Store A in Memory
        {
            uint16 Address = {};
            if(CurrentInstr == 0x8D) // Absolute
            {
                Address = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset);
                BytesRead = 3;
                *CyclesElapsed = 4;
            }
            else if(CurrentInstr == 0x91) // (Indirect), Y
            {
                uint8 ZeroAddress = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset); 
                uint16 IndirectAddress = readCpuMemory16(ZeroAddress, MemoryOffset);
                Address = IndirectAddress + Registers->Y;
                BytesRead = 2;
                *CyclesElapsed = 6; // TODO: Check timing on this           
            }
            else if(CurrentInstr == 0x99) // Absolute, Y
            {
                Address = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset); 
                Address += Registers->Y;
                BytesRead = 3;
                *CyclesElapsed = 4; // TODO: Check timing on this
            }
            else if(CurrentInstr == 0x85) // Zero Page
            {
                uint8 Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 3;
            }
            writeCpuMemory8(Registers->A, Address, MemoryOffset);
            break;
        }
      
        
        case 0x8E: // STX(Absolute) - store x in memory
        {
            uint16 Address = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset);
                        
            writeCpuMemory8(Registers->X, Address, MemoryOffset);
            BytesRead = 3;
            *CyclesElapsed = 4;
            break;
        }

        case 0x84: // STY(ZeroPage) - Store Y in memory
        {
            uint8 Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
            writeCpuMemory8(Registers->Y, Address, MemoryOffset);
            BytesRead = 2;
            *CyclesElapsed = 3;
            break;
        }
        
        case 0xA0: // LDY(Immediate) - load y
        {
            uint8 Value = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
            Registers->Y = Value;

            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);
            BytesRead = 2;
            *CyclesElapsed = 2;
            
            break;
        }
        case 0xA4: // LDY(ZeroPage)
        {
            uint8 Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
            uint8 Value = readCpuMemory8(Address, MemoryOffset);
            Registers->Y = Value;

            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);
            
            BytesRead = 2;
            *CyclesElapsed = 3;
            break;
        }
        
        case 0xA6: // LDX(ZeroPage)
        {
            uint8 Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
            uint8 Value = readCpuMemory8(Address, MemoryOffset);
            Registers->X = Value;

            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);
            
            BytesRead = 2;
            *CyclesElapsed = 3;
            break;
        }
        
        case 0x9A: // TXS - Transfer X to Stack Pointer
        {
            uint8 Value = Registers->X;
            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);
            Registers->StackPtr = Value;
            BytesRead = 1;
            *CyclesElapsed = 2;
            break;
        }
        case 0x8A: // TXA - Transfer X to A
        {
            Registers->A = Registers->X;
            BytesRead = 1;
            *CyclesElapsed = 2;
            break;
        }
        case 0xAA: // TAX - Transfer A to X
        {
            Registers->X = Registers->A;
            BytesRead = 1;
            *CyclesElapsed = 2;
            break;
        }

        case 0x48: // PHA - Push accumulator on stack
        {
            PushStack(Registers->A, &Registers->StackPtr, MemoryOffset);
            BytesRead = 1;
            *CyclesElapsed = 3;
            break;
        }
        case 0x68: // PLA - Pop to accumulator
        {
            Registers->A = PopStack(&Registers->StackPtr, MemoryOffset);
            BytesRead = 1;
            *CyclesElapsed = 4;
            break;
        }
        
        case 0xCA: // DEX - Decrement X by one
        {
            Registers->X -= 1;
            setNegative(Registers->X, &Registers->Flags);
            setZero(Registers->X, &Registers->Flags);
            BytesRead = 1;
            *CyclesElapsed = 2;
            break;
        }
        case 0x88: // DEY - Decrement Y
        {
            Registers->Y -= 1;
            setNegative(Registers->Y, &Registers->Flags);
            setZero(Registers->Y, &Registers->Flags);
            BytesRead = 1;
            *CyclesElapsed = 2;
            break;
        }
        case 0xE8: // INX - increment X
        {
            Registers->X += 1;
            setNegative(Registers->X, &Registers->Flags);
            setZero(Registers->X, &Registers->Flags);
            BytesRead = 1;
            *CyclesElapsed = 2;
            break;
        }
        case 0xC8: // INX - increment X
        {
            Registers->Y += 1;
            setNegative(Registers->Y, &Registers->Flags);
            setZero(Registers->Y, &Registers->Flags);
            BytesRead = 1;
            *CyclesElapsed = 2;
            break;
        }
        case 0xE6: // INC(ZeroPage) - increment Memory
        {
            uint8 Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
            uint8 Value = readCpuMemory8(Address, MemoryOffset);
            Value++;
            writeCpuMemory8(Value, Address, MemoryOffset);
            
            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);
            BytesRead = 2;
            *CyclesElapsed = 5;
            
            break;
        }
            
        case 0xC6: // DEC(ZeroPage) - Decrement Memory
        {
            uint8 Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
            uint8 Value = readCpuMemory8(Address, MemoryOffset);
            Value -= 1;
            writeCpuMemory8(Value, Address, MemoryOffset);

            setNegative(Value, &Registers->Flags);
            setZero(Value, &Registers->Flags);

            BytesRead = 2;
            *CyclesElapsed = 5;
            break;
        }

        case 0xC5: case 0xC9: case 0xD9:
        {
            uint8 Value = {};
            if(CurrentInstr == 0xC5) // Zero Page
            {
                uint8 Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
                Value = readCpuMemory8(Address, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 3;
            }
            else if(CurrentInstr == 0xC9) // Immediate
            {
                Value = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
                BytesRead = 2;
                *CyclesElapsed = 2;
            }
            else if(CurrentInstr == 0xD9) // Absolute, Y
            {
                uint16 Address = readCpuMemory16(Registers->PrgCounter + 1, MemoryOffset);
                Address += Registers->Y;
                Value = readCpuMemory8(Address, MemoryOffset);
                BytesRead = 3;
                *CyclesElapsed = 4; // TODO: **
            }

            cmp(Value, Registers->A, &Registers->Flags);

            break;
        }

        case 0xE0: // CPX(Immediate) - Compare against X
        {
            uint8 Value = readCpuMemory8(Registers->PrgCounter+1, MemoryOffset);

            cmp(Value, Registers->X, &Registers->Flags);
            
            BytesRead = 2;
            *CyclesElapsed = 2;
            break;
        }
        case 0xE4: // CPX(ZeroPage) - Compare against X
        {
            uint8 Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
            uint8 Value = readCpuMemory8(Address, MemoryOffset);

            cmp(Value, Registers->X, &Registers->Flags);
            
            BytesRead = 2;
            *CyclesElapsed = 3;
                  
            break;
        }
        case 0xC0: // CPY(Immediate) - Compare against Y
        {
            uint8 Value = readCpuMemory8(Registers->PrgCounter+1, MemoryOffset);

            cmp(Value, Registers->Y, &Registers->Flags);
            
            BytesRead = 2;
            *CyclesElapsed = 2;
            break;
        }
        
        case 0x78: // Set Interrupt Disable Flag
        {
            Registers->Flags = Registers->Flags | (1 << 2); // Set interrupt flag

            BytesRead = 1;
            *CyclesElapsed = 2;
            break;
        }
        case 0xD8: // Clear decimal flag
        {
            Registers->Flags = Registers->Flags & ~(1 << 3); // clear decimal flag

            BytesRead = 1;
            *CyclesElapsed = 2;
            break;
        }
        case 0x18: // Clear carry flag
        {
            Registers->Flags = Registers->Flags & ~(1);
            BytesRead = 1;
            *CyclesElapsed = 2;
            break;            
        }
        case 0x69: // ADC(Immediate)
        {
            uint8 Value = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
            adc(Value, Registers);
            BytesRead = 2;
            *CyclesElapsed = 2;
            break;
        }
        case 0x65: // ADC(zeropage)
        {
            uint8 Address = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset);
            uint8 Value = readCpuMemory8(Address, MemoryOffset);
            adc(Value, Registers);
            BytesRead = 2;
            *CyclesElapsed = 3;
            break;
        }
        case 0xE1: // SBC(Indirect, X)
        {
            uint8 ZeroAddress = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset); 
            uint16 IndirectAddress = readCpuMemory16(ZeroAddress + Registers->X, MemoryOffset);
            uint8 Value = readCpuMemory8(IndirectAddress, MemoryOffset);
            sbc(Value, Registers);
            BytesRead = 2;
            *CyclesElapsed = 6;
            break;
        }
        case 0xF1: // SBC(Indirect), Y - Subtract with carry
        {
            uint8 ZeroAddress = readCpuMemory8(Registers->PrgCounter + 1, MemoryOffset); 
            uint16 IndirectAddress = readCpuMemory16(ZeroAddress, MemoryOffset);
            uint16 FinalAddress = IndirectAddress + Registers->Y;
            uint8 Value = readCpuMemory8(FinalAddress, MemoryOffset);
            sbc(Value, Registers);
            BytesRead = 2;
            *CyclesElapsed = 5; // TODO: Boundary cross is +1
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
    //C000  4C F5 C5  JMP $C5F5                       A:00 X:00 Y:00 P:24 SP:FD CYC:  0
    sprintf(LogBuffer, "OpCode: %X Flags:%X A:%X  X:%X  Y:%X  P:%X  SP:%X  BytesRead:%d  Cycles:%d \n",
            CurrentInstr, Registers->Flags, Registers->A, Registers->X, Registers->Y, Registers->PrgCounter,
            Registers->StackPtr, BytesRead, *CyclesElapsed);
    OutputDebugString(LogBuffer);
    
#endif


    
    Registers->PrgCounter += BytesRead;

    
    if(NMICalled)
    {
        uint8 HighByte = (uint8)(Registers->PrgCounter >> 8);
        uint8 LowByte = (uint8)Registers->PrgCounter;
                        
        PushStack(HighByte, &Registers->StackPtr, MemoryOffset);
        PushStack(LowByte, &Registers->StackPtr, MemoryOffset);
        PushStack(Registers->Flags, &Registers->StackPtr, MemoryOffset);

        Registers->Flags = Registers->Flags | (1 << 2);
            
        // Read the break location from the IRQ/BRK Vector
#define NMI_VEC 0xFFFA
        Registers->PrgCounter = readCpuMemory16(NMI_VEC, MemoryOffset);
           
        BytesRead = 0; // TODO: Check this
        *CyclesElapsed = *CyclesElapsed + 7;
        
        NMICalled = false;
    }
}
