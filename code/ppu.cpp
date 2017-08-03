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
        Vram->VRamAdrs += 0x1000; // increment fine Y
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
            y = 0; // coarse Y = 0, nametable not switched
        else
        {
            y += 1; // increment coarse Y
            Vram->VRamAdrs = (Vram->VRamAdrs & ~0x03E0) | (y << 5); // put coarse Y back into v
        }
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

static void clearSecondaryOam(oam_sprite *SecondaryOam)
{
    uint8 *Data = (uint8 *)SecondaryOam;
    
    for(uint16 Byte = 0; Byte < (SECOND_OAM_SPRITE_NUM * sizeof(oam_sprite)); ++Byte)
    {
        Data[Byte] = 0xFF;
    }
}

/* Loops through the OAM and find the sprites one the next scanline */
static void evaluateSecondaryOam(uint8 *Oam, oam_sprite *SecondaryOam, uint16 Scanline)
{
    uint8 PreparedCount = 0;
            
    for(uint8 OamSpriteCount = 0; OamSpriteCount < OAM_SPRITE_TOTAL; ++OamSpriteCount)
    {
        oam_sprite *Sprite = (oam_sprite *)Oam + OamSpriteCount;
        
        if(Scanline >= Sprite->Y && Scanline < (Sprite->Y + PIXEL_PER_TILE))
        {
            if(PreparedCount < SECOND_OAM_SPRITE_NUM)
            {
                SecondaryOam[PreparedCount++] = *Sprite;
            }
            if(Sprite->Y == 0xFF)
                Assert(1);
        }

        if(PreparedCount == SECOND_OAM_SPRITE_NUM)
            break;
    }
}

static void spriteEvaluation(ppu *Ppu)
{
    uint16 Sprite8x16 = Ppu->SpriteSize8x16;
    
    uint16 SpritePatternBase = Ppu->SPRTPattenBase;
    
    for(uint8 SpriteIdx = 0; SpriteIdx < SECOND_OAM_SPRITE_NUM; ++SpriteIdx)
    {
        oam_sprite Sprite = Ppu->SecondaryOam[SpriteIdx];

        if(Sprite.Y == 0xFF && Sprite.Tile == 0xFF)
            break;
                
        bool32 FlippedVert = Sprite.Atrb & (1 << 7);

        if(Sprite8x16)
        {
            if(Sprite.Tile & 1)
                SpritePatternBase = 0x1000;
            else
                SpritePatternBase = 0;
        }
                
        uint8 TileRelY = (uint8)(Ppu->Scanline - Sprite.Y - 1) % PIXEL_PER_TILE; // NOTE TODO: -1?????   
        Assert(0 <= TileRelY && TileRelY < PIXEL_PER_TILE);

        uint64 LowAddress;
        uint64 HighAddress;
                    
        if(FlippedVert)
        {
            LowAddress = (SpritePatternBase + (Sprite.Tile * 16)) + (7 - TileRelY);
            HighAddress = (SpritePatternBase + (Sprite.Tile * 16) + 8) + (7 - TileRelY);
        }
        else
        {
            LowAddress = (SpritePatternBase + (Sprite.Tile * 16)) + TileRelY;
            HighAddress = (SpritePatternBase + (Sprite.Tile * 16) + 8) + TileRelY;
        }
                    
        Ppu->SpriteTileLow[SpriteIdx] = readPpu8(LowAddress, Ppu);
        Ppu->SpriteTileHigh[SpriteIdx] = readPpu8(HighAddress, Ppu);                
    }
}


