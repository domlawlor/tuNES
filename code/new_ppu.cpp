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


//Pre-render scanline (-1, 261)
//Visible scanlines (0-239)

// Post-render scanline (240)
// Ppu is idle during this scanline

//Vertical blanking lines (241-260)


static void visibleLine(ppu *Ppu)
{
    uint16 Scanline = Ppu->Scanline;
    uint16 Cycle = Ppu->ScanlineCycle;

    if(1 <= Cycle && Cycle <= 256)
    {
        uint16 PixelX = Cycle - 1;
        uint16 PixelY = Scanline;

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
            Ppu->SupressNmiSet = false;            
    }
}

static void preRenderLine(ppu *Ppu)
{
    uint16 Cycle = Ppu->ScanlineCycle;

    if(Cycle == 1)
    {
        // TODO: Old Ppu was slightly different. Check over this old implementation
        DrawScreen = true;
        Ppu->SpriteOverflow = false;
        Ppu->SpriteZeroDelaySet = false;
        Ppu->SpriteZeroHit = false;
        Ppu->VerticalBlank = false;
        setNmi(false);
    }
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
                // IDLE for a scanline
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

            if(Ppu->Scanline == 240)
            {
                Ppu->ScanlineType = POST_RENDER;
            }
            else if(241 <= Ppu->Scanline && Ppu->Scanline < 261)
            {
                Ppu->ScanlineType = VBLANK;
            }
            else if(Ppu->Scanline == 261)
            {
                Ppu->ScanlineType = PRE_RENDER;
            }
            else
            {
                Assert(0);
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
    
    // TODO: Check over OAM code
    OamData = Ppu->Oam;

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
