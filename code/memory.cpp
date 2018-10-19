
#include "nes.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"

static u8 ReadPpuRegister(u16 address);
static void WritePpuRegister(u8 Byte, u16 address);
static u8 ReadApuRegister(u16 address);
static void WriteApuRegister(u8 byte, u16 address);

// TODO: Forward declarations, is this the best idea?
static void RunPpu(Ppu *ppu, u16 clocksToRun);

static void RunPpuCatchup(u8 clocksIntoCurrentOp)
{
    // TODO: Find a better way to get Global values?
    Cpu *cpu = &globalNes->cpu;
    Ppu *ppu = &globalNes->ppu;
    //ppu *Apu = &GlobalNes->Apu;
    
    // New ClocksInto Op should be minus 1. Because we send in what
    // cycle we want to catch up too. We don't want to run that cycle
    // yet. Just one behind it
    u16 newClocks = (clocksIntoCurrentOp - 1) - cpu->lastClocksIntoOp;
    
    // Add the clocks already elapsed in Op.
    u16 clocksToRun = cpu->catchupClocks + newClocks;

    u16 ppuClocksToRun = clocksToRun * 3;

    RunPpu(ppu, ppuClocksToRun);
        
    cpu->catchupClocks = 0;
    cpu->lastClocksIntoOp = (clocksIntoCurrentOp - 1);
    Assert((clocksIntoCurrentOp-1) >= 0);
}

static void Write8(u8 byte, u16 address, u64 memoryOffset)
{   
    u8 *newAddress = (u8 *)(address + memoryOffset);
    *newAddress = byte;
}

static u8 Read8(u16 address, u64 memoryOffset)
{
    u8 *newAddress = (u8 *)(address + memoryOffset);
    u8 value = *newAddress;
    return(value);
}

static u8 Read8(u16 address, u64 memoryOffset, u8 currentCycle)
{
    /*
    if((0x2000 <= address && address <= 0x2007) || address == 0x4014)
    {
        runPpuCatchup(CurrentCycle);
    }
    */

    return Read8(address, memoryOffset);
}

static u16 CpuMemoryMirror(u16 address)
{
    // NOTE: Mirrors the address for the 2kb ram 
    if(0x0800 <= address && address < 0x2000)
		address = (address % 0x0800);
    // NOTE: Mirror for PPU Registers
    if(0x2008 <= address && address < 0x4000)
		address = (address % (0x2008 - 0x2000)) + 0x2000;
    return(address);
}

static u8 readCpu8(u16 address, Cpu *cpu)
{
	address = CpuMemoryMirror(address);
        
    if((0x2000 <= address && address < 0x2008) ||
       (address == 0x4014))
    {
        return ReadPpuRegister(address);
    }
    else if((0x4000 <= address && address <= 0x4013) ||
		address == 0x4015 || address == 0x4017)
    {
        return ReadApuRegister(address);
    }
   
    u8 value = Read8(address, cpu->memoryBase);
    
    // Input
    if(address == 0x4016 || address == 0x4017)
    {
        if(!cpu->padStrobe)
		{
            if(address == 0x4016)
            {
                cpu->pad1CurrentButton = ++(cpu->pad1CurrentButton);
            }
            else
            {
                cpu->pad2CurrentButton = ++(cpu->pad2CurrentButton);
            }
        }
        
        u16 inputAddress;
        u8 btnValue;

        if(address == 0x4016)
        {
            inputAddress = 0x4016;
            btnValue = cpu->inputPad1.buttons[cpu->pad1CurrentButton] & 1;
        }
        else
        {
            inputAddress = 0x4017;
            btnValue = cpu->inputPad2.buttons[cpu->pad2CurrentButton] & 1;
        }

        u8 currentValue = Read8(inputAddress, cpu->memoryBase);
        u8 newValue = (currentValue & 0xFE) | btnValue;
        Write8(newValue, inputAddress, cpu->memoryBase);
    }
    
    return(value);
}

static u8 ReadCpu8(u16 address, Cpu *cpu, u8 currentCycle)
{
    if((0x2000 <= address && address <= 0x2007) || address == 0x4014)
    {
        RunPpuCatchup(currentCycle);
    }
    
    return ReadCpu8(address, cpu);
}

static u16 ReadCpu16(u16 address, Cpu * cpu)
{
    // NOTE: Little Endian
    u8 lowByte = ReadCpu8(address, cpu);
    u8 highByte = ReadCpu8(address +1, cpu);
        
    u16 newAddress = (highByte << 8) | lowByte;
    return(newAddress);
}

