
#include "nes.h"
#include "cpu.h"
#include "ppu.h"

static uint8 readPpuRegister(uint16 Address);
static void writePpuRegister(uint8 Byte, uint16 Address);

static void write8(uint8 Byte, uint16 Address, uint64 MemoryOffset)
{   
    uint8 *NewAddress = (uint8 *)(Address + MemoryOffset);
    *NewAddress = Byte;
}

static uint8 read8(uint16 Address, uint64 MemoryOffset)
{
    uint8 *NewAddress = (uint8 *)(Address + MemoryOffset);
    uint8 Value = *NewAddress;
    return(Value);
}


static uint16 cpuMemoryMirror(uint16 Address)
{
    // NOTE: Mirrors the address for the 2kb ram 
    if(0x0800 <= Address && Address < 0x2000)
        Address = (Address % 0x0800);
    // NOTE: Mirror for PPU Registers
    if(0x2008 <= Address && Address < 0x4000)
        Address = (Address % (0x2008 - 0x2000)) + 0x2000;
    return(Address);
}

static uint16 ppuMemoryMirror(uint16 Address)
{
    ppu * Ppu = GlobalPpu;
    
    if(Address >= 0x4000) // Over half of the memory map is mirrored
        Address = Address % 0x4000; 

    if(0x3F20 <= Address && Address < 0x4000)
        Address = (Address % 0x20) + 0x3F00;
        
    if(0x3F00 <= Address && Address < 0x3F20) // Palette
    {
        if(Address == 0x3F10)
            Address = 0x3F00;
        if(Address == 0x3F14)
            Address = 0x3F04;
        if(Address == 0x3F18)
            Address = 0x3F08;
        if(Address == 0x3F1C)
            Address = 0x3F0C;
    }
   
    // NOTE: Nametable Mirroring. Controlled by Cartridge
    if(0x3000 <= Address && Address < 0x3F00) // This first as it maps to the nametable range
        Address = (Address % 0x0F00) + 0x2000;

    return Address;
}

static uint8 readCpu8(uint16 Address, cpu *Cpu)
{
    Address = cpuMemoryMirror(Address);
        
    if((0x2000 <= Address && Address < 0x2008) ||
       (Address == 0x4014))
    {
        // TODO: Catch up the ppu before we read. Depending on how many cycles has elapsed.
        
        return readPpuRegister(Address);
    }
   
    uint8 Value = read8(Address, Cpu->MemoryBase);
    
    // Input
    if(Address == 0x4016 || Address == 0x4017)
    {
        if( !Cpu->PadStrobe )
        {
            if(Address == 0x4016)
                Cpu->Pad1CurrentButton = ++(Cpu->Pad1CurrentButton);
            else
                Cpu->Pad2CurrentButton = ++(Cpu->Pad2CurrentButton);
        }
        
        uint16 InputAddress;
        uint8 BtnValue;
        if(Address == 0x4016)
        {
            InputAddress = 0x4016;
            BtnValue = Cpu->InputPad1.buttons[Cpu->Pad1CurrentButton] & 1;
        }
        else
        {
            InputAddress = 0x4017;
            BtnValue = Cpu->InputPad2.buttons[Cpu->Pad2CurrentButton] & 1;
        }

        uint8 CurrentValue = read8(InputAddress, Cpu->MemoryBase);
        uint8 NewValue = (CurrentValue & 0xFE) | BtnValue;
        write8(NewValue, InputAddress, Cpu->MemoryBase);
    }
    
    return(Value);
}

static uint16 readCpu16(uint16 Address, cpu * Cpu)
{
    // NOTE: Little Endian
    uint8 LowByte = readCpu8(Address, Cpu);
    uint8 HighByte = readCpu8(Address+1, Cpu);
        
    uint16 NewAddress = (HighByte << 8) | LowByte;
    return(NewAddress);
}

static uint16 bugReadCpu16(uint16 Address, cpu * Cpu)
{
    // NOTE: This is a bug in the nes 6502 that will wrap the value instead of going to new page.
    //       Only happens with indirect addressing.
    
    uint8 LowByte = readCpu8(Address, Cpu);
    uint16 Byte2Adrs = (Address & 0xFF00) | (uint16)((uint8)(Address + 1));
    uint8 HighByte = readCpu8(Byte2Adrs, Cpu);
        
    uint16 NewAddress = (HighByte << 8) | LowByte;
    return(NewAddress);
}

