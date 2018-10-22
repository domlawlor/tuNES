/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

void NromInit(Cartridge *cartridge, Cpu *cpu, Ppu *ppu)
{
    u16 memPrgBank1 = 0x8000;
    u16 memPrgBank2 = 0xC000;

    u8 * bankToCpy1;
    u8 * bankToCpy2;
        
    if(cartridge->prgBankCount == 1)
    {
        bankToCpy1 = cartridge->prgData;
        bankToCpy2 = cartridge->prgData;
    }
    else if(cartridge->prgBankCount == 2)
    {
        bankToCpy1 = cartridge->prgData;
        bankToCpy2 = cartridge->prgData + Kilobytes(16);
    }
        
    MemoryCopy((u8 *)memPrgBank1 + cpu->memoryBase, bankToCpy1, Kilobytes(16));
    MemoryCopy((u8 *)memPrgBank2 + cpu->memoryBase, bankToCpy2, Kilobytes(16));

    // Map CHR Data to ppu
    if(cartridge->chrBankCount == 1)
    {
        MemoryCopy((u8 *)ppu->memoryBase, cartridge->chrData, Kilobytes(8));
    }
}

void Mmc1Init(Cartridge *cartridge, Cpu *cpu, Ppu *ppu)
{
    u16 memPrgBank1 = 0x8000;
    u16 memPrgBank2 = 0xC000;

    u8 * bankToCpy1 = cartridge->prgData;
    u8 * bankToCpy2 = cartridge->prgData + ((cartridge->prgBankCount - 1) * Kilobytes(16));
           
    MemoryCopy((u8 *)memPrgBank1 + cpu->memoryBase, bankToCpy1, Kilobytes(16));
    MemoryCopy((u8 *)memPrgBank2 + cpu->memoryBase, bankToCpy2, Kilobytes(16));
 }

void UnromInit(Cartridge *cartridge, Cpu *cpu, Ppu *ppu)
{
    u16 memPrgBank1 = 0x8000;
    u16 memPrgBank2 = 0xC000;

    u8 * bankToCpy1 = cartridge->prgData;
    u8 * bankToCpy2 = cartridge->prgData + ((cartridge->prgBankCount - 1) * Kilobytes(16));
           
    MemoryCopy((u8 *)memPrgBank1 + cpu->memoryBase, bankToCpy1, Kilobytes(16));
    MemoryCopy((u8 *)memPrgBank2 + cpu->memoryBase, bankToCpy2, Kilobytes(16));
}

void AxromInit(Cartridge *cartridge, Cpu *cpu, Ppu *ppu)
{
    u16 memoryPrgBank = 0x8000;
    u8 *bankToCpy = cartridge->prgData + ((cartridge->prgBankCount) * Kilobytes(16)) - Kilobytes(32);
    MemoryCopy((u8 *)memoryPrgBank + cpu->memoryBase, bankToCpy, Kilobytes(32));

    ppu->mirrorType = SINGLE_SCREEN_BANK_A;
}

void (*MapperInit[MapperTotal])(Cartridge *cartridge, Cpu *cpu, Ppu *ppu) =
{
    NromInit, Mmc1Init, UnromInit, 0, 0, 0, 0, AxromInit 
};

void NromUpdate(Nes *nes, u8 byteWritten, u16 address)
{
//    Assert(0);
}

