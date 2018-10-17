/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

/*
  262 Scanlines a frame
  341 Clocks Per scanline
  1 Cpu Cycle = 3 Ppu Cycles
  Each Ppu Cycle produces a pixel output

 */

/*
TODO LIST

- Power up state
- DMA takes 512 cpu cycles. It stops normal execution of the cpu
- Secondary OAM Clear and sprite eval do not happen on pre render line. Sprite tile fetches still do

*/

#include "palette.cpp"

static void drawPixel(ppu *Ppu, uint16 X, uint16 Y, colour Colour)
{
    uint32 *CurrentPixel = (Ppu->BasePixel + (Y * PIXEL_WIDTH)) + X;
    *CurrentPixel  = ((Colour.R << 16) | (Colour.G << 8) | Colour.B);
}

static void resetScrollHorz(ppu *Ppu)
{   
    Ppu->VRamAdrs &= ~(0x041F);
    Ppu->VRamAdrs |= (Ppu->TempVRamAdrs & 0x041F);
}

static void resetScrollVert(ppu *Ppu)
{
    Ppu->VRamAdrs = Ppu->TempVRamAdrs;
}

static void scrollIncHorz(ppu *Ppu)
{
    //NOTE: Code take from nesdev wiki. Could be quicker??
    if ((Ppu->VRamAdrs & 0x001F) == 31) // if coarse X == 31
    {
        Ppu->VRamAdrs &= ~0x001F;  // coarse X = 0
        Ppu->VRamAdrs ^= 0x0400;   // switch horizontal nametable
    }
    else
    {
        Ppu->VRamAdrs += 1; // increment coarse X
    }
}

static void scrollIncVert(ppu *Ppu)
{
    // NOTE: Code take from nesdev wiki. Could be quicker??
    if ((Ppu->VRamAdrs & 0x7000) != 0x7000) // if fine Y < 7
    {
        Ppu->VRamAdrs += 0x1000; // increment fine Y
    }
    else
    {
        Ppu->VRamAdrs &= ~0x7000; // fine Y = 0        
        uint16 y = (Ppu->VRamAdrs & 0x03E0) >> 5 ; // let y = coarse Y
    
        if (y == 29)
        {
            y = 0; // coarse Y = 0
            Ppu->VRamAdrs ^= 0x0800; // switch vertical nametable
        }
        else if (y == 31)
        {
            y = 0; // coarse Y = 0, nametable not switched
        }
        else
        {
            y += 1; // increment coarse Y
        }
        Ppu->VRamAdrs = (Ppu->VRamAdrs & ~0x03E0) | (y << 5); // put coarse Y back into v
    }
}

static void loadFutureData(ppu *Ppu)
{    
    if((1 <= Ppu->ScanlineCycle && Ppu->ScanlineCycle <= 255) ||
       (321 <= Ppu->ScanlineCycle && Ppu->ScanlineCycle <= 336))
    {
        uint8 Cycle = (Ppu->ScanlineCycle) % 8;
        if(Cycle == 1) 
        {
            Ppu->LowPatternShiftReg = (Ppu->LowPatternShiftReg << 8) | Ppu->NextLowPattern;
            Ppu->HighPatternShiftReg = (Ppu->HighPatternShiftReg << 8) | Ppu->NextHighPattern;
            Ppu->PaletteLatchOld = Ppu->PaletteLatchNew;
            Ppu->PaletteLatchNew = Ppu->NextAtrbByte << 2;               
        }
        else if(Cycle == 2)
        {
            uint16 NametableAddress = 0x2000 | (Ppu->VRamAdrs & 0x0FFF);
            Ppu->NextNametableAdrs = readPpu8(NametableAddress, Ppu) << 4;
            Ppu->NextNametableAdrs += Ppu->BGPatternBase;
        }
        else if(Cycle == 4)
        {
            uint16 AtrbAddress = 0x23C0 | (Ppu->VRamAdrs & 0x0C00) |
                ((Ppu->VRamAdrs >> 4) & 0x38) | ((Ppu->VRamAdrs >> 2) & 0x07);   
            uint8 Atrb = readPpu8(AtrbAddress, Ppu);
            int QuadrantSelect = ((Ppu->VRamAdrs & 2) >> 1) | ((Ppu->VRamAdrs & 0x40) >> 5);
        
            Ppu->NextAtrbByte = ((0xFF & Atrb) >> (QuadrantSelect*2)) & 3;       
        }   
        else if(Cycle == 6)
        {        
            Ppu->NextNametableAdrs = Ppu->NextNametableAdrs + ((Ppu->VRamAdrs & 0x7000) >> 12);
            Ppu->NextLowPattern = readPpu8(Ppu->NextNametableAdrs, Ppu);
        }
        else if(Cycle == 0)
        {
            Ppu->NextHighPattern = readPpu8(Ppu->NextNametableAdrs+8, Ppu);
            scrollIncHorz(Ppu);
        }
    }
    else if(Ppu->ScanlineCycle == 256)
    {
        scrollIncVert(Ppu);
    }
    else if(Ppu->ScanlineCycle == 257)
    {
        resetScrollHorz(Ppu);
    }    
}