static void writeCpu8(uint8 Byte, uint16 Address, cpu *Cpu)
{
    Address = cpuMemoryMirror(Address);


    if((0x2000 <= Address && Address < 0x2008) ||
       (Address == 0x4014))
    {
        writePpuRegister(Byte, Address);
    }
    
    write8(Byte, Address, Cpu->MemoryBase);
    
    // Input
    if(Address == 0x4016 || Address == 0x4017)
    {
        uint8 Reg1Value = read8(0x4016, Cpu->MemoryBase);
        uint8 Reg2Value = read8(0x4017, Cpu->MemoryBase);

        uint8 Bit0 = (Reg1Value | Reg2Value) & 1;

        if(Bit0 == 0)
        {
            if(Cpu->PadStrobe)
            {
                Cpu->Pad1CurrentButton = Cpu->Pad2CurrentButton = input::B_A;
            }
            Cpu->PadStrobe = false;
        }
        else if(Bit0 == 1)
        {
            Cpu->PadStrobe = true;
        }        

        uint8 BtnValue = Cpu->InputPad1.buttons[Cpu->Pad1CurrentButton] & 1;
        write8(BtnValue, 0x4016, Cpu->MemoryBase);

        BtnValue = Cpu->InputPad2.buttons[Cpu->Pad2CurrentButton] & 1;
        write8(BtnValue, 0x4017, Cpu->MemoryBase);
    }

    // Mapper
    if(Address >= 0x8000)
    {
        Cpu->MapperWrite = true;
        Cpu->MapperReg = Byte;
        Cpu->MapperWriteAddress = Address;
    }
}

static uint8 * getNametableBank(uint16 Address, ppu *Ppu)
{
    uint8 * Result = 0;

    switch(Ppu->MirrorType)
    {
        case SINGLE_SCREEN_BANK_A:
        {
            Result = Ppu->NametableBankA;
            break;
        }
        case SINGLE_SCREEN_BANK_B:
        {
            Result = Ppu->NametableBankB;
            break;
        }
        case VERTICAL_MIRROR:
        {
            if(Address < 0x2400 || (0x2800 <= Address && Address < 0x2C00) )
            {
                Result = Ppu->NametableBankA;
            }
            else
            {
                Result = Ppu->NametableBankB;
            }
            break;
        }
        case HORIZONTAL_MIRROR:
        {
            if(Address < 0x2800)
            {
                Result = Ppu->NametableBankA;
            }
            else
            {
                Result = Ppu->NametableBankB;
            } 
            break;
        }
        case FOUR_SCREEN_MIRROR:
        {
            Assert(0);
            if(Address < 0x2400)
            {
                Result = Ppu->NametableBankA;
            }
            else if(Address < 0x2800)
            {
                Result = Ppu->NametableBankB;
            }
            else if(Address < 0x2C00)
            {
                Result = Ppu->NametableBankC;
            }
            else
            {
                Result = Ppu->NametableBankD;
            }
            break;
        }
    }
    
    return Result;
}

static uint8 readNametable(uint16 Address, ppu *Ppu)
{
    uint8 Result = 0;
    
    uint8 *Nametable = getNametableBank(Address, Ppu);
    Result = Nametable[Address % 0x400];
    
    return Result;
}

static void writeNametable(uint8 Byte, uint16 Address, ppu *Ppu)
{
    uint8 *Nametable = getNametableBank(Address, Ppu);
    Nametable[Address % 0x400] = Byte;;
}

static uint8 readPpu8(uint16 Address, ppu *Ppu)
{
    uint8 Result;
    
    Address = ppuMemoryMirror(Address);
            
    if(Address == 0x3F04 || Address == 0x3F08 || Address == 0x3F0C ||
       Address == 0x3F14 || Address == 0x3F18 || Address == 0x3F1C)
        Address = 0x3F00;

    // If address in nametable range. Then map to the current mirror state and return
    if(0x2000 <= Address && Address < 0x3000)
    {
        Result = readNametable(Address, Ppu);
    }
    else
    {
        Result = read8(Address, Ppu->MemoryBase);
    }
    return(Result);
}
static void writePpu8(uint8 Byte, uint16 Address, ppu *Ppu)
{    
    Address = ppuMemoryMirror(Address);

    if(0x2000 <= Address && Address < 0x3000)
    {
        writeNametable(Byte, Address, Ppu);
    }
    else
    {
        write8(Byte, Address, Ppu->MemoryBase);
    }
}


