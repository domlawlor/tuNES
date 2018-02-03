/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#include "palette.cpp"

static void drawPixel(ppu *Ppu, uint16 X, uint16 Y, uint8 *Colour)
{
    uint32 *CurrentPixel = (Ppu->BasePixel + (Y * PIXEL_WIDTH)) + X;
    *CurrentPixel  = ((Colour[0] << 16) | (Colour[1] << 8) | Colour[2]);
}

static void resetScrollHorz(vram_io *VRamIO)
{   
    VRamIO->VRamAdrs &= ~(0x041F);
    VRamIO->VRamAdrs |= (VRamIO->TempVRamAdrs & 0x041F);
}

static void resetScrollVert(vram_io *VRamIO)
{
    VRamIO->VRamAdrs = VRamIO->TempVRamAdrs;
}

static void scrollIncHorz(vram_io *Vram)
{
    //NOTE: Code take from nesdev wiki. Could be quicker??
    if ((Vram->VRamAdrs & 0x001F) == 31) // if coarse X == 31
    {
        Vram->VRamAdrs &= ~0x001F;  // coarse X = 0
        Vram->VRamAdrs ^= 0x0400;   // switch horizontal nametable
    }
    else
        Vram->VRamAdrs += 1;                // increment coarse X
}

static void scrollIncVert(vram_io *Vram)
{
    // NOTE: Code take from nesdev wiki. Could be quicker??
    if ((Vram->VRamAdrs & 0x7000) != 0x7000) // if fine Y < 7
    {
        Vram->VRamAdrs += 0x1000; // increment fine Y
    }
    else
    {
        Vram->VRamAdrs &= ~0x7000; // fine Y = 0        
        uint16 y = (Vram->VRamAdrs & 0x03E0) >> 5 ; // let y = coarse Y
    
        if (y == 29)
        {
            y = 0; // coarse Y = 0
            Vram->VRamAdrs ^= 0x0800; // switch vertical nametable
        }
        else if (y == 31)
        {
            y = 0; // coarse Y = 0, nametable not switched
        }
        else
        {
            y += 1; // increment coarse Y
        }
        Vram->VRamAdrs = (Vram->VRamAdrs & ~0x03E0) | (y << 5); // put coarse Y back into v
    }
}

static void loadFutureData(ppu *Ppu)
{                            
    uint8 Cycle = (Ppu->ScanlineCycle - 1) % 8;
    if(Cycle == 0) 
    {
        Ppu->LowPatternShiftReg = (Ppu->LowPatternShiftReg << 8) | Ppu->NextLowPattern;
        Ppu->HighPatternShiftReg = (Ppu->HighPatternShiftReg << 8) | Ppu->NextHighPattern;
        Ppu->PaletteLatchOld = Ppu->PaletteLatchNew;
        Ppu->PaletteLatchNew = Ppu->NextAtrbByte << 2;
               
        
        uint16 NametableAddress = 0x2000 | (Ppu->VRamIO.VRamAdrs & 0x0FFF);
        Ppu->NextNametableAdrs = readPpu8(NametableAddress, Ppu) << 4;
        Ppu->NextNametableAdrs += Ppu->BGPatternBase;
    }
    if(Cycle == 2)
    {
        uint16 AtrbAddress = 0x23C0 | (Ppu->VRamIO.VRamAdrs & 0x0C00) |
            ((Ppu->VRamIO.VRamAdrs >> 4) & 0x38) | ((Ppu->VRamIO.VRamAdrs >> 2) & 0x07);   
        uint8 Atrb = readPpu8(AtrbAddress, Ppu);
        int quadrantSelect = ((Ppu->VRamIO.VRamAdrs & 2) >> 1) | ((Ppu->VRamIO.VRamAdrs & 0x40) >> 5);
        
        Ppu->NextAtrbByte = ((0xFF & Atrb) >> (quadrantSelect*2)) & 3;       
    }   
    if(Cycle == 4)
    {        
        Ppu->NextNametableAdrs = Ppu->NextNametableAdrs + ((Ppu->VRamIO.VRamAdrs & 0x7000) >> 12);
        Ppu->NextLowPattern = readPpu8(Ppu->NextNametableAdrs, Ppu);
    }
    if(Cycle == 6)
    {
        Ppu->NextHighPattern = readPpu8(Ppu->NextNametableAdrs+8, Ppu);
    }           
    if(Cycle == 7)
    {
        if(Ppu->ScanlineCycle == 256)
        {
            scrollIncVert(&Ppu->VRamIO);
        }
        else
        {
            scrollIncHorz(&Ppu->VRamIO);               
        }
    }
}

