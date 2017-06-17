/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#include "palette.cpp"

#define PIXEL_WIDTH 256
#define PIXEL_HEIGHT 240

#define TILES_COUNT_X 32
#define TILES_COUNT_Y 30

#define PIXEL_PER_TILE 8

#define BGRD_PALETTE_ADRS 0x3F00
#define SPRT_PALETTE_ADRS 0x3F10

#define ATTRIBUTE_OFFSET 0x3C0

#define PIXELS_PER_TILE 8

#define PIXELS_PER_ATRB_BYTE 32
#define ATRB_BYTE_PER_ROW 8

#define SECOND_OAM_SPRITE_NUM 8

struct oam_sprite
{
    uint8 Y;
    uint8 Tile;
    uint8 Atrb;
    uint8 X;
};

struct ppu_registers
{
    uint8 Ctrl1;
    uint8 Ctrl2;
    uint8 Status;
    uint8 OamAddress;
    uint8 OamIO;
    uint8 ScrollAddress;
    uint8 VRamAddress;
    uint8 VRamIO;
};

struct bgrd_data
{
    uint8 NametableByte;
    uint8 AtrbByte;
    uint8 LowTile;
    uint8 HighTile;
};
            
bgrd_data CurrentTile;
bgrd_data NextTile;
bgrd_data LoadingTile;
            
struct vram_io
{
    uint16 VRamAdrs;
    uint16 TempVRamAdrs;
    uint8 LatchWrite;
    uint8 FineX;
};

struct ppu
{
    uint64 MemoryBase;
    uint32 *BasePixel;
    
    ppu_registers *Registers; // NOTE: Pointer because points to cpu memory

    uint8 *OamDma;
    
    uint8 Oam[OAM_SIZE];
    oam_sprite PreparedSprites[SECOND_OAM_SPRITE_NUM];
    uint8 SpriteTileLow[SECOND_OAM_SPRITE_NUM];
    uint8 SpriteTileHigh[SECOND_OAM_SPRITE_NUM];
    bool32 EnabledSprite[SECOND_OAM_SPRITE_NUM];
    
    uint16 Scanline;
    uint16 ScanlineCycle;

    vram_io VRamIO;
};

inline void drawPixel(ppu *Ppu, uint16 X, uint16 Y, uint8 *Colour)
{
    uint32 *CurrentPixel = (Ppu->BasePixel + (Y * PIXEL_WIDTH)) + X;
    *CurrentPixel  = ((Colour[0] << 16) | (Colour[1] << 8) | Colour[2]);
}


static uint16 ppuMemoryMirror(uint16 InAddress)
{
    uint16 Address = InAddress;
    if(Address >= 0x4000) // Over half of the memory map is mirrored
        Address = Address % 0x4000; 

    if(0x3F20 <= Address && Address < 0x4000)
        Address = (Address % (0x3F20 - 0x3F00)) + 0x3F00;
        
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
        if(Address == 0x3F04 || Address == 0x3F08 || Address == 0x3F0C)
            Address = 0x3F00;
    }
   
    // NOTE: Nametable Mirroring. Controlled by Cartridge
    if(0x3000 <= Address && Address < 0x3F00) // This first as it maps to the nametable range
        Address = (Address % 0x0F00) + 0x2000;
    
    if(Address >= 0x2000 && Address < 0x3000) 
    {
        switch(GlobalMirrorType)
        {
            case SINGLE_SCREEN_MIRROR:
            {
                Address = (Address % 0x0400) + 0x2000;
                break;
            }
            case VERTICAL_MIRROR:
            {
                if(Address >= 0x2800 && Address < 0x3000)
                    Address -= 0x0800;
                break;
            }
            case HORIZONTAL_MIRROR:
            {
                if( (Address >= 0x2400 && Address < 0x2800) ||
                    (Address >= 0x2C00 && Address < 0x3000) )
                    Address -= 0x0400; 
                break;
            }
            case FOUR_SCREEN_MIRROR:
            {
                break;
            }
            default:
            {
                Assert(0);
                break;
            }
        }
    }

