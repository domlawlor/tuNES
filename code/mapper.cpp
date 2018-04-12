/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

void nromInit(cartridge *Cartridge, cpu *Cpu, ppu *Ppu)
{
    uint16 MemPrgBank1 = 0x8000;
    uint16 MemPrgBank2 = 0xC000;

    uint8 * BankToCpy1;
    uint8 * BankToCpy2;
        
    if(Cartridge->PrgBankCount == 1)
    {
        BankToCpy1 = Cartridge->PrgData;
        BankToCpy2 = Cartridge->PrgData;
    }
    else if(Cartridge->PrgBankCount == 2)
    {
        BankToCpy1 = Cartridge->PrgData;
        BankToCpy2 = Cartridge->PrgData + Kilobytes(16);
    }
        
    copyMemory((uint8 *)MemPrgBank1 + Cpu->MemoryBase, BankToCpy1, Kilobytes(16));
    copyMemory((uint8 *)MemPrgBank2 + Cpu->MemoryBase, BankToCpy2, Kilobytes(16));

    // Map CHR Data to Ppu
    if(Cartridge->ChrBankCount == 1)
    {
        copyMemory((uint8 *)Ppu->MemoryBase, Cartridge->ChrData, Kilobytes(8));
    }
}

void mmc1Init(cartridge *Cartridge, cpu *Cpu, ppu *Ppu)
{
    uint16 MemPrgBank1 = 0x8000;
    uint16 MemPrgBank2 = 0xC000;

    uint8 * BankToCpy1 = Cartridge->PrgData;
    uint8 * BankToCpy2 = Cartridge->PrgData + ((Cartridge->PrgBankCount - 1) * Kilobytes(16));
           
    copyMemory((uint8 *)MemPrgBank1 + Cpu->MemoryBase, BankToCpy1, Kilobytes(16));
    copyMemory((uint8 *)MemPrgBank2 + Cpu->MemoryBase, BankToCpy2, Kilobytes(16));
 }

void unromInit(cartridge *Cartridge, cpu *Cpu, ppu *Ppu)
{
    uint16 MemPrgBank1 = 0x8000;
    uint16 MemPrgBank2 = 0xC000;

    uint8 * BankToCpy1 = Cartridge->PrgData;
    uint8 * BankToCpy2 = Cartridge->PrgData + ((Cartridge->PrgBankCount - 1) * Kilobytes(16));
           
    copyMemory((uint8 *)MemPrgBank1 + Cpu->MemoryBase, BankToCpy1, Kilobytes(16));
    copyMemory((uint8 *)MemPrgBank2 + Cpu->MemoryBase, BankToCpy2, Kilobytes(16));
}

void axromInit(cartridge *Cartridge, cpu *Cpu, ppu *Ppu)
{
    uint16 MemoryPrgBank = 0x8000;
    uint8 *BankToCpy = Cartridge->PrgData + ((Cartridge->PrgBankCount) * Kilobytes(16)) - Kilobytes(32);
    copyMemory((uint8 *)MemoryPrgBank + Cpu->MemoryBase, BankToCpy, Kilobytes(32));

    Ppu->MirrorType = SINGLE_SCREEN_BANK_A;
}

void (*mapperInit[MAPPER_TOTAL])(cartridge *Cartridge, cpu *Cpu, ppu *Ppu) =
{
    nromInit, mmc1Init, unromInit, 0, 0, 0, 0, axromInit 
};

void nromUpdate(nes *Nes, uint8 ByteWritten, uint16 Address)
{
//    Assert(0);
}