// NOTE: Reverse the bits within a byte, eg. 0011 0101 becomes 1010 1100
//       Works by diving into halves and swaping the two. 
static uint8 byteReverse(uint8 byte)
{
    byte = (byte & 0xF0) >> 4 | (byte & 0x0F) << 4; // 1111 0000 and 0000 1111 swap
    byte = (byte & 0xCC) >> 2 | (byte & 0x33) << 2; // 1100 and 0011 swap
    byte = (byte & 0xAA) >> 1 | (byte & 0x55) << 1; // 10 and 01 swap
    
    return byte;
}

static void clearSecondaryOam(ppu *Ppu)
{
    uint8 *Data = (uint8 *)Ppu->SecondaryOam;
    
    for(uint16 Byte = 0; Byte < (SECONDARY_OAM_SPRITE_MAX * sizeof(sprite)); ++Byte)
    {
        Data[Byte] = 0xFF;
    }
}

static void clearPreparedSprites(ppu *Ppu)
{
    uint8 *Data = (uint8 *)Ppu->PreparedSprites;
    
    for(uint16 Byte = 0; Byte < (SECONDARY_OAM_SPRITE_MAX * sizeof(sprite)); ++Byte)
    {
        Data[Byte] = 0xFF;
    }
}

/* Loops through the OAM and find the sprites one the next scanline */
static void evaluateSecondaryOam(ppu *Ppu)
{
    uint8 *Oam = Ppu->Oam;
    sprite *SecondaryOam = Ppu->SecondaryOam;
    Ppu->SecondarySpriteCount = 0;
    
    uint8 SpriteHeight = (Ppu->SpriteSize8x16 != 0) ? 16: 8;
                
    for(uint8 OamSpriteCount = 0;
        OamSpriteCount < OAM_SPRITE_TOTAL && Ppu->SecondarySpriteCount != SECONDARY_OAM_SPRITE_MAX;
        ++OamSpriteCount)
    {
        oam_sprite *OamSprite = (oam_sprite *)Oam + OamSpriteCount;
        
        if(OamSprite->Y <= Ppu->Scanline && Ppu->Scanline < (OamSprite->Y + SpriteHeight))
        {
            sprite Sprite = {};
            Sprite.OamData = *OamSprite;
            Sprite.Priority = !(Sprite.OamData.Atrb & (1 << 5));

            if(OamSpriteCount == 0 && OamSprite->Y != 255)
            {
                Sprite.SpriteZero = true;
            }

            // Palette
            Sprite.PaletteValue = Sprite.OamData.Atrb & 3;

            // Chr Pattern Select
            uint8 TileRelY = (uint8)(Ppu->Scanline - Sprite.OamData.Y ) % SpriteHeight;
            uint8 YOffset = (Sprite.OamData.Atrb & (1 << 7)) ? ((SpriteHeight-1) - TileRelY) : TileRelY;

            uint8 TileIndex;
            uint16 SpritePatternBase;
            
            if(!Ppu->SpriteSize8x16)
            {
                TileIndex = Sprite.OamData.Tile; 
                SpritePatternBase = Ppu->SPRTPattenBase;
            }
            else
            {
                TileIndex = Sprite.OamData.Tile & ~0x1; // Bit 0 is ignored in 8x16 mode 
                SpritePatternBase = (Sprite.OamData.Tile & 1) ? 0x1000 : 0;

                if(YOffset >= 8)
                {
                    YOffset -= 8;
                    TileIndex += 1;
                }
            }

            uint16 LowAddress  = (SpritePatternBase + (TileIndex * 16)) + YOffset;
            uint16 HighAddress = (SpritePatternBase + (TileIndex * 16) + 8) + YOffset;
            
            Sprite.PatternLow  = readPpu8(LowAddress, Ppu);
            Sprite.PatternHigh = readPpu8(HighAddress, Ppu);
            
            if(Sprite.OamData.Atrb & (1 << 6))
            {
                Sprite.PatternLow = byteReverse(Sprite.PatternLow);
                Sprite.PatternHigh = byteReverse(Sprite.PatternHigh);
            }
            
            SecondaryOam[Ppu->SecondarySpriteCount++] = Sprite;
        }
    }
}