#if 1
    // Debug tests, first is doing mirror again to see if it changes,
    // if so then need to reorder mirror
    uint16 TestAddress = Address;
    if(InAddress != Address)
        TestAddress = ppuMemoryMirror(Address); 
    Assert(TestAddress == Address);
    Assert(Address < 0x4000);
    Assert( !(Address >= 0x3F20 && Address < 0x4000) );
    Assert( !(Address >= 0x3000 && Address < 0x3F00) );
#endif

    return Address;
}


internal uint8 readPpu8(uint16 Address, ppu *Ppu)
{
    Address = ppuMemoryMirror(Address);
         
    uint8 Result = read8(Address, Ppu->MemoryBase);
    return(Result);
}
internal void writePpu8(uint8 Byte, uint16 Address, ppu *Ppu)
{
    Address = ppuMemoryMirror(Address);
    write8(Byte, Address, Ppu->MemoryBase);
}

static uint8 getLowTileByte(uint8 NameTableValue, uint16 PatternBase, ppu *Ppu)
{
    uint16 FineY = (Ppu->VRamIO.VRamAdrs & 0xF000) >> 12;
    uint64 LowAddress = (PatternBase + (NameTableValue * 16)) + FineY;
    uint8 LowPattern = readPpu8(LowAddress, Ppu);
    return(LowPattern);
}

static uint8 getHighTileByte(uint8 NameTableValue, uint16 PatternBase, ppu *Ppu)
{
    uint16 FineY = (Ppu->VRamIO.VRamAdrs & 0xF000) >> 12;
    uint64 HighAddress = (PatternBase + (NameTableValue * 16)) + FineY + 8;
    uint8 HighPattern = readPpu8(HighAddress, Ppu);
    return(HighPattern);
}

static uint8 getPixelPattern(uint8 X, uint8 FineX, bgrd_data CurrentTile, bgrd_data NextTile)
{
    uint16 RelX = (X % 8) + FineX;

    uint8 LowTile = CurrentTile.LowTile;
    uint8 HighTile = CurrentTile.HighTile;
    
    if(RelX >= 8)
    {
        RelX -= 8;
        LowTile = NextTile.LowTile;
        HighTile = NextTile.HighTile;
    }
    
    return (((HighTile >> (7 - RelX)) & 1) << 1) | (LowTile  >> (7 - RelX)) & 1;
}
static uint8 getPixelAtrb(uint8 X, vram_io *VRamIO, bgrd_data CurrentTile, bgrd_data NextTile)
{    
    uint8 CoarseX = VRamIO->VRamAdrs & 0x001F;
    uint8 CoarseY = (VRamIO->VRamAdrs & 0x03E0) >> 5 ;
    uint16 RelX = (X % 8) + VRamIO->FineX;
    uint16 ActualX = X + VRamIO->FineX; 
    
    uint8 Attribute = CurrentTile.AtrbByte;

    if( (CoarseX % 4 == 3) && (RelX >= 8) )
    {
        RelX -= 8;
        Attribute = NextTile.AtrbByte;
    }
    
    uint8 BlockRelX = (ActualX % PIXELS_PER_ATRB_BYTE) / 16;
    uint8 BlockRelY = (CoarseY % 4) / 2;

    uint8 AtrbValue;
    
    if(BlockRelX == 0 && BlockRelY == 0)
        AtrbValue = Attribute;
    else if(BlockRelX == 0 && BlockRelY == 1)
        AtrbValue = Attribute >> 4;
    else if(BlockRelX == 1 && BlockRelY == 0)
        AtrbValue = Attribute >> 2;
    else if(BlockRelX == 1 && BlockRelY == 1)
        AtrbValue = Attribute >> 6;
       
    return(AtrbValue & 3);    
}