static void WriteCpu8(u8 byte, u16 address, Cpu *cpu)
{
	address = cpuMemoryMirror(address);

    if((0x2000 <= address && address < 0x2008) ||
       (address == 0x4014))
    {
        writePpuRegister(Byte, address);
    }
    else if((0x4000 <= address && address <= 0x4013) ||
            address == 0x4015 || address == 0x4017)
    {
        writeApuRegister(Byte, address);
    }
    
    
    write8(Byte, address, Cpu->MemoryBase);
    
    // Input
    if(address == 0x4016 || address == 0x4017)
    {
        u8 Reg1Value = read8(0x4016, Cpu->MemoryBase);
        u8 Reg2Value = read8(0x4017, Cpu->MemoryBase);

        u8 Bit0 = (Reg1Value | Reg2Value) & 1;

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

        u8 BtnValue = Cpu->InputPad1.Buttons[Cpu->Pad1CurrentButton] & 1;
        write8(BtnValue, 0x4016, Cpu->MemoryBase);

        BtnValue = Cpu->InputPad2.Buttons[Cpu->Pad2CurrentButton] & 1;
        write8(BtnValue, 0x4017, Cpu->MemoryBase);
    }

    // Mapper
    if(address >= 0x8000)
    {
        mapperUpdate[GlobalNes->Cartridge.MapperNum](GlobalNes, Byte, address);
    }
}

static void writeCpu8(u8 Byte, u16 address, cpu *Cpu, u8 CurrentCycle)
{
    if((0x2000 <= address && address <= 0x2007) || address == 0x4014)
    {
        runPpuCatchup(CurrentCycle);
    }
    writeCpu8(Byte, address, Cpu);
}