static void prepareSprites(ppu *Ppu)
{
    Ppu->PreparedSpriteCount = Ppu->SecondarySpriteCount;
        
    for(int8 SpriteIdx = 0; SpriteIdx < Ppu->SecondarySpriteCount; ++SpriteIdx)
    {
        Ppu->PreparedSprites[SpriteIdx] = Ppu->SecondaryOam[SpriteIdx];
    }
}

static void ppuTick(ppu *Ppu)
{    
    vram_io *VRamIO = &Ppu->VRamIO; 

    bool32 RenderingEnabled = Ppu->ShowBackground || Ppu->ShowSprites;

    if(Ppu->SpriteZeroDelaySet)
    {
        Ppu->SpriteZeroDelaySet = false;
        Ppu->SpriteZeroHit = true;
    }

    bool32 VisibleLine = (0 <= Ppu->Scanline && Ppu->Scanline <= 239);
    bool32 VBlankLine = (241 <= Ppu->Scanline && Ppu->Scanline <= 260);
    bool32 PreRenderLine = (Ppu->Scanline == 261);

    if(PreRenderLine || VisibleLine)
    {
        if(RenderingEnabled)
        {   
            if( (1 <= Ppu->ScanlineCycle && Ppu->ScanlineCycle <= 256) ||
                (321 <= Ppu->ScanlineCycle && Ppu->ScanlineCycle <= 336) )
                loadFutureData(Ppu);
            if(Ppu->ScanlineCycle == 257)
                resetScrollHorz(VRamIO);
            if(PreRenderLine && 280 <= Ppu->ScanlineCycle && Ppu->ScanlineCycle <= 304)
                resetScrollVert(VRamIO);
        }    

        if(PreRenderLine)
        {
            if(Ppu->ScanlineCycle == 0)
            {
                DrawScreen = true; // NOTE: Always draw screen here. Nmi is exclusive to this
                Ppu->SpriteOverflow = false;
                Ppu->SpriteZeroDelaySet = false;
                Ppu->SpriteZeroHit = false;
            }
            if(Ppu->ScanlineCycle == 1)
            {
                Ppu->VerticalBlank = false;
                setNmi(false);
            }
        }
        
        if(VisibleLine)
        {            
            if(RenderingEnabled)
            {
                if(Ppu->ScanlineCycle == 1)
                {
                    clearSecondaryOam(Ppu);
                }
                if(Ppu->ScanlineCycle == 65 && Ppu->Scanline != 239)
                {
                    evaluateSecondaryOam(Ppu);
                }
                if(Ppu->ScanlineCycle == 257)
                {
                    clearPreparedSprites(Ppu);
                    prepareSprites(Ppu);
                }
            }

            if(1 <= Ppu->ScanlineCycle && Ppu->ScanlineCycle <= 256)
            {
                uint16 PixelX = Ppu->ScanlineCycle - 1;
                uint16 PixelY = Ppu->Scanline;

                uint8 *Colour; // Points to a palette value
                
                uint8 BackgroundColour = 0;

                // Get the default colour
                uint8 BlankPaletteIndex = readPpu8(BGRD_PALETTE_ADRS, Ppu);
                getPaletteValue(BlankPaletteIndex, &Colour);
                
                /* *********************** */
                /* Background Calculations */

                if(Ppu->ShowBackground && !(PixelX < 8 && !Ppu->ShowBGLeft8Pixels) )
                {
                    uint8 XOffset = 15 - (VRamIO->FineX + (PixelX % 8));
                        
                    uint8 PatternPixelValue = (((Ppu->HighPatternShiftReg >> (XOffset-1) ) & 2) |
                                               (Ppu->LowPatternShiftReg >> XOffset) & 1);

                    if(PatternPixelValue != 0) // NOTE: If Value is zero, then it is a background/transparent
                    {
                        uint8 AtrbPixelValue = (XOffset >= 8) ? Ppu->PaletteLatchOld : Ppu->PaletteLatchNew;
                        BackgroundColour = AtrbPixelValue | PatternPixelValue;
                    }
                            
                    uint8 BgrdPaletteIndex = readPpu8(BGRD_PALETTE_ADRS + BackgroundColour, Ppu);
                    getPaletteValue(BgrdPaletteIndex, &Colour);
                }
                
                /* ******************* */
                /* Sprite Calculations */
                        
                if(Ppu->ShowSprites && !(PixelX < 8 && !Ppu->ShowSPRTLeft8Pixels) )
                {
                    for(int16 SpriteIdx = Ppu->PreparedSpriteCount - 1; SpriteIdx >= 0; --SpriteIdx)
                    {
                        sprite *Sprite = Ppu->PreparedSprites + SpriteIdx;
                        uint8 SpriteX = Sprite->OamData.X;
                            
                        if(PixelX != 0xFF && SpriteX <= PixelX && PixelX < (SpriteX + PIXEL_PER_TILE)) 
                        {
                            uint8 RelX = ((PixelX - (SpriteX)) % 8);                                    
                            uint8 PatternValue = (((Sprite->PatternHigh >> (7 - RelX)) & 1) << 1) |
                                ((Sprite->PatternLow >> (7 - RelX)) & 1);
                                    
                            if(PatternValue != 0) // If bottom two bits is 0, then is multiple of 4.
                            {
                                uint8 SpriteColour = (Sprite->PaletteValue << 2) | PatternValue;

                                if(Sprite->SpriteZero && !Ppu->SpriteZeroHit &&
                                   Ppu->ShowBackground && BackgroundColour != 0 &&
                                   Ppu->Scanline <= 239 /*&& Sprite->OamData.Y != 255*/ && Ppu->ScanlineCycle != 256)
                                {
                                    Ppu->SpriteZeroHit = true;
                                }

                                if( Sprite->Priority || BackgroundColour == 0 )
                                {
                                    uint8 SprtPaletteIndex = readPpu8(SPRT_PALETTE_ADRS + SpriteColour, Ppu);                       
                                    getPaletteValue(SprtPaletteIndex, &Colour);                        
                                }
                            }
                        }
                    }
                }

                // PIXEL OUTPUT - using the resulting colour
                drawPixel(Ppu, PixelX, PixelY, Colour);                        
            }
        }
    }
    if(VBlankLine)
    {
        if(Ppu->Scanline == 241 && Ppu->ScanlineCycle == 1)
        {
            if(!Ppu->SupressVbl)
            {
                Ppu->VerticalBlank = true;
            }

            NmiFlag = Ppu->GenerateNMI && Ppu->VerticalBlank && !Ppu->SupressNmiSet;

            if(Ppu->SupressNmiSet)
                Ppu->SupressNmiSet = false;            
        }
    }
        
    // Incrementing to the next cycle. If reached end of
    // scanline cycles then increment scanline.
    ++Ppu->ScanlineCycle;
    
    // Move to next scanline
    if(Ppu->ScanlineCycle == 341)
    {        
        Ppu->Scanline += 1;
        Ppu->ScanlineCycle = 0;
    }

    // Past the last scanline, go to the start(end of frame)
    if(Ppu->Scanline == 262)
    {
        Ppu->Scanline = 0;
        Ppu->OddFrame = !Ppu->OddFrame;
        
        if(RenderingEnabled && Ppu->OddFrame)
        {
            Ppu->ScanlineCycle++; // NOTE: Odd frame with rendering.
        }
    }

}