static void writePpuRegister(uint8 Byte, uint16 Address)
{
    ppu * Ppu = GlobalPpu;
    
    Ppu->OpenBus = Byte;
    
    switch(Address)
    {
        case 0x2000:
        {
            Ppu->NametableBase = Byte & 3;
            Ppu->VRamIO.TempVRamAdrs &= ~0xC00;
            Ppu->VRamIO.TempVRamAdrs |= (Byte & 3) << 10;            
            Ppu->VRamIncrement = ((Byte & 4) != 0) ? 32 : 1;
            Ppu->SPRTPattenBase = ((Byte & 8) != 0) ? 0x1000 : 0;
            Ppu->BGPatternBase = ((Byte & 16) != 0) ? 0x1000 : 0;
            Ppu->SpriteSize8x16 = ((Byte & 32) != 0);
            Ppu->PpuSlave = ((Byte & 64) != 0);
            Ppu->GenerateNMI = ((Byte & 128) != 0);


            /*
            if(Ppu->Scanline == 261 && Ppu->ScanlineCycle == 0)
            {
                NmiTriggered = false;
            }
            else if(Ppu->GenerateNMI && !Ppu->SupressNmi && Ppu->VerticalBlank && !NmiTriggered)
            {
                TriggerNmi = Ppu->GenerateNMI;
            }
            else if(!Ppu->GenerateNMI && Ppu->Scanline == 241 && (Ppu->ScanlineCycle == 1  || Ppu->ScanlineCycle == 2))
            {
                TriggerNmi = false;
                GlobalCpu->StartNmi = false;
                }*/

            //NmiTriggered = Ppu->GenerateNMI;
            break;
        }
        case 0x2001:
        {
            Ppu->GreyScale           = ((Byte & 1) != 0);
            Ppu->ShowBGLeft8Pixels   = ((Byte & 2) != 0);
            Ppu->ShowSPRTLeft8Pixels = ((Byte & 4) != 0);
            Ppu->ShowBackground      = ((Byte & 8) != 0);
            Ppu->ShowSprites         = ((Byte & 16) != 0);
            Ppu->EmphasizeRed        = ((Byte & 32) != 0);
            Ppu->EmphasizeGreen      = ((Byte & 64) != 0);
            Ppu->EmphasizeBlue       = ((Byte & 128) != 0);
            break;
        }
        case 0x2003:
        {
            Ppu->OamAddress = Byte;
            break;
        }
        case 0x2004:
        {
            // If Writing OAM Data while rendering, then a glitch increments it by 4 instead
            if(Ppu->Scanline < 240 || Ppu->Scanline == 261 || Ppu->ShowBackground || Ppu->ShowSprites)
            {
                Ppu->OamAddress += 4;
            }
            else
            {
                /*
                if( ( OAMADDR & 0x03 ) == 0x02 )
                    b&=0xe3;
                map.ppuwriteoam(Byte.toUnsignedInt(OAMADDR), b); TODO: Look at the example code
                */
                Ppu->Oam[Ppu->OamAddress] = Byte;
                Ppu->OamAddress++;
            }
            break;
        }
        case 0x2005:
        {
            if(Ppu->VRamIO.LatchWrite == 0)
            {
                Ppu->VRamIO.FineX = Byte & 7; // Bit 0,1, and 2 are fine X
                Ppu->VRamIO.TempVRamAdrs &= ~(0x001F); // Clear Bits
                Ppu->VRamIO.TempVRamAdrs |= ((uint16)Byte) >> 3;
                Ppu->VRamIO.LatchWrite = 1;
            }
            else
            {
                Ppu->VRamIO.TempVRamAdrs &= ~(0x73E0); // Clear Bits
                Ppu->VRamIO.TempVRamAdrs |= ((uint16)(Byte & 0x7)) << 12; // Set fine scroll Y, bits 0-2 set bit 12-14
                Ppu->VRamIO.TempVRamAdrs |= ((uint16)(Byte & 0xF8)) << 2; // Set coarse Y, bits 3-7 set bit 5-9
                Ppu->VRamIO.LatchWrite = 0;
            }
                
            break;
        }
        case 0x2006:
        {
            if(Ppu->VRamIO.LatchWrite == 0)
            {
                Ppu->VRamIO.TempVRamAdrs &= 0xC0FF; // Clear Bits About to be set 
                Ppu->VRamIO.TempVRamAdrs |= ((uint16)(Byte & 0x003F)) << 8;
                Ppu->VRamIO.TempVRamAdrs &= 0x3FFF; // Clear 14th bit 
                Ppu->VRamIO.LatchWrite = 1;
            }
            else
            { 
                Ppu->VRamIO.TempVRamAdrs &= 0x7F00; // Clear low byte
                Ppu->VRamIO.TempVRamAdrs |= (uint16)(Byte & 0x00FF); 
                Ppu->VRamIO.VRamAdrs = Ppu->VRamIO.TempVRamAdrs;
                Ppu->VRamIO.LatchWrite = 0;
            }
            
            break;
        }
        case 0x2007:
        {
            writePpu8(Byte, Ppu->VRamIO.VRamAdrs, Ppu);

            if( !(Ppu->ShowBackground || Ppu->ShowSprites) ||
                (Ppu->Scanline > 240 && Ppu->Scanline <= 260) )
            {            
                Ppu->VRamIO.VRamAdrs += Ppu->VRamIncrement;
            }
            break;
        }
        case 0x4014:
        {
            // NOTE: OAM DMA Write
            for(uint16 index = Ppu->OamAddress; index < OAM_SIZE; ++index)
            {
                uint16 NewAddress = (Byte << 8) | index; 
                OamData[index] = read8(NewAddress, GlobalCpu->MemoryBase);
            }            
            break;
        }
    }
}