static u8 * getNametableBank(u16 address, ppu *Ppu)
{
    u8 * Result = 0;

    switch(Ppu->mirrorType)
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
            if(address < 0x2400 || (0x2800 <= address && address < 0x2C00) )
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
            if(address < 0x2800)
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
            if(address < 0x2400)
            {
                Result = Ppu->NametableBankA;
            }
            else if(address < 0x2800)
            {
                Result = Ppu->NametableBankB;
            }
            else if(address < 0x2C00)
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

static u8 readNametable(u16 address, ppu *Ppu)
{
    u8 Result = 0;
    
    u8 *Nametable = getNametableBank(address, Ppu);
    Result = Nametable[address % 0x400];
    
    return Result;
}

static void writeNametable(u8 Byte, u16 address, ppu *Ppu)
{
    u8 *Nametable = getNametableBank(address, Ppu);
    Nametable[address % 0x400] = Byte;
}

static u16 ppuMemoryMirror(u16 address)
{
    ppu * Ppu = &GlobalNes->Ppu;
    
    if(address >= 0x4000) // Over half of the memory map is mirrored
        address = address % 0x4000; 

    if(0x3F20 <= address && address < 0x4000)
        address = (address % 0x20) + 0x3F00;
        
    if(0x3F00 <= address && address < 0x3F20) // Palette
    {
        if(address == 0x3F10)
            address = 0x3F00;
        if(address == 0x3F14)
            address = 0x3F04;
        if(address == 0x3F18)
            address = 0x3F08;
        if(address == 0x3F1C)
            address = 0x3F0C;
    }
   
    // NOTE: Nametable Mirroring. Controlled by Cartridge
    if(0x3000 <= address && address < 0x3F00) // This first as it maps to the nametable range
        address -= 0x1000;

    return address;
}

static u8 readPpu8(u16 address, ppu *Ppu)
{
    u8 Result;
    
    address = ppuMemoryMirror(address);
            
    if((Ppu->ShowBackground || Ppu->ShowSprites) &&
       (address == 0x3F04 || address == 0x3F08 || address == 0x3F0C))
    {
        address = 0x3F00;
    }
    
    // If address in nametable range. Then map to the current mirror state and return
    if(0x2000 <= address && address < 0x3000)
    {
        Result = readNametable(address, Ppu);
    }
    else
    {
        Result = read8(address, Ppu->MemoryBase);
    }
    return(Result);
}

static void writePpu8(u8 Byte, u16 address, ppu *Ppu)
{    
    address = ppuMemoryMirror(address);
    
    if(0x2000 <= address && address < 0x3000)
    {
        writeNametable(Byte, address, Ppu);
    }
    else
    {
        write8(Byte, address, Ppu->MemoryBase);
    }
}

static void writePpuRegister(u8 Byte, u16 address)
{
    ppu * Ppu = &GlobalNes->Ppu;
    
    GlobalOpenBus = Byte;
    
    switch(address)
    {
        case 0x2000:
        {
            Ppu->NametableBase  = Byte & 3;            
            Ppu->VRamIncrement  = ((Byte & 4) != 0) ? 32 : 1;
            Ppu->SPRTPattenBase = ((Byte & 8) != 0) ? 0x1000 : 0;
            Ppu->BGPatternBase  = ((Byte & 16) != 0) ? 0x1000 : 0;
            Ppu->SpriteSize8x16 = ((Byte & 32) != 0);
            Ppu->PpuSlave       = ((Byte & 64) != 0);
            Ppu->GenerateNMI    = ((Byte & 128) != 0);

            Ppu->TempVRamAdrs &= ~0xC00;
            Ppu->TempVRamAdrs |= (Ppu->NametableBase) << 10;
            
            // Nmi On Timing
            if( !(Ppu->Scanline == 261 && Ppu->ScanlineCycle == 1) )
            {
                setNmi(Ppu->GenerateNMI && Ppu->VerticalBlank);
            }

            // Nmi off timing test
            if(Ppu->Scanline == 241 && (Ppu->ScanlineCycle == 4))
            {
                setNmi(true);
            }

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

            Ppu->RenderingEnabled = (Ppu->ShowBackground || Ppu->ShowSprites);
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
                Ppu->Oam[Ppu->OamAddress] = Byte;
                Ppu->OamAddress++;
            }
            break;
        }
        case 0x2005:
        {
            if(Ppu->LatchWrite == 0)
            {
                Ppu->FineX = Byte & 7; // Bit 0,1, and 2 are fine X
                Ppu->TempVRamAdrs &= ~(0x001F); // Clear Bits
                Ppu->TempVRamAdrs |= ((u16)Byte) >> 3;
                Ppu->LatchWrite = 1;
            }
            else
            {
                Ppu->TempVRamAdrs &= ~(0x73E0); // Clear Bits
                Ppu->TempVRamAdrs |= ((u16)(Byte & 0x7)) << 12; // Set fine scroll Y, bits 0-2 set bit 12-14
                Ppu->TempVRamAdrs |= ((u16)(Byte & 0xF8)) << 2; // Set coarse Y, bits 3-7 set bit 5-9
                Ppu->LatchWrite = 0;
            }
                
            break;
        }
        case 0x2006:
        {
            if(Ppu->LatchWrite == 0)
            {
                Ppu->TempVRamAdrs &= 0xC0FF; // Clear Bits About to be set 
                Ppu->TempVRamAdrs |= ((u16)(Byte & 0x003F)) << 8;
                Ppu->TempVRamAdrs &= 0x3FFF; // Clear 14th bit 
                Ppu->LatchWrite = 1;
            }
            else
            { 
                Ppu->TempVRamAdrs &= 0x7F00; // Clear low byte
                Ppu->TempVRamAdrs |= (u16)(Byte & 0x00FF); 
                Ppu->VRamAdrs = Ppu->TempVRamAdrs;
                Ppu->LatchWrite = 0;
            }
            
            break;
        }
        case 0x2007:
        {
            writePpu8(Byte, Ppu->VRamAdrs, Ppu);

            if( !(Ppu->ShowBackground || Ppu->ShowSprites) ||
                (Ppu->Scanline > 240 && Ppu->Scanline <= 260) )
            {            
                Ppu->VRamAdrs += Ppu->VRamIncrement;
            }
            break;
        }
        case 0x4014:
        {
            // NOTE: OAM DMA Write
            // TODO: Happens over time!
            for(u16 ByteCount = 0; ByteCount < 256; ++ByteCount)
            {
                u16 NewAddress = (Byte << 8) | ByteCount;

                u8 Index = (Ppu->OamAddress + ByteCount);
                Ppu->Oam[Index] = read8(NewAddress, GlobalNes->Cpu.MemoryBase);
            }            
            break;
        }
    }
}