static void runPpu(ppu *Ppu, uint16 ClocksToRun)
{
    while(ClocksToRun > 0)
    {
        --ClocksToRun;
        ppuTick(Ppu);
    }
}


static void runPpu(ppu *Ppu)
{    
    ppuTick(Ppu);
}

static void
initPpu(ppu *Ppu, uint64 MemoryBase, uint32 * BasePixel)
{
    ZeroMemory((uint8 *)MemoryBase, Kilobytes(64));
    
    *Ppu = {};
    
    OamData = Ppu->Oam;

    Ppu->MemoryBase = MemoryBase;
    Ppu->BasePixel = BasePixel;
    
    // Palette at startup according to Blargg
    uint8 PaletteSize = 32;
    uint8 PaletteStartup[] = {0x09, 0x01, 0x00, 0x01,
                              0x00, 0x02, 0x02, 0x0D,
                              0x08, 0x10, 0x08, 0x24,
                              0x00, 0x00, 0x04, 0x2C,
                              0x09, 0x01, 0x34, 0x03,
                              0x00, 0x04, 0x00, 0x14,
                              0x08, 0x3A, 0x00, 0x02,
                              0x00, 0x20, 0x2C, 0x08};

    for(uint8 Idx = 0; Idx < PaletteSize; ++Idx)
    {
        write8(PaletteStartup[Idx], 0x3F00 + Idx, Ppu->MemoryBase);
    }
}