static void loadFutureData(ppu *Ppu, bgrd_data *Current, bgrd_data *Next, bgrd_data *Loading)
{                            
    if((Ppu->ScanlineCycle % 8) == 1) 
    {
        uint16 NametableAddress = 0x2000 | (Ppu->VRamIO.VRamAdrs & 0x0FFF);
        Loading->NametableByte = readPpu8(NametableAddress, Ppu);
    }
    if((Ppu->ScanlineCycle % 8) == 3)
    {
        uint16 AtrbAddress = 0x23C0 | (Ppu->VRamIO.VRamAdrs & 0x0C00) |
            ((Ppu->VRamIO.VRamAdrs >> 4) & 0x38) | ((Ppu->VRamIO.VRamAdrs >> 2) & 0x07);   
        Loading->AtrbByte = readPpu8(AtrbAddress, Ppu);
    }

    uint16 BgrdPatternBase = 0x0000;
    if(Ppu->Registers->Ctrl1 & (1 << 4))
        BgrdPatternBase = 0x1000;
    
    if((Ppu->ScanlineCycle % 8) == 5)
    {
        Loading->LowTile = getLowTileByte(Loading->NametableByte, BgrdPatternBase, Ppu); 
    }
    if((Ppu->ScanlineCycle % 8) == 7)
    {
        Loading->HighTile = getHighTileByte(Loading->NametableByte, BgrdPatternBase, Ppu);
    }           
    if((Ppu->ScanlineCycle % 8) == 0)
    {
        *Current = *Next;
        *Next = *Loading;
    }
}

static void evaluateSecondaryOam(uint8 *Oam, oam_sprite *PreparedSprites, uint16 Scanline)
{
    uint8 PreparedCount = 0;

    // NOTE: Clear Secondary OAM
    for(uint16 index = 0; index < SECOND_OAM_SPRITE_NUM; ++index)
    {
        PreparedSprites[index] = {};
    }
            
    for(uint8 OamSpriteCount = 0; OamSpriteCount < OAM_SPRITE_TOTAL; ++OamSpriteCount)
    {
        oam_sprite *Sprite = (oam_sprite *)Oam + OamSpriteCount;
                
        if(Scanline >= (Sprite->Y + 1) && Scanline < ((Sprite->Y + 1) + PIXEL_PER_TILE) ) // Plus one because normally evaluated on previous scanline 
        {
            PreparedSprites[PreparedCount++] = *Sprite;
        }

        // NOTE: Filled Secondary Oam with 8 bytes. 
        if(PreparedCount == SECOND_OAM_SPRITE_NUM)
        {
            break;
            // TODO: Set sprite overflow flag?? 
        }
    }
}

void resetScrollHorz(vram_io *VRamIO)
{   
    VRamIO->VRamAdrs &= ~(0x0C1F);//~(0x041F);
    VRamIO->VRamAdrs |= (VRamIO->TempVRamAdrs & 0x041F);
}

void resetScrollVert(vram_io *VRamIO)
{
/*    VRamIO->VRamAdrs &= ~(0x7BE0);
    VRamIO->VRamAdrs |= (VRamIO->TempVRamAdrs & 0x7BE0);
*/
    VRamIO->VRamAdrs = VRamIO->TempVRamAdrs;
}