void ppuTick(screen_buffer *BackBuffer, ppu *Ppu)
{    
    vram_io *VRamIO = &Ppu->VRamIO; 

    bool32 RenderingEnabled = Ppu->ShowBackground || Ppu->ShowSprites;
    
    // TODO: Should pre render line actually be scanline zero instead?? Might put sprites in wrong position?
    bool32 VisibleLine = (0 <= Ppu->Scanline && Ppu->Scanline <= 239);
    bool32 PostRenderLine = (Ppu->Scanline == 240);
    bool32 VBlankLine = (241 <= Ppu->Scanline && Ppu->Scanline <= 260);
    bool32 PreRenderLine = (Ppu->Scanline == 261);
    
    if(VisibleLine)
    {
        // NOTE: Do sprite Evaluation
        if(Ppu->ScanlineCycle == 0)
        {
            spriteEvaluation(Ppu);
        }
        else {

            if(Ppu->ScanlineCycle == 257)
            {
                clearSecondaryOam(Ppu->SecondaryOam);          
                evaluateSecondaryOam(Ppu->Oam, Ppu->SecondaryOam, Ppu->Scanline);
            }   

            if(RenderingEnabled)
            {
                if(1 <= Ppu->ScanlineCycle && Ppu->ScanlineCycle <= 256)
                {
                    uint16 PixelX = Ppu->ScanlineCycle - 1;
                    uint16 PixelY = Ppu->Scanline;


                    // NOTE: Loading next background information. Also scrolling
                    if(Ppu->ScanlineCycle != 1)
                        loadFutureData(Ppu);
                    
                    /* *********************** */
                    /* Background Calculations */

                    uint8 BgrdColourIndex;
                
                    if( !(PixelX < 8 && !Ppu->ShowBGLeft8Pixels) )
                    {
                        uint8 XOffset = 15 - (VRamIO->FineX + (PixelX % 8));
                        
                        uint8 PatternPixelValue = (((HighPatternShiftReg >> (XOffset-1) ) & 2) |
                                                   (LowPatternShiftReg >> XOffset) & 1);
                        uint8 AtrbPixelValue = (XOffset >= 8) ? PaletteLatchOld : PaletteLatchNew;

                        BgrdColourIndex = AtrbPixelValue | PatternPixelValue;
                    }
                
                    /* ******************* */
                    /* Sprite Calculations */

                    bool32 Sprite0Check = false;
                    bool32 SpritePriority;
                
#define NO_COLOUR 0xFF
                    oam_sprite *Sprite;
                    uint8 SpriteColourIndex = NO_COLOUR;
                    if( !(PixelX < 8 && !Ppu->ShowSPRTLeft8Pixels) )
                    {
                        for(int8 SpriteIdx = SECOND_OAM_SPRITE_NUM - 1; SpriteIdx >= 0; --SpriteIdx)
                        {
                            Sprite = Ppu->SecondaryOam + SpriteIdx;
                            
                            if(PixelX != 0xFF && PixelX >= Sprite->X && PixelX < (Sprite->X + PIXEL_PER_TILE)) 
                            {
                                uint8 PixColourHigh = Sprite->Atrb & 3; 
                                uint8 PatternLow = Ppu->SpriteTileLow[SpriteIdx];
                                uint8 PatternHigh = Ppu->SpriteTileHigh[SpriteIdx];
                                uint8 LowPattern;
                                uint8 HighPattern;

                                if(Sprite->Atrb & (1 << 6))
                                {
                                    uint8 RelX = ((PixelX - (Sprite->X)) % 8); 
                                    LowPattern  = (PatternLow >> RelX) & 1;
                                    HighPattern = (PatternHigh >> RelX) & 1;
                                }                        
                                else
                                {
                                    uint8 RelX = ((PixelX - (Sprite->X)) % 8);
                                    LowPattern  = (PatternLow >> (7 - RelX)) & 1;
                                    HighPattern = (PatternHigh >> (7 - RelX)) & 1;
                                }
                    
                                uint8 Value = (HighPattern << 1) | LowPattern;
                    
                                if((((PixColourHigh << 2) | Value) % 4) != 0)
                                    SpriteColourIndex = (PixColourHigh << 2) | Value;

                                if(SpriteIdx == 0 && ((SpriteColourIndex & 3) != 0) && SpriteColourIndex != NO_COLOUR)
                                {
                                    Sprite0Check = true;
                                }

                                SpritePriority = !(Sprite->Atrb & (1 << 5));
                            }
                        }
                    }

                    /** **************/
                    /* Drawing Pixel */
            
                    uint8 Colour[3] = {};
                    bool32 BgrdTransparent = true;
            
                    if(Ppu->ShowBackground)
                    {
                        if((BgrdColourIndex & 3) != 0)
                            BgrdTransparent = false;

                        uint8 BgrdPaletteIndex = readPpu8(BGRD_PALETTE_ADRS + BgrdColourIndex, Ppu);
                        getPaletteValue(BgrdPaletteIndex, Colour);
                    }

                    bool32 SpriteTransparent = true; 
            
                    if(Ppu->ShowSprites)
                    {
                        if((SpriteColourIndex & 3) != 0)
                            SpriteTransparent = false;
                    
                        if(SpriteColourIndex != NO_COLOUR && !SpriteTransparent)
                        {
                            if(BgrdTransparent || SpritePriority)
                            {
                                uint8 SprtPaletteIndex = readPpu8(SPRT_PALETTE_ADRS + SpriteColourIndex, Ppu);                       
                                getPaletteValue(SprtPaletteIndex, Colour);                        
                            }
                        
                            if(Sprite0Check && !BgrdTransparent && Ppu->ScanlineCycle < 256)
                            {
                                Ppu->Sprite0Hit = true;
                            }
                        }
                    }            
                
                    drawPixel(Ppu, PixelX, PixelY, Colour);

                }            

                if(Ppu->ScanlineCycle == 257)
                    resetScrollHorz(VRamIO);
                if(321 <= Ppu->ScanlineCycle && Ppu->ScanlineCycle <= 340)
                    loadFutureData(Ppu);
            }
        }
    }
    if(PostRenderLine)
    {
        // NOTE: IDLE
        DrawScreen = true;
    }
    if(VBlankLine)
    {
        if(Ppu->Scanline == 241 && Ppu->ScanlineCycle == 1)
        {
            Ppu->VerticalBlank = true;
            TriggerNmi = Ppu->GenerateNMI;
        }
    }
    if(PreRenderLine)
    {
        if(Ppu->ScanlineCycle == 1)
        {
            Ppu->SpriteOverflow = false;
            Ppu->Sprite0Hit = false;
            Ppu->VerticalBlank = false; 
            NmiTriggered = false;
        }

        if(RenderingEnabled)
        {
            if(1 <= Ppu->ScanlineCycle && Ppu->ScanlineCycle <= 256)
                loadFutureData(Ppu);
            if(Ppu->ScanlineCycle == 257)
                resetScrollHorz(VRamIO);

            if(280 <= Ppu->ScanlineCycle && Ppu->ScanlineCycle <= 304)
                resetScrollVert(VRamIO);
            
            if(321 <= Ppu->ScanlineCycle && Ppu->ScanlineCycle <= 336)
                loadFutureData(Ppu);

            // NOTE: Last cycle of prerender is skipped if it is an odd frame
            if(Ppu->OddFrame && Ppu->ScanlineCycle == 339)
                Ppu->ScanlineCycle += 1;
        }
    }
            
    // Incrementing to the next cycle. If reached end of
    // scanline cycles then increment scanline.
    ++Ppu->ScanlineCycle;
    
    if(Ppu->ScanlineCycle == 341)
    {
        Ppu->Scanline += 1;
        Ppu->ScanlineCycle = 0;
    }

    if(Ppu->Scanline == 262)
    {
        Ppu->Scanline = 0;
         Ppu->OddFrame = !Ppu->OddFrame;
    }
}