static u8 readPpuRegister(u16 address)
{
    ppu * Ppu = &GlobalNes->Ppu;
    
    u8 Byte = 0;
    
    switch(address)
    {
        case 0x2002:
        {
            // NOTE: Reading VBL one cycle before it is set, returns clear and supresses vbl
            if( !(Ppu->Scanline == 241 && (Ppu->ScanlineCycle == 1) ) )
                   
            {
                Byte |= Ppu->VerticalBlank ? 0x80 : 0;
                Ppu->SupressVbl = false;
            }
            else
            {
                Ppu->SupressVbl = true;
            }

            // NMI Supression
            if(Ppu->Scanline == 241 &&
               (Ppu->ScanlineCycle == 1 || Ppu->ScanlineCycle == 2 || Ppu->ScanlineCycle == 3))
            {
                setNmi(false);
                TriggerNmi = false;

                if(Ppu->ScanlineCycle == 1)
                    Ppu->SupressNmiSet = true;
            }
            
            Ppu->VerticalBlank = false;

            Byte |= Ppu->SpriteZeroHit ? 0x40 : 0;
            Byte |= Ppu->SpriteOverflow ? 0x20 : 0;
            Byte |= (GlobalOpenBus & 0x1F); // Low 5 bits is the open bus

            Ppu->LatchWrite = 0; // VRAM latch reset
            GlobalOpenBus = Byte;
            break;
        }
        case 0x2004:
        {
            GlobalOpenBus = Ppu->Oam[Ppu->OamAddress];
            break;
        }
        case 0x2007:
        {
            b32 OnPalette = !((Ppu->VRamAdrs & 0x3FFF) < 0x3F00);

            if(OnPalette)
            {
                Ppu->VRamDataBuffer = readPpu8(Ppu->VRamAdrs-0x1000, Ppu);
                Byte = readPpu8(Ppu->VRamAdrs, Ppu);

                // Pulled from nes dev forum
                Byte &= 0x3F;
                Byte |= GlobalOpenBus & 0xC0;
            }
            else
            {
                Byte = Ppu->VRamDataBuffer;
                Ppu->VRamDataBuffer = readPpu8(Ppu->VRamAdrs, Ppu);
            }

            if( !(Ppu->RenderingEnabled) ||
                (Ppu->Scanline > 240 && Ppu->Scanline <= 260) )
            {            
                Ppu->VRamAdrs += Ppu->VRamIncrement;
            }
            else
            {
                // TODO: Weird update of Vram, check ppu_scrolling on wiki
            }
                        
            GlobalOpenBus = Byte;
            break;
        }
        case 0x4014:
        {
            break;
        }
    }

    return(GlobalOpenBus);
}


u8 LengthCounterTable[] = {0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06,
                              0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E,
                              0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16,
                              0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E };