void scrollIncHorz(vram_io *Vram)
{
    // NOTE: Code take from nesdev wiki. Could be quicker??
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


void ppuTick(screen_buffer *BackBuffer, ppu *Ppu)
{    
    ppu_registers *Registers = Ppu->Registers;
    vram_io *VRamIO = &Ppu->VRamIO; 
    
    if(OamDataChange)
    {
        OamDataChange = false;
        Ppu->Oam[Registers->OamAddress] = Registers->OamIO;
        ++Registers->OamAddress;
    }

    if(ScrollAdrsChange)
    {
        ScrollAdrsChange = false;

        if(VRamIO->LatchWrite == 0)
        {
            VRamIO->FineX = Registers->ScrollAddress & 7; // Bit 0,1, and 2 are fine X
            VRamIO->TempVRamAdrs &= ~(0x001F); // Clear Bits
            VRamIO->TempVRamAdrs |= ((uint16)Registers->ScrollAddress) >> 3;
            VRamIO->LatchWrite = 1;
        }
        else
        {
            VRamIO->TempVRamAdrs &= ~(0x73E0); // Clear Bits
            VRamIO->TempVRamAdrs |= ((uint16)(Registers->ScrollAddress & 0x0007)) << 12; // Set fine scroll Y, bits 0-2 set bit 12-14
            VRamIO->TempVRamAdrs |= ((uint16)(Registers->ScrollAddress & 0x00F8)) << 2; // Set coarse Y, bits 3-7 set bit 5-9
            VRamIO->LatchWrite = 0;
        }
    }
    
    // NOTE: This is where data is transferred from Cpu via IO registers
    if(VRamAdrsChange)
    {
        VRamAdrsChange = false;

        if(VRamIO->LatchWrite == 0)
        {
            VRamIO->TempVRamAdrs &= ~(0x7F00); // Clear Bits. 14th bit does not get set again
            VRamIO->TempVRamAdrs |= ((uint16)(Registers->VRamAddress & 0x003F)) << 8;
            VRamIO->LatchWrite = 1;
        }
        else
        { 
            VRamIO->TempVRamAdrs &= ~(0x00FF); // Clear Bit that are about to be loaded
            VRamIO->TempVRamAdrs |= (uint16)(Registers->VRamAddress & 0x00FF); 
            VRamIO->VRamAdrs = VRamIO->TempVRamAdrs;
            VRamIO->LatchWrite = 0;
            
            // NOTE: If address is on the pallette. Then IO register is updated immediately
            if(0x3F00 <= VRamIO->VRamAdrs && VRamIO->VRamAdrs <= 0x3FFF)
                Registers->VRamIO = readPpu8(VRamIO->VRamAdrs, Ppu);
        }
    }

    if(IOWriteFromCpu)
    {
        IOWriteFromCpu = false;
        
        writePpu8(Registers->VRamIO, VRamIO->VRamAdrs, Ppu);
        if(Registers->Ctrl1 & (1 << 2))
            VRamIO->VRamAdrs += 32;
        else
            VRamIO->VRamAdrs += 1;
    }

    bool32 VRamAdrsOnPalette = (0x3F00 <= VRamIO->VRamAdrs && VRamIO->VRamAdrs <= 0x3FFF);
    
    if(IOReadFromCpu)
    {
        IOReadFromCpu = false;

        if(VRamAdrsOnPalette)
        {
            if(Registers->Ctrl1 & (1 << 2))
                VRamIO->VRamAdrs += 32;
            else
                VRamIO->VRamAdrs += 1;
            Registers->VRamIO = readPpu8(VRamIO->VRamAdrs, Ppu);
        }
        else
        {
            Registers->VRamIO = readPpu8(VRamIO->VRamAdrs, Ppu);
            if(Registers->Ctrl1 & (1 << 2))
                VRamIO->VRamAdrs += 32;
            else
                VRamIO->VRamAdrs += 1;
        }
    }
    
    if(ResetVRamIOAdrs)
    {
        VRamIO->LatchWrite = 0;
        ResetVRamIOAdrs = false;
    }
    if(ResetScrollIOAdrs)
    {
        VRamIO->LatchWrite = 0;
        ResetScrollIOAdrs = false;
    }
    
    uint16 Sprite8x16 = Registers->Ctrl1 & (1 << 5);
    
    uint16 SpritePatternBase = 0x0000;
    if(!Sprite8x16 && Registers->Ctrl1 & (1 << 3))
    {
        SpritePatternBase = 0x1000;        
    }
    

    bool32 BackgroundEnabled = Registers->Ctrl2 & (1 << 3);
    bool32 SpritesEnabled = Registers->Ctrl2 & (1 << 4);
    
    bool32 VisibleLine = (0 <= Ppu->Scanline && Ppu->Scanline <= 239);
    bool32 PostRenderLine = (Ppu->Scanline == 240);
    bool32 VBlankLine = (241 <= Ppu->Scanline && Ppu->Scanline <= 260);
    bool32 PreRenderLine = (Ppu->Scanline == 261);
    
    if(VisibleLine)
    {
        // NOTE: Do sprite Evaluation
        if(Ppu->ScanlineCycle == 0)
        {            
            evaluateSecondaryOam(Ppu->Oam, Ppu->PreparedSprites, Ppu->Scanline);
            
            for(uint8 SpriteIdx = 0; SpriteIdx < SECOND_OAM_SPRITE_NUM; ++SpriteIdx)
            {
                oam_sprite Sprite = Ppu->PreparedSprites[SpriteIdx];

                bool32 FlippedVert = Sprite.Atrb & (1 << 7);

                if(Sprite8x16)
                {
                    if(Sprite.Tile & 1)
                        SpritePatternBase = 0x1000;
                    else
                        SpritePatternBase = 0;
                }
                
                if((Sprite.Y+1) != 0 && Sprite.X != 0)
                {
                    uint8 TileRelY = (uint8)(Ppu->Scanline - (Sprite.Y+1)) % (uint8)PIXEL_PER_TILE;    
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
        }
        
        if(1 <= Ppu->ScanlineCycle && Ppu->ScanlineCycle <= 256)
        {
            uint16 PixelX = Ppu->ScanlineCycle - 1;
            uint16 PixelY = Ppu->Scanline;
                                    
            /* *********************** */
            /* Background Calculations */
                      
            uint8 PatternPixelValue = getPixelPattern(PixelX, VRamIO->FineX, CurrentTile, NextTile);
            uint8 AtrbPixelValue = getPixelAtrb(PixelX, VRamIO, CurrentTile, NextTile);

            uint8 BgrdColourIndex = (AtrbPixelValue << 2) | PatternPixelValue;

            /* ******************* */
            /* Sprite Calculations */

#define NO_COLOUR 0xFF
            oam_sprite *Sprite;
            uint8 SpriteColourIndex = NO_COLOUR;

            for(int8 SpriteIdx = SECOND_OAM_SPRITE_NUM - 1; SpriteIdx >= 0; --SpriteIdx)
            {
                Sprite = Ppu->PreparedSprites + SpriteIdx; 

                bool32 FlippedHorz = Sprite->Atrb & (1 << 6);
                                
                if(Sprite->X != 0 && PixelX >= Sprite->X && PixelX < (Sprite->X + 8)) 
                {
                    uint8 PixColourHigh = Sprite->Atrb & 3; 
                    uint8 PatternLow = Ppu->SpriteTileLow[SpriteIdx];
                    uint8 PatternHigh = Ppu->SpriteTileHigh[SpriteIdx];
                        
                    uint8 RelX = (PixelX - Sprite->X) % 8 ; 

                    uint8 LowPattern;
                    uint8 HighPattern;
                    
                    if(FlippedHorz)
                    {
                        LowPattern  = (PatternLow >> RelX) & 1;
                        HighPattern = (PatternHigh >> RelX) & 1;
                    }
                    else
                    {
                        LowPattern  = (PatternLow >> (7 - RelX)) & 1;
                        HighPattern = (PatternHigh >> (7 - RelX)) & 1;
                    }
                    
                    uint8 Value = (HighPattern << 1) | LowPattern;
                    

                    if((((PixColourHigh << 2) | Value) % 4) != 0)
                        SpriteColourIndex = (PixColourHigh << 2) | Value;
                }
            }

            /** **************/
            /* Drawing Pixel */
            
            uint8 Colour[3] = {};
            bool32 BgrdBaseColour = true;
            
            if(BackgroundEnabled)
            {
                if((BgrdColourIndex % 4) != 0)
                    BgrdBaseColour = false;

                uint8 BgrdPaletteIndex = readPpu8(BGRD_PALETTE_ADRS + BgrdColourIndex, Ppu);
                getPaletteValue(BgrdPaletteIndex, Colour);
            }

            bool32 SprtPixTransparent = true; 
            
            if(SpritesEnabled)
            {
                bool32 Priority = !(Sprite->Atrb & (1 << 5));
                if((SpriteColourIndex % 4) != 0)
                    SprtPixTransparent = false;
                    
                if(SpriteColourIndex != NO_COLOUR && !SprtPixTransparent)
                {
                    if(BgrdBaseColour || (!BgrdBaseColour && Priority))
                    {
                        uint8 SprtPaletteIndex = readPpu8(SPRT_PALETTE_ADRS + SpriteColourIndex, Ppu);                       
                        getPaletteValue(SprtPaletteIndex, Colour);                        
                    }
                }
            }

            // TODO: NOTE: SPRITE 0 should only work for the first sprite in the secondary oam ??? Causes problems with scrolling
            
            // Sprite 0
            if(!BgrdBaseColour && !SprtPixTransparent)
                Registers->Status = Registers->Status | (1 << 6);

            // Draw Pixel
            if(BackgroundEnabled || SpritesEnabled)
                drawPixel(Ppu, PixelX, PixelY, Colour);

            // NOTE: Loading next background information. Also scrolling
            loadFutureData(Ppu, &CurrentTile, &NextTile, &LoadingTile);            
            if((Ppu->ScanlineCycle % 8) == 0 && (BackgroundEnabled || SpritesEnabled) )
                scrollIncHorz(VRamIO);            

            
            if(Ppu->ScanlineCycle == 256 && (BackgroundEnabled || SpritesEnabled))
                scrollIncVert(VRamIO);
        }

        if(Ppu->ScanlineCycle == 257 && (BackgroundEnabled || SpritesEnabled))
            resetScrollHorz(VRamIO);
        
        if(321 <= Ppu->ScanlineCycle && Ppu->ScanlineCycle <= 340)
        {
            loadFutureData(Ppu, &CurrentTile, &NextTile, &LoadingTile);            
            if((Ppu->ScanlineCycle % 8) == 0 && (BackgroundEnabled || SpritesEnabled))
                scrollIncHorz(VRamIO);
        }
    }
    if(PostRenderLine)
    {
        //debugOutputPatternTable(Ppu);
        DrawScreen = true;
    }
    if(VBlankLine)
    {
        if(Ppu->Scanline == 241 && Ppu->ScanlineCycle == 1)
        {
            Registers->Status |= (1 << 7); // Set VBlank Status

            // NOTE: if turning on NMI when in vblank. The nmi will be generated immediately.
            if( (Registers->Status & (1 << 7)) && ( Registers->Ctrl1 & (1 << 7)) )
            {
                NmiTriggered = true;
            }
        }
    }
    if(PreRenderLine)
    {
        if(Ppu->ScanlineCycle == 1)
        {
            Registers->Status &= ~(1 << 5); // Clear Sprite Overflow
            Registers->Status &= ~(1 << 6); // Clear Sprite Zero Hit
            Registers->Status &= ~(1 << 7); // Clear Vblank status
            NmiTriggered = false;
        }
    
        if(1 <= Ppu->ScanlineCycle && Ppu->ScanlineCycle <= 256)
        {
            loadFutureData(Ppu, &CurrentTile, &NextTile, &LoadingTile);            
            if((Ppu->ScanlineCycle % 8) == 0 && (BackgroundEnabled || SpritesEnabled))
                scrollIncHorz(VRamIO);
        }

        if(Ppu->ScanlineCycle == 256 && (BackgroundEnabled || SpritesEnabled))
            scrollIncVert(VRamIO);
        
        if(Ppu->ScanlineCycle == 257 && BackgroundEnabled)
            resetScrollHorz(VRamIO);

        if(280 <= Ppu->ScanlineCycle && Ppu->ScanlineCycle <= 304 && (BackgroundEnabled || SpritesEnabled))
            resetScrollVert(VRamIO);

        if(321 <= Ppu->ScanlineCycle && Ppu->ScanlineCycle <= 340)
        {            
            loadFutureData(Ppu, &CurrentTile, &NextTile, &LoadingTile);            
            if((Ppu->ScanlineCycle % 8) == 0 && (BackgroundEnabled || SpritesEnabled))
                scrollIncHorz(VRamIO);        
        }
    }
    
    ++Ppu->ScanlineCycle;

    if(Ppu->ScanlineCycle == 341)
    {
        Ppu->Scanline += 1;
        Ppu->ScanlineCycle = 0;
    }

    if(Ppu->Scanline == 262)
    {
        Ppu->Scanline = 0;
    }
}




// NOTE: Debug functions


// TODO: Not yet functional with atributes.
void debugOutputPatternTable(ppu *Ppu) {
    for(int y = 0; y < 128; ++y)
    {
        for(int x = 0; x < 128; ++x)
        {
            uint8 PatternBase = 0x0000; // 0x1000;

            uint16 PatternIndex = ((y * 128) + x) / 8;
            
            uint8 RelX = x % 8;
            uint8 RelY = y % 8;

            uint64 LowAdrs = PatternBase + (PatternIndex * 16) + RelY;
            uint8 LowPattern = readPpu8(LowAdrs, Ppu);

            uint64 HighAdrs = PatternBase + (PatternIndex * 16) + RelY + 8;
            uint8 HighPattern = readPpu8(HighAdrs, Ppu);
            
            uint8 PatternValue = ((HighPattern >> (7 - RelX)) & 1) << 1 | (LowPattern  >> (7 - RelX)) & 1;



            // Attrib
            uint8 AttribIndex = PatternIndex / 4;
            uint16 AttribAds = (0x2000 + 0x3C0) + AttribIndex;
            uint8 Atrb = readPpu8(AttribAds, Ppu);
            
/*            
            
            uint8 CoarseX = VRamIO->VRamAdrs & 0x001F;
            uint8 CoarseY = (VRamIO->VRamAdrs & 0x03E0) >> 5 ;
            uint16 RelX = (X % 8) + VRamIO->FineX;
            uint16 ActualX = X + VRamIO->FineX; 
    
            uint8 BlockRelX = (ActualX % PIXELS_PER_ATRB_BYTE) / 16;
            uint8 BlockRelY = (CoarseY % 4) / 2;

            uint8 AtrbValue;
    
            if(BlockRelX == 0 && BlockRelY == 0)
                AtrbValue = Attribute;
            else if(BlockRelX == 1 && BlockRelY == 0)
                AtrbValue = Attribute >> 2;
            else if(BlockRelX == 0 && BlockRelY == 1)
                AtrbValue = Attribute >> 4;
            else if(BlockRelX == 1 && BlockRelY == 1)
                AtrbValue = Attribute >> 6;

            AtrbValue &= 3;
            
            uint8 BgrdColourIndex = (AtrbValue << 2) | PatternValue;

            uint8 BgrdPaletteIndex = readPpu8(BGRD_PALETTE_ADRS + BgrdColourIndex, Ppu);
            */
            uint8 Colour[3] = {};

            //getPaletteValue(BgrdPaletteIndex, Colour);

            
            if(PatternValue == 0)
            {
                Colour[0] = 255;
                Colour[1] = 0;
                Colour[2] = 0;
            }
            else if(PatternValue == 1)
            {

                Colour[0] = 0;
                Colour[1] = 255;
                Colour[2] = 0;                
            }
            else if(PatternValue == 2)
            {
                Colour[0] = 0;
                Colour[1] = 0;
                Colour[2] = 255;

            }
            else if(PatternValue == 3)
            {
                Colour[0] = 255;
                Colour[1] = 255;
                Colour[2] = 255;
            }
                        
            drawPixel(Ppu, x, y, Colour);
        }
    }
}