// NOTE: Reverse the bits within a byte, eg. 0011 0101 becomes 1010 1100
//       Works by diving into halves and swaping the two. 
inline uint8 byteReverse(uint8 byte)
{
    byte = (byte & 0xF0) >> 4 | (byte & 0x0F) << 4; // 1111 0000 and 0000 1111 swap
    byte = (byte & 0xCC) >> 2 | (byte & 0x33) << 2; // 1100 and 0011 swap
    byte = (byte & 0xAA) >> 1 | (byte & 0x55) << 1; // 10 and 01 swap
    
    return byte;
}

/* Loops through the OAM and find the sprites one the next scanline */
static void evaluateSecondaryOam(ppu *Ppu)
{
    uint8 *Oam = Ppu->Oam;
    sprite *SecondaryOam = Ppu->SecondaryOam;
    Ppu->SecondarySpriteCount = 0;
    
    uint8 SpriteHeight = (Ppu->SpriteSize8x16 != 0) ? 16: 8;
                
    for(uint8 OamSpriteCount = 0;
        OamSpriteCount < OamSpriteTotal && Ppu->SecondarySpriteCount != SecondaryOamSpriteMax;
        ++OamSpriteCount)
    {
        oam_sprite *OamSprite = (oam_sprite *)Oam + OamSpriteCount;
        
        if(OamSprite->Y <= Ppu->Scanline && Ppu->Scanline < (OamSprite->Y + SpriteHeight))
        {
            sprite Sprite = {};
            Sprite.OamData = *OamSprite;
            Sprite.Priority = !(Sprite.OamData.Atrb & (1 << 5));

            if(OamSpriteCount == 0)
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

static void visibleLine(ppu *Ppu)
{
    uint16 Scanline = Ppu->Scanline;
    uint16 Cycle = Ppu->ScanlineCycle;

    if(Ppu->RenderingEnabled)
    {
        loadFutureData(Ppu);

        if(Cycle == 64) // NOTE: Clearing takes 64 cycles. Running on very last one
        {
            // Secondary Oam Clear
            Ppu->SecondarySpriteCount = 0;
            for(uint8 SpriteCount = 0; SpriteCount < SecondaryOamSpriteMax; ++SpriteCount)
            {
                sprite *Sprite = Ppu->SecondaryOam + SpriteCount;
                Sprite->OamData.Y    = 0xFF;
                Sprite->OamData.Tile = 0xFF;
                Sprite->OamData.Atrb = 0xFF;
                Sprite->OamData.X    = 0xFF;

                Sprite->Priority = false;
                Sprite->SpriteZero = false;

                Sprite->PaletteValue = 0;
                Sprite->PatternLow = 0;
                Sprite->PatternHigh = 0;
            }
            // TODO: attempting to read $2004 will return $FF
        }
        else if(Cycle == 256) // Sprite Evaluation happens from cycle 65 to 256
        {
            evaluateSecondaryOam(Ppu);
        }
        else if(Cycle == 257)
        {
            // Clear prepared sprites
            Ppu->PreparedSpriteCount = 0;
            for(uint8 SpriteCount = 0; SpriteCount < SecondaryOamSpriteMax; ++SpriteCount)
            {
                sprite *Sprite = Ppu->PreparedSprites + SpriteCount;
                Sprite->OamData.Y    = 0xFF;
                Sprite->OamData.Tile = 0xFF;
                Sprite->OamData.Atrb = 0xFF;
                Sprite->OamData.X    = 0xFF;

                Sprite->Priority = false;
                Sprite->SpriteZero = false;

                Sprite->PaletteValue = 0;
                Sprite->PatternLow = 0;
                Sprite->PatternHigh = 0;
            }
            
            // Copy to prepared sprites to secondary buffer can evaluate the next scanline
            Ppu->PreparedSpriteCount = Ppu->SecondarySpriteCount;
        
            for(int8 SpriteIdx = 0; SpriteIdx < Ppu->SecondarySpriteCount; ++SpriteIdx)
            {
                Ppu->PreparedSprites[SpriteIdx] = Ppu->SecondaryOam[SpriteIdx];
            }
            // TODO: First empty slot has sprite 64s y coord followed by 3 0xFF bytes. Other empty slots are all 0xFF
        }
    }
    
    if(1 <= Cycle && Cycle <= 256)
    {
        uint16 PixelX = Cycle - 1;
        uint16 PixelY = Scanline;

        colour PixelColour = {};

        // Get the default colour
        uint8 BlankPaletteIndex = readPpu8(BackgroundPaletteAddress, Ppu);
        PixelColour = Palette[BlankPaletteIndex];
                
        /* *********************** */
        /* Background Calculations */
                
        uint8 BackgroundResult = 0;
        
        if(Ppu->ShowBackground && !(PixelX < 8 && !Ppu->ShowBGLeft8Pixels) )
        {
            uint8 XOffset = 15 - (Ppu->FineX + ((PixelX) % 8));
                        
            uint8 PatternPixelValue = (((Ppu->HighPatternShiftReg >> (XOffset-1) ) & 2) |
                                       (Ppu->LowPatternShiftReg >> XOffset) & 1);

            if(PatternPixelValue != 0) // NOTE: If Value is zero, then it is a background/transparent
            {
                uint8 AtrbPixelValue = (XOffset >= 8) ? Ppu->PaletteLatchOld : Ppu->PaletteLatchNew;
                BackgroundResult = AtrbPixelValue | PatternPixelValue;
            }
                            
            uint8 BgrdPaletteIndex = readPpu8(BackgroundPaletteAddress + BackgroundResult, Ppu);
            PixelColour = Palette[BgrdPaletteIndex];
        }
                
        /* ******************* */
        /* Sprite Calculations */
                        
        if(Ppu->ShowSprites && !(PixelX < 8 && !Ppu->ShowSPRTLeft8Pixels) )
        {
            for(int16 SpriteIdx = Ppu->PreparedSpriteCount - 1; SpriteIdx >= 0; --SpriteIdx)
            {
                sprite *Sprite = Ppu->PreparedSprites + SpriteIdx;
                uint8 SpriteX = Sprite->OamData.X;
                            
                if(PixelX != 0xFF && SpriteX <= PixelX && PixelX < (SpriteX + PixelsPerTile)) 
                {
                    uint8 RelX = ((PixelX - (SpriteX)) % 8);                                    
                    uint8 PatternValue = (((Sprite->PatternHigh >> (7 - RelX)) & 1) << 1) |
                        ((Sprite->PatternLow >> (7 - RelX)) & 1);
                                    
                    if(PatternValue != 0) // If bottom two bits is 0, then is multiple of 4.
                    {
                        uint8 SpriteColour = (Sprite->PaletteValue << 2) | PatternValue;

                        if(Sprite->SpriteZero && !Ppu->SpriteZeroHit &&
                           Ppu->ShowBackground && BackgroundResult != 0 &&
                           Ppu->Scanline <= 239 /*&& Sprite->OamData.Y != 255*/ && Ppu->ScanlineCycle != 256)
                        {
                            Ppu->SpriteZeroHit = true;
                        }

                        if( Sprite->Priority || BackgroundResult == 0 )
                        {
                            uint8 SprtPaletteIndex = readPpu8(SpritePaletteAddress + SpriteColour, Ppu);                       
                            PixelColour = Palette[SprtPaletteIndex];                        
                        }
                    }
                }
            }
        }
        
        // PIXEL OUTPUT - using the resulting colour
        drawPixel(Ppu, PixelX, PixelY, PixelColour);                        
    }

}

static void vblankLine(ppu *Ppu)
{
    uint16 Scanline = Ppu->Scanline;
    uint16 Cycle = Ppu->ScanlineCycle;

    if(Scanline == 241 && Cycle == 1)
    {
        // TODO TODO Check over
        if(!Ppu->SupressVbl)
        {
            Ppu->VerticalBlank = true;
        }

        NmiFlag = Ppu->GenerateNMI && Ppu->VerticalBlank && !Ppu->SupressNmiSet;

        if(Ppu->SupressNmiSet)
        {
            Ppu->SupressNmiSet = false;
        }
        
        GlobalDrawScreen = true;
    }
}

static void preRenderLine(ppu *Ppu)
{
    uint16 Cycle = Ppu->ScanlineCycle;

    if(Cycle == 1)
    {
        Ppu->SpriteOverflow = false;
        Ppu->SpriteZeroHit = false;
        Ppu->VerticalBlank = false;
        setNmi(false);
    }

    if(Ppu->RenderingEnabled)
    {

        if(Cycle == 64) // NOTE: Clearing takes 64 cycles. Running on very last one
        {
            // Secondary Oam Clear
            uint8 *Data = (uint8 *)Ppu->SecondaryOam;
    
            for(uint16 Byte = 0; Byte < (SecondaryOamSpriteMax * sizeof(sprite)); ++Byte)
            {
                Data[Byte] = 0xFF;
            }
        }
        else if(Cycle == 257)
        {
            // Clear prepared sprites
            uint8 *Data = (uint8 *)Ppu->PreparedSprites;
    
            for(uint16 Byte = 0; Byte < (SecondaryOamSpriteMax * sizeof(sprite)); ++Byte)
            {
                Data[Byte] = 0xFF;
            }
        }
        
        loadFutureData(Ppu);

        if(280 <= Cycle && Cycle <= 304)
        {
            resetScrollVert(Ppu);
        }
    }
}

static void postRenderLine(ppu *Ppu)
{
    // Does nothing
}

static void runPpu(ppu *Ppu, uint16 ClocksToRun)
{
    while(ClocksToRun > 0)
    {
        --ClocksToRun;
        
        switch(Ppu->ScanlineType)
        {
            case VISIBLE:
            {
                visibleLine(Ppu);
                break;
            }
            case POST_RENDER:
            {
                postRenderLine(Ppu);
                break;
            }
            case VBLANK:
            {
                vblankLine(Ppu);
                break;
            }
            case PRE_RENDER:
            {
                preRenderLine(Ppu);
                break;
            }
            default:
            {
                Assert(0);
            }
        }
    
        // Once finished, update the cycle and scanline if on new one
        ++Ppu->ScanlineCycle;

        Assert(Ppu->ScanlineCycle <= 341);

        // New Scanline
        if(Ppu->ScanlineCycle == 341)
        {
            Ppu->ScanlineCycle = 0;
            ++Ppu->Scanline;

            Assert(Ppu->Scanline <= 262);
            
            // If reached new Frame
            if(Ppu->Scanline == 262)
            {
                Ppu->Scanline = 0;
                Ppu->OddFrame = !Ppu->OddFrame;

                Ppu->ScanlineType = VISIBLE;

                // On oddframe, when rendering is enabled, the first cycle of frame is skipped
                if(Ppu->OddFrame && Ppu->RenderingEnabled)
                {
                    ++Ppu->ScanlineCycle;
                }
            }
            else if(Ppu->Scanline == 261)
            {
                Ppu->ScanlineType = PRE_RENDER;
            }
            else if(241 <= Ppu->Scanline)
            {
                Ppu->ScanlineType = VBLANK;
            }            
            else if(Ppu->Scanline == 240)
            {
                Ppu->ScanlineType = POST_RENDER;
            }
        }
    }
}


static void
initPpu(ppu *Ppu, uint64 MemoryBase, uint32 * BasePixel)
{
    ZeroMemory((uint8 *)MemoryBase, Kilobytes(64));
    
    *Ppu = {};

    Ppu->MemoryBase = MemoryBase;
    Ppu->BasePixel = BasePixel;

    // TODO: This is to test if oam is initialised differently.
    
    
    // TODO: Check over OAM code
    
    
    
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