static uint8 readPpuRegister(uint16 Address)
{
    ppu * Ppu = GlobalPpu;
    
    uint8 Byte = 0;
    
    switch(Address)
    {
        case 0x2002:
        {
            // One before vbl set, vbl is clear and does not get set the next cycle
            // On vbl set or one after, clears it, and supresses nmi for that frame?
            // Two or more cycles after, reads normally,             
            if( Ppu->Scanline == 241 && (Ppu->ScanlineCycle == 1 || Ppu->ScanlineCycle == 1) )
            {
                Ppu->SupressVbl = true;
            }
/*            else if( Ppu->Scanline == 240 && (Ppu->ScanlineCycle == 339 || Ppu->ScanlineCycle == 340) )
            {
                // Do nothing?
                }*/
            else if( Ppu->Scanline == 241 && (Ppu->ScanlineCycle == 1 || Ppu->ScanlineCycle == 2 || Ppu->ScanlineCycle == 3))
            {
                Byte |= Ppu->VerticalBlank ? 0x80 : 0;
                Ppu->SupressNmi = true;
                TriggerNmi = false;
                GlobalCpu->StartNmi = false;
            }
            else
            {
                // Else if not anywhere near where vbl is set
                Byte |= Ppu->VerticalBlank ? 0x80 : 0; // Read normally
            }
            
            Ppu->VerticalBlank = false;
            
            Byte |= Ppu->Sprite0Hit ? 0x40 : 0;
            Byte |= Ppu->SpriteOverflow ? 0x20 : 0;
            Byte |= (Ppu->OpenBus & 0x1F); // Low 5 bits is the open bus
            
            Ppu->VRamIO.LatchWrite = 0;
            NmiTriggered = false;
            Ppu->OpenBus = Byte;
            break;
        }
        case 0x2004:
        {
            Ppu->OpenBus = Ppu->Oam[Ppu->OamAddress];
            break;
        }
        case 0x2007:
        {
            bool32 OnPalette = !((Ppu->VRamIO.VRamAdrs&0x3FFF) < 0x3F00);

            if(OnPalette)
            {
                Ppu->VRamDataBuffer = readPpu8(Ppu->VRamIO.VRamAdrs-0x1000, Ppu);
                Byte = readPpu8(Ppu->VRamIO.VRamAdrs, Ppu);

                // Pulled from nes dev forum
                Byte &= 0x3F;
                Byte |= Ppu->OpenBus & 0xC0;
            }
            else
            {
                Byte = Ppu->VRamDataBuffer;
                Ppu->VRamDataBuffer = readPpu8(Ppu->VRamIO.VRamAdrs, Ppu);
            }

            if( !(Ppu->ShowBackground || Ppu->ShowSprites) ||
                (Ppu->Scanline > 240 && Ppu->Scanline <= 260) )
            {            
                Ppu->VRamIO.VRamAdrs += Ppu->VRamIncrement;
            }
                        
            Ppu->OpenBus = Byte;
            break;
        }
        case 0x4014:
        {
            break;
        }
    }

    return Ppu->OpenBus;
}