static void writeApuRegister(u8 Byte, u16 address)
{
    apu * Apu = &GlobalNes->Apu;
    
    GlobalOpenBus = Byte;
    
    switch(address)
    {
        case 0x4000:
        case 0x4004:
        {
            square *Square = (0x4000 <= address && address < 0x4004) ? &Apu->Square1 : &Apu->Square2;

            Square->DutyCycle         = ((Byte & 0xC0) >> 6);
            Square->LengthCounterHalt = ((Byte & 0x20) != 0);
            Square->EnvelopeDisable   = ((Byte & 0x10) != 0);
            Square->VolumePeriod      = (Byte & 0xF);
            break;
        }
        case 0x4001:
        case 0x4005:
        {
            square *Square = (0x4000 <= address && address < 0x4004) ? &Apu->Square1 : &Apu->Square2;

            Square->EnableSweep = ((Byte & 0x80) != 0);
            Square->SweepPeriod = ((Byte & 0x70) >> 4);
            Square->Negative    = ((Byte & 0x8) != 0);
            Square->ShiftCount  = (Byte & 0x7);

            Square->SweepReset = true;
            break;
        }
        case 0x4002:
        case 0x4006:
        {
            square *Square = (0x4000 <= address && address < 0x4004) ? &Apu->Square1 : &Apu->Square2;
            Square->PeriodLow = Byte;
            break;
        }
        case 0x4003:
        {
            if(Apu->Square1Enabled)
                Apu->Square1.LengthCounter = LengthCounterTable[((Byte & 0xF8) >> 3)];
            Apu->Square1.PeriodHigh = (Byte & 0x7);

            Apu->Square1.RestartEnv = true;
            break;
        }
        case 0x4007:
        {
            if(Apu->Square2Enabled)
                Apu->Square2.LengthCounter = LengthCounterTable[((Byte & 0xF8) >> 3)];
            Apu->Square2.PeriodHigh = (Byte & 0x7);

            Apu->Square2.RestartEnv = true;
            break;
        }
        case 0x4008:
        {
            Apu->Triangle.LinearCtrl    = ((Byte & 0x80) != 0);
            Apu->Triangle.LinearCounter = (Byte & 0x7F);
            break;
        }
        case 0x400A:
        {
            Apu->Triangle.PeriodLow = Byte;
            break;
        }
        case 0x400B:
        {
            if(Apu->TriangleEnabled)
                Apu->Triangle.LengthCounter = LengthCounterTable[((Byte & 0xF8) >> 3)];
            Apu->Triangle.PeriodHigh    = (Byte & 0x7);
            break;
        }
        case 0x400C:
        {
            Apu->Noise.LengthCounterHalt = ((Byte & 0x20) != 0);
            Apu->Noise.EnvelopeDisable   = ((Byte & 0x10) != 0);
            Apu->Noise.VolumePeriod      = (Byte & 0xF);
            break;
        }
        case 0x400E:
        {
            Apu->Noise.LoopNoise  = ((Byte & 0x80) != 0);
            Apu->Noise.LoopPeriod = (Byte & 0xF);
            break;
        }
        case 0x400F:
        {
            if(!Apu->NoiseEnabled)
                Apu->Noise.LengthCounter = LengthCounterTable[((Byte & 0xF8) >> 3)];
            Apu->Noise.RestartEnv = true; // TODO: Unsure if noise envelope is reset
            break;
        }
        case 0x4010:
        {
            Apu->Dmc.IRQEnable = ((Byte & 0x80) != 0);
            Apu->Dmc.Loop      = ((Byte & 0x40) != 0);
            Apu->Dmc.FreqIndex = (Byte & 0xF);
            break;
        }
        case 0x4011:
        {
            Apu->Dmc.LoadCounter = (Byte & 0x7F);
            break;
        }
        case 0x4012:
        {
            Apu->Dmc.SampleAddress = Byte;
            break;
        }
        case 0x4013:
        {
            Apu->Dmc.SampleLength = Byte;
            break;
        }
        case 0x4015:
        {
            Apu->DmcEnabled = ((Byte & 0x10) != 0);
            Apu->NoiseEnabled = ((Byte & 0x8) != 0);
            Apu->TriangleEnabled = ((Byte & 0x4) != 0);            
            Apu->Square2Enabled = ((Byte & 0x2) != 0);
            Apu->Square1Enabled = ((Byte & 0x1) != 0);
                        
            if(!Apu->DmcEnabled)
            {
                Apu->Dmc.SampleLength = 0;
            }
            if(!Apu->Square1Enabled)
            {
                Apu->Square1.LengthCounter = 0;
            }
            if(!Apu->Square2Enabled)
            {
                Apu->Square2.LengthCounter = 0;
            }
            if(!Apu->TriangleEnabled)
            {
                Apu->Triangle.LengthCounter = 0;
            }
            if(!Apu->NoiseEnabled)
            {
                Apu->Noise.LengthCounter = 0;
            }

            Apu->DmcInterrupt = false;
            
            // TODO: If DMC Bit is set, sample will only be restarted it bytes is 0. Else
            //       the remaining bytes will finish before the next sample is fetched
            
            break;
        }
        case 0x4017:
        {
            Apu->Mode       = ((Byte & 0x80) != 0);
            Apu->IRQInhibit = ((Byte & 0x40) != 0);

            Apu->FrameCounter = 0;

            // NOTE/TODO Writing to $4017 resets the frame counter and the quarter/half frame
            // triggers happen simultaneously, but only on "odd" cycles (and only after the
            // first "even" cycle after the write occurs) - thus, it happens either 2 or 3 cycles
            // after the write (i.e. on the 2nd or 3rd cycle of the next instruction).
            // After 2 or 3 clock cycles (depending on when the write is performed), the timer is reset.
            // Writing to $4017 with bit 7 set ($80) will immediately clock all of its controlled units at
            //the beginning of the 5-step sequence; with bit 7 clear, only the sequence is reset without
            // clocking any of its units.
            break;
        }        
    }
}

static u8 ReadApuRegister(u16 address)
{
    Apu * apu = &globalNes->apu;
    
    u8 byte = 0;
    
    switch(address)
    {
        case 0x4015:
        {
            byte |= (apu->dmcInterrupt != 0) ? 0x80 : 0;
            byte |= (apu->frameInterrupt != 0) ? 0x40 : 0;
					 
            byte |= (apu->dmc.sampleLength > 0) ? 0x10 : 0;
            byte |= (apu->noise.lengthCounter > 0) ? 0x08 : 0;
            byte |= (apu->triangle.lengthCounter > 0) ? 0x4 : 0;
            byte |= (apu->square2.lengthCounter > 0) ? 0x2 : 0;
            byte |= (apu->square1.lengthCounter > 0) ? 0x1 : 0;

            apu->frameInterrupt = false;
            
            // TODO: If an interrupt Flag was set the same moment as read,
            // it will be read as set and not be cleared
            /*if(0)
                ;
            */
            globalOpenBus = byte;
            break;
        }
    }
    
    return(globalOpenBus);
}


