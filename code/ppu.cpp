/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#include "memory.cpp"
#include "palette.cpp"


static void drawPixel(ppu *Ppu, uint16 X, uint16 Y, uint8 *Colour)
{
    uint32 *CurrentPixel = (Ppu->BasePixel + (Y * PIXEL_WIDTH)) + X;
    *CurrentPixel  = ((Colour[0] << 16) | (Colour[1] << 8) | Colour[2]);
}

uint16 LowPatternShiftReg;
uint16 HighPatternShiftReg;
uint8 PaletteLatchOld;
uint8 PaletteLatchNew;

uint8 NextLowPattern;
uint8 NextHighPattern;
uint8 NextAtrbByte;
uint16 NextNametableAdrs;

void resetScrollHorz(vram_io *VRamIO)
{   
    VRamIO->VRamAdrs &= ~(0x041F);
    VRamIO->VRamAdrs |= (VRamIO->TempVRamAdrs & 0x041F);
}

void resetScrollVert(vram_io *VRamIO)
{
    VRamIO->VRamAdrs = VRamIO->TempVRamAdrs;
}

void scrollIncHorz(vram_io *Vram)
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

void scrollIncVert(vram_io *Vram)
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
        LowPatternShiftReg = (LowPatternShiftReg << 8) | NextLowPattern;
        HighPatternShiftReg = (HighPatternShiftReg << 8) | NextHighPattern;
        PaletteLatchOld = PaletteLatchNew;
        PaletteLatchNew = NextAtrbByte << 2;
               
        
        uint16 NametableAddress = 0x2000 | (Ppu->VRamIO.VRamAdrs & 0x0FFF);
        NextNametableAdrs = readPpu8(NametableAddress, Ppu) << 4;
        NextNametableAdrs += Ppu->BGPatternBase;
    }
    if(Cycle == 2)
    {
        uint16 AtrbAddress = 0x23C0 | (Ppu->VRamIO.VRamAdrs & 0x0C00) |
            ((Ppu->VRamIO.VRamAdrs >> 4) & 0x38) | ((Ppu->VRamIO.VRamAdrs >> 2) & 0x07);   
        uint8 Atrb = readPpu8(AtrbAddress, Ppu);
        int quadrantSelect = ((Ppu->VRamIO.VRamAdrs & 2) >> 1) | ((Ppu->VRamIO.VRamAdrs & 0x40) >> 5);
        
        NextAtrbByte = ((0xFF & Atrb) >> (quadrantSelect*2)) & 3;       
    }   
    if(Cycle == 4)
    {        
        NextNametableAdrs = NextNametableAdrs + ((Ppu->VRamIO.VRamAdrs & 0x7000) >> 12);
        NextLowPattern = readPpu8(NextNametableAdrs, Ppu);
    }
    if(Cycle == 6)
    {
        NextHighPattern = readPpu8(NextNametableAdrs+8, Ppu);
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
            
    for(uint8 OamSpriteCount = 0;
        OamSpriteCount < OAM_SPRITE_TOTAL && Ppu->SecondarySpriteCount != SECONDARY_OAM_SPRITE_MAX;
        ++OamSpriteCount)
    {
        oam_sprite *OamSprite = (oam_sprite *)Oam + OamSpriteCount;
        
        if(Ppu->SecondarySpriteCount < SECONDARY_OAM_SPRITE_MAX &&
           OamSprite->Y <= Ppu->Scanline && Ppu->Scanline < (OamSprite->Y + PIXEL_PER_TILE))
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

            // CHR Data            
            uint16 SpritePatternBase = Ppu->SPRTPattenBase;
            if(Ppu->SpriteSize8x16)
            {
                SpritePatternBase = (Sprite.OamData.Tile & 1) ? 0x1000 : 0;
            }
            
            uint8 TileRelY = (uint8)(Ppu->Scanline - Sprite.OamData.Y ) % PIXEL_PER_TILE; // NOTE TODO: -1?????               
            Assert(0 <= TileRelY && TileRelY < PIXEL_PER_TILE);

            uint8 YOffset = (Sprite.OamData.Atrb & (1 << 7)) ? (7 - TileRelY) : TileRelY;
            
            uint16 LowAddress  = (SpritePatternBase + (Sprite.OamData.Tile * 16)) + YOffset;
            uint16 HighAddress = (SpritePatternBase + (Sprite.OamData.Tile * 16) + 8) + YOffset;

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
        Ppu->SpriteZeroHit = true;
        Ppu->SpriteZeroDelaySet = false;
    }
    
    // Incrementing to the next cycle. If reached end of
    // scanline cycles then increment scanline.
    ++Ppu->ScanlineCycle;
    ++Ppu->CycleCount;
    
    if(Ppu->ScanlineCycle == 341)
    {        
        Ppu->Scanline += 1;
        Ppu->ScanlineCycle = 0;
    }

    if(Ppu->Scanline == 262)
    {
/*
        char TextBuffer[256];
        _snprintf(TextBuffer, 256, "PpuCycles = %d, CpuCycles = %d\n", Ppu->CycleCount, GlobalCpu->CycleCount);
        OutputDebugString(TextBuffer);

        Ppu->CycleCount = 0;
        GlobalCpu->CycleCount = 0;
*/
        Ppu->Scanline = 0;
        Ppu->OddFrame = !Ppu->OddFrame;
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
            if(Ppu->ScanlineCycle == 1)
            {
                Ppu->SpriteOverflow = false;
                Ppu->SpriteZeroHit = false;

                Ppu->VerticalBlank = false;
                setNmi(false);
            }
            if(Ppu->ScanlineCycle == 339 && Ppu->ShowBackground && Ppu->OddFrame)
            {
                Ppu->ScanlineCycle++; // NOTE: Odd frame with rendering. Skips cycle 240
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
                if(Ppu->ScanlineCycle == 65)
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

                uint8 Colour[3] = {};

                uint8 BlankColour = 0;
                uint8 BackgroundColour = 0;

                if(0x3F00 <= Ppu->VRamIO.VRamAdrs && Ppu->VRamIO.VRamAdrs <= 0x3FFF)
                {
                    BlankColour = readPpu8(Ppu->VRamIO.VRamAdrs, Ppu);
                }

                getPaletteValue(BlankColour, Colour);
                
                /* *********************** */
                /* Background Calculations */

                if(Ppu->ShowBackground && !(PixelX < 8 && !Ppu->ShowBGLeft8Pixels) )
                {
                    uint8 XOffset = 15 - (VRamIO->FineX + (PixelX % 8));
                        
                    uint8 PatternPixelValue = (((HighPatternShiftReg >> (XOffset-1) ) & 2) |
                                               (LowPatternShiftReg >> XOffset) & 1);

                    if(PatternPixelValue != 0) // NOTE: If Value is zero, then it is a background/transparent
                    {
                        uint8 AtrbPixelValue = (XOffset >= 8) ? PaletteLatchOld : PaletteLatchNew;
                        BackgroundColour = AtrbPixelValue | PatternPixelValue;
                    }
                            
                    uint8 BgrdPaletteIndex = readPpu8(BGRD_PALETTE_ADRS + BackgroundColour, Ppu);
                    getPaletteValue(BgrdPaletteIndex, Colour);
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
                                   Ppu->Scanline <= 239 && Sprite->OamData.Y != 255 && Ppu->ScanlineCycle != 256)
                                {
                                    Ppu->SpriteZeroDelaySet = true;
                                }

                                if( Sprite->Priority || BackgroundColour == 0 )
                                {
                                    uint8 SprtPaletteIndex = readPpu8(SPRT_PALETTE_ADRS + SpriteColour, Ppu);                       
                                    getPaletteValue(SprtPaletteIndex, Colour);                        
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
            
            DrawScreen = true; // NOTE: Always draw screen here. Nmi is exclusive to this
        }
    }
}