void mmc1Update(nes *Nes, uint8 ByteWritten, uint16 Address)
{
    cartridge *Cartridge = &Nes->Cartridge;
    cpu *Cpu = &Nes->Cpu;
    ppu *Ppu = &Nes->Ppu;
    
    uint16 PrgRomBank1 = 0x8000;
    uint16 PrgRomBank2 = 0xC000;

    bool32 IsLargePrg = (Cartridge->PrgBankCount > 16);
    bool32 IsLargeChr = (Cartridge->ChrBankCount > 1);

    // TODO: Figure a way to deal with these static values
    static uint8 PrgRomMode = 0x3; // Initially starts here(control register is 0xC)
    static bool32 Chr8KbMode;
    
    bool32 IsClearBitSet = (ByteWritten & (1 << 7)) != 0;
    if(IsClearBitSet)
    {
        Cartridge->MapperWriteCount = 0;
        ByteWritten = 0;
        Cartridge->MapperInternalReg = 0;

        uint8 PrevPrgRomMode = PrgRomMode;
        PrgRomMode = 0x3;

        if(PrevPrgRomMode != PrgRomMode) // TODO: Update the banks
        {
            Assert(0);
        }
    }
    else
    {
        ++Cartridge->MapperWriteCount;

        // Shifted where the first write is the least significant, last write is most significant
        Cartridge->MapperInternalReg = (Cartridge->MapperInternalReg >> 1);
        Cartridge->MapperInternalReg = (Cartridge->MapperInternalReg & 0xF); // clear 5-bit
        Cartridge->MapperInternalReg |= ((ByteWritten & 1) << 4); 
                
        if(Cartridge->MapperWriteCount == 5) // On 5th write
        {
            uint8 DataReg = Cartridge->MapperInternalReg;

            // TODO: Potentially change all address bounds checks to this style?
            bool32 bit13Set = (Address & (1 << 13)) != 0;
            bool32 bit14Set = (Address & (1 << 14)) != 0;

            if(!bit13Set && !bit14Set)     // Control Reg
            {
                uint8 Mirror = DataReg & 3;
                if(Mirror == 0)
                    Ppu->MirrorType = SINGLE_SCREEN_BANK_A;
                if(Mirror == 1)
                    Ppu->MirrorType = SINGLE_SCREEN_BANK_B;
                if(Mirror == 2)
                    Ppu->MirrorType = VERTICAL_MIRROR;
                if(Mirror == 3)
                    Ppu->MirrorType = HORIZONTAL_MIRROR;

                uint8 PrevPrgRomMode = PrgRomMode;
                PrgRomMode = (DataReg & 0xC) >> 2;

                if(PrevPrgRomMode != PrgRomMode) // TODO: Update the banks
                {
                    Assert(0);
                }
                    
                Chr8KbMode = ((DataReg & 0x10) != 0);
            }
            else if(bit13Set && !bit14Set) // CHR Bank 0
            {
                uint8 SizeToCpy = 0;
                    
                if(Chr8KbMode) // 8kb mode Low bit ignored
                {
                    DataReg = DataReg >> 1;
                    SizeToCpy = Kilobytes(8);
                }
                else
                {
                    SizeToCpy = Kilobytes(4);
                }

                // TODO: We shouldn't need to copy the memory to another place.
                //       Bank switch is a direct line to memory, so it should just change a pointer
                    
                // If CHR bank count is 0, then it uses RAM?? TODO: Check
                if(Cartridge->ChrBankCount > 0) {
                    uint8 * BankToCpy = Cartridge->ChrData + (DataReg * SizeToCpy);              
                    copyMemory((uint8 *)Ppu->MemoryBase, BankToCpy, SizeToCpy);
                }
            }
            else if(!bit13Set && bit14Set) // CHR Bank 1
            {
                if(!Chr8KbMode && Cartridge->ChrBankCount > 0)
                {
                    uint8 * BankToCpy = Cartridge->ChrData + (DataReg * Kilobytes(4));              
                    copyMemory((uint8 *)Ppu->MemoryBase + 0x1000, BankToCpy, Kilobytes(4));
                }
            }
            else if(bit13Set && bit14Set) // PRG bank
            {
                uint8 * BankToCpy;
                if(PrgRomMode == 0 || PrgRomMode == 1) // 32kib Mode
                {
                    DataReg = DataReg >> 1;
                    BankToCpy = Cartridge->PrgData + (DataReg * Kilobytes(32));              
                    copyMemory((uint8 *)Cpu->MemoryBase + 0x8000, BankToCpy, Kilobytes(32));
                }
                else if(PrgRomMode == 2) // 16kb fixed low bank - swap highbank
                {
                    BankToCpy = Cartridge->PrgData + (DataReg * Kilobytes(16));              
                    copyMemory((uint8 *)Cpu->MemoryBase + 0xC000, BankToCpy, Kilobytes(16));
                        
                }
                else if(PrgRomMode == 3) // 16kb fixed high bank - swap lowbank
                {
                    DataReg &= 0xF;
                    Assert(DataReg < Cartridge->PrgBankCount);
                        
                    BankToCpy = Cartridge->PrgData + (DataReg * Kilobytes(16));              
                    copyMemory((uint8 *)Cpu->MemoryBase + 0x8000, BankToCpy, Kilobytes(16));
                }
            }
                
            Cartridge->MapperWriteCount = 0;
            Cartridge->MapperInternalReg = 0;
        }
    }
}

void unromUpdate(nes *Nes, uint8 ByteWritten, uint16 Address)
{
    cartridge *Cartridge = &Nes->Cartridge;
    cpu *Cpu = &Nes->Cpu;
    
    uint16 MemPrgBank1 = 0x8000;
    uint8 BankNumber = ByteWritten;
    
    uint8 * BankToCpy = Cartridge->PrgData + (BankNumber * Kilobytes(16));
    copyMemory((uint8 *)MemPrgBank1 + Cpu->MemoryBase, BankToCpy, Kilobytes(16));
}

void axromUpdate(nes *Nes, uint8 ByteWritten, uint16 Address)
{
    cartridge *Cartridge = &Nes->Cartridge;
    cpu *Cpu = &Nes->Cpu;
    ppu *Ppu = &Nes->Ppu;
        
    uint16 MemoryPrgBank = 0x8000;
    
    uint8 SelectedBank = ByteWritten & 7;
    uint8 *BankToCpy = Cartridge->PrgData + (SelectedBank * Kilobytes(32));
    
    copyMemory((uint8 *)MemoryPrgBank + Cpu->MemoryBase, BankToCpy, Kilobytes(32));

    // Nametable Single Screen bank select
    if(ByteWritten & 0x10)
    {
        Ppu->MirrorType = SINGLE_SCREEN_BANK_B;   
    }
    else
    {
        Ppu->MirrorType = SINGLE_SCREEN_BANK_A;
    }
}

void (*mapperUpdate[MAPPER_TOTAL])(nes *Nes, uint8 ByteWritten, uint16 Address) =
{
    nromUpdate, mmc1Update, unromUpdate, 0, 0, 0, 0, axromUpdate 
};