void Mmc1Update(Nes *nes, u8 byteWritten, u16 address)
{
    Cartridge *cartridge = &nes->cartridge;
    Cpu *cpu = &nes->cpu;
    Ppu *ppu = &nes->ppu;
    
    u16 prgRomBank1 = 0x8000;
    u16 prgRomBank2 = 0xC000;

    b32 isLargePrg = (cartridge->prgBankCount > 16);
    b32 isLargeChr = (cartridge->chrBankCount > 1);

    // TODO: Figure a way to deal with these static values
    static u8 prgRomMode = 0x3; // Initially starts here(control register is 0xC)
    static b32 chr8KbMode;
    
    b32 isClearBitSet = (byteWritten & (1 << 7)) != 0;
    if(isClearBitSet)
    {
        cartridge->mapperWriteCount = 0;
        byteWritten = 0;
        cartridge->mapperInternalReg = 0;

        u8 prevprgRomMode = prgRomMode;
        prgRomMode = 0x3;

        if(prevprgRomMode != prgRomMode) // TODO: Update the banks
        {
            Assert(0);
        }
    }
    else
    {
        ++cartridge->mapperWriteCount;

        // Shifted where the first write is the least significant, last write is most significant
        cartridge->mapperInternalReg = (cartridge->mapperInternalReg >> 1);
        cartridge->mapperInternalReg = (cartridge->mapperInternalReg & 0xF); // clear 5-bit
        cartridge->mapperInternalReg |= ((byteWritten & 1) << 4); 
                
        if(cartridge->mapperWriteCount == 5) // On 5th write
        {
            u8 dataReg = cartridge->mapperInternalReg;

            // TODO: Potentially change all address bounds checks to this style?
            b32 bit13Set = (address & (1 << 13)) != 0;
            b32 bit14Set = (address & (1 << 14)) != 0;

            if(!bit13Set && !bit14Set)     // Control Reg
            {
                u8 mirror = dataReg & 3;
                if(mirror == 0)
                    ppu->mirrorType = SINGLE_SCREEN_BANK_A;
                if(mirror == 1)
                    ppu->mirrorType = SINGLE_SCREEN_BANK_B;
                if(mirror == 2)
                    ppu->mirrorType = VERTICAL_MIRROR;
                if(mirror == 3)
                    ppu->mirrorType = HORIZONTAL_MIRROR;

                u8 PrevprgRomMode = prgRomMode;
                prgRomMode = (dataReg & 0xC) >> 2;

                if(PrevprgRomMode != prgRomMode) // TODO: Update the banks
                {
                    Assert(0);
                }
                    
                chr8KbMode = ((dataReg & 0x10) != 0);
            }
            else if(bit13Set && !bit14Set) // CHR Bank 0
            {
                u8 sizeToCpy = 0;
                    
                if(chr8KbMode) // 8kb mode Low bit ignored
                {
					dataReg = dataReg >> 1;
					sizeToCpy = (u8)Kilobytes(8);
                }
				else
				{
					sizeToCpy = (u8)Kilobytes(4);
				}

                // TODO: We shouldn't need to copy the memory to another place.
                //       Bank switch is a direct line to memory, so it should just change a pointer
                    
                // If CHR bank count is 0, then it uses RAM?? TODO: Check
                if(cartridge->chrBankCount > 0) {
                    u8 * bankToCpy = cartridge->chrData + (dataReg * sizeToCpy);
                    MemoryCopy((u8 *)ppu->memoryBase, bankToCpy, sizeToCpy);
                }
            }
            else if(!bit13Set && bit14Set) // CHR Bank 1
            {
                if(!chr8KbMode && cartridge->chrBankCount > 0)
                {
                    u8 * bankToCpy = cartridge->chrData + (dataReg * Kilobytes(4));              
                    MemoryCopy((u8 *)ppu->memoryBase + 0x1000, bankToCpy, Kilobytes(4));
                }
            }
            else if(bit13Set && bit14Set) // PRG bank
            {
                u8 * bankToCpy;
                if(prgRomMode == 0 || prgRomMode == 1) // 32kib Mode
                {
                    dataReg = dataReg >> 1;
                    bankToCpy = cartridge->prgData + (dataReg * Kilobytes(32));              
                    MemoryCopy((u8 *)cpu->memoryBase + 0x8000, bankToCpy, Kilobytes(32));
                }
                else if(prgRomMode == 2) // 16kb fixed low bank - swap high bank
                {
                    bankToCpy = cartridge->prgData + (dataReg * Kilobytes(16));              
                    MemoryCopy((u8 *)cpu->memoryBase + 0xC000, bankToCpy, Kilobytes(16));
                        
                }
                else if(prgRomMode == 3) // 16kb fixed high bank - swap low bank
                {
                    dataReg &= 0xF;
                    Assert(dataReg < cartridge->prgBankCount);
                        
                    bankToCpy = cartridge->prgData + (dataReg * Kilobytes(16));              
                    MemoryCopy((u8 *)cpu->memoryBase + 0x8000, bankToCpy, Kilobytes(16));
                }
            }
                
            cartridge->mapperWriteCount = 0;
            cartridge->mapperInternalReg = 0;
        }
    }
}

void UnromUpdate(Nes *nes, u8 byteWritten, u16 address)
{
    Cartridge *cartridge = &nes->cartridge;
    Cpu *cpu = &nes->cpu;
    
    u16 memPrgBank1 = 0x8000;
    u8 bankNumber = byteWritten;
    
    u8 * bankToCpy = cartridge->prgData + (bankNumber * Kilobytes(16));
    MemoryCopy((u8 *)memPrgBank1 + cpu->memoryBase, bankToCpy, Kilobytes(16));
}

void AxromUpdate(Nes *nes, u8 byteWritten, u16 address)
{
    Cartridge *cartridge = &nes->cartridge;
    Cpu *cpu = &nes->cpu;
    Ppu *ppu = &nes->ppu;
        
    u16 memoryPrgBank = 0x8000;
    
    u8 selectedBank = byteWritten & 7;
    u8 *bankToCpy = cartridge->prgData + (selectedBank * Kilobytes(32));
    
    MemoryCopy((u8 *)memoryPrgBank + cpu->memoryBase, bankToCpy, Kilobytes(32));

    // Nametable Single Screen bank select
    if(byteWritten & 0x10)
    {
        ppu->mirrorType = SINGLE_SCREEN_BANK_B;   
    }
    else
    {
        ppu->mirrorType = SINGLE_SCREEN_BANK_A;
    }
}

void (*MapperUpdate[MapperTotal])(Nes *nes, u8 byteWritten, u16 address) =
{
    NromUpdate, Mmc1Update, UnromUpdate, 0, 0, 0, 0, AxromUpdate 
};
