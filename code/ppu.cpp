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

    uint16 VRamIOAddress;
    uint16 ScrollPosition;

    uint8 PixelFetchX;
    uint8 PixelFetchY;
    
    uint8 NameTableByte1;
    uint8 NameTableByte2;

    uint8 AtrbTableByte1;
    uint8 AtrbTableByte2;

    uint8 LowBGTileByte1;
    uint8 LowBGTileByte2;
};

inline void drawPixel(ppu *Ppu, uint16 X, uint16 Y, uint8 *Colour)
{
    uint32 *CurrentPixel = (Ppu->BasePixel + (Y * PIXEL_WIDTH)) + X;
    *CurrentPixel  = ((Colour[0] << 16) | (Colour[1] << 8) | Colour[2]);
}

internal uint8 readPpu8(uint16 Address, uint64 MemoryOffset)
{
    if(0x3000 <= Address && Address < 0x3F00)
        Address = (Address % (0x3000 - 0x2000)) + 0x2000;
    if(0x3F20 <= Address && Address < 0x4000)
        Address = (Address % (0x3F20 - 0x3F00)) + 0x3F00;
    if(Address >= 0x4000)
        Address = Address % 0x4000; 

    uint8 Result = read8(Address, MemoryOffset);
    return(Result);
}
internal void writePpu8(uint8 Byte, uint16 Address, uint64 MemoryOffset)
{
    if(0x3000 <= Address && Address < 0x3F00)
        Address = (Address % (0x3000 - 0x2000)) + 0x2000;
    if(0x3F20 <= Address && Address < 0x4000)
        Address = (Address % (0x3F20 - 0x3F00)) + 0x3F00;
    if(Address >= 0x4000)
        Address = Address % 0x4000; 
    
    write8(Byte, Address, MemoryOffset);
}

internal uint8 getNameTableValue(uint16 X, uint16 Y, uint16 TableBaseAddress, uint64 MemoryOffset)
{
    uint16 TileX = X / PIXEL_PER_TILE;
    uint16 TileY = Y / PIXEL_PER_TILE;
    
    Assert(0 <= TileX && TileX < TILES_COUNT_X);
    Assert(0 <= TileY && TileY < TILES_COUNT_Y);

    uint16 Address = (TableBaseAddress + (TileY * TILES_COUNT_X)) + TileX;
    uint8 Value = readPpu8(Address, MemoryOffset);
    return(Value);
}

#define PIXELS_PER_TILE 8

internal uint8 getPatternValue(uint16 X, uint16 Y, uint8 NameTableValue, uint16 PatternBase, uint64 MemoryOffset)
{
    uint8 TileRelX = X % PIXEL_PER_TILE;
    uint8 TileRelY = Y % PIXEL_PER_TILE;
    
    Assert(0 <= TileRelX && TileRelX < PIXEL_PER_TILE);
    Assert(0 <= TileRelY && TileRelY < PIXEL_PER_TILE);
  
    uint64 LowAddress = (PatternBase + (NameTableValue * 16)) + TileRelY;
    uint64 HighAddress = (PatternBase + (NameTableValue * 16) + 8) + TileRelY;
    
    uint8 LowPattern = readPpu8(LowAddress, MemoryOffset);
    uint8 HighPattern = readPpu8(HighAddress, MemoryOffset);

    LowPattern  = (LowPattern >> (7 - TileRelX)) & 1;
    HighPattern = (HighPattern >> (7 - TileRelX)) & 1;

    uint8 Value = (HighPattern << 1) | LowPattern;
    
    return(Value);
}

#define PIXELS_PER_ATRB_BYTE 32
#define ATRB_BYTE_PER_ROW 8

internal uint8 getAttributeValue(uint16 X, uint16 Y, uint16 AtrbTableBaseAdrs, uint64 MemoryOffset)
{
    uint8 AttributeByteX = X / PIXELS_PER_ATRB_BYTE;
    uint8 AttributeByteY = Y / PIXELS_PER_ATRB_BYTE;
    
    uint16 AtrbByteAdrs = (AtrbTableBaseAdrs + (AttributeByteY * ATRB_BYTE_PER_ROW)) + AttributeByteX;
    uint8 Attribute = readPpu8(AtrbByteAdrs, MemoryOffset);
    
    uint8 BlockRelX = (X % PIXELS_PER_ATRB_BYTE) / 16;
    uint8 BlockRelY = (Y % PIXELS_PER_ATRB_BYTE) / 16;

    uint8 Value;
    
    if(BlockRelX == 0)
        if(BlockRelY == 0)
            Value = Attribute;
        else if(BlockRelY == 1)
            Value = Attribute >> 4;
    if(BlockRelX == 1)
        if(BlockRelY == 0)
            Value = Attribute >> 2;
        else if(BlockRelY == 1)
            Value = Attribute >> 6;
    
    Value = Value & 3;
    
    return(Value);
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

void ppuTick(screen_buffer *BackBuffer, ppu *Ppu)
{    
    ppu_registers *Registers = Ppu->Registers;

    if(OamDataChange)
    {
        OamDataChange = false;
        Ppu->Oam[Registers->OamAddress] = Registers->OamIO;
        ++Registers->OamAddress;
    }
    
    if(ScrollAdrsChange)
    {
        ScrollAdrsChange = false;
        Ppu->ScrollPosition = (Ppu->ScrollPosition << 8) | (uint16)Registers->ScrollAddress;
    }
    
    // NOTE: This is where data is transferred from Cpu via IO registers
    if(VRamAdrsChange)
    {
        VRamAdrsChange = false;
        Ppu->VRamIOAddress = (Ppu->VRamIOAddress << 8) | (uint16)Registers->VRamAddress;
        
        // NOTE: If address is on the pallette. Then IO register is updated immediately
        if(0x3F00 <= Ppu->VRamIOAddress && Ppu->VRamIOAddress <= 0x3FFF)
            Registers->VRamIO = readPpu8(Ppu->VRamIOAddress, Ppu->MemoryBase);
    }

    if(IOWriteFromCpu)
    {
        IOWriteFromCpu = false;
        
        writePpu8(Registers->VRamIO, Ppu->VRamIOAddress, Ppu->MemoryBase);
        if(Registers->Ctrl1 & (1 << 2))
            Ppu->VRamIOAddress += 32;
        else
            ++Ppu->VRamIOAddress;
    }

    VRamAdrsOnPalette = (0x3F00 <= Ppu->VRamIOAddress && Ppu->VRamIOAddress <= 0x3FFF);
    
    if(IOReadFromCpu)
    {
        IOReadFromCpu = false;

        if(VRamAdrsOnPalette)
        {
            if(Registers->Ctrl1 & (1 << 2))
                Ppu->VRamIOAddress += 32;
            else
                ++Ppu->VRamIOAddress;
            Registers->VRamIO = readPpu8(Ppu->VRamIOAddress, Ppu->MemoryBase);
        }
        else
        {
            Registers->VRamIO = readPpu8(Ppu->VRamIOAddress, Ppu->MemoryBase);
            if(Registers->Ctrl1 & (1 << 2))
                Ppu->VRamIOAddress += 32;
            else
                ++Ppu->VRamIOAddress;
        }
    }
    
    if(ResetVRamIOAdrs)
    {
        Ppu->VRamIOAddress = 0;
        ResetVRamIOAdrs = false;
    }
    if(ResetScrollIOAdrs)
    {
        Ppu->ScrollPosition = 0;
        ResetScrollIOAdrs = false;
    }
    

    uint16 Sprite8x16 = Registers->Ctrl1 & (1 << 5);

        
    uint16 SpritePatternBase = 0x0000;
    if(Registers->Ctrl1 & (1 << 3))
        SpritePatternBase = 0x1000;

    uint16 BgrdPatternBase = 0x0000;
    if(Registers->Ctrl1 & (1 << 4))
        BgrdPatternBase = 0x1000;

    uint8 NameTableBaseNum = Registers->Ctrl1 & 3;
    uint16 NameTableBaseAdrs; 
    if(NameTableBaseNum == 0)
        NameTableBaseAdrs = 0x2000;
    else if(NameTableBaseNum == 1)
        NameTableBaseAdrs = 0x2400;
    else if(NameTableBaseNum == 2)
        NameTableBaseAdrs = 0x2800;
    else if(NameTableBaseNum == 3)
        NameTableBaseAdrs = 0x2C00;

    bool32 BackgroundEnabled = Registers->Ctrl2 & (1 << 3);
    bool32 SpritesEnabled = Registers->Ctrl2 & (1 << 4);
    
    bool32 VisibleLine = (0 <= Ppu->Scanline && Ppu->Scanline <= 239);
    bool32 PostRenderLine = (Ppu->Scanline == 240);
    bool32 VBlankLine = (241 <= Ppu->Scanline && Ppu->Scanline <= 260);
    bool32 PreRenderLine = (Ppu->Scanline == 261);

    if(VisibleLine)
    {
        // NOTE: At the moment I am going to just produce a pixel per cycle with fetching happening each time.
        // TODO: Timing of fetches so less loading happens, using fine X scrolling to move through bytes

        // NOTE: Do sprite Evaluation
        if(Ppu->ScanlineCycle == 0)
        {
            evaluateSecondaryOam(Ppu->Oam, Ppu->PreparedSprites, Ppu->Scanline);
            
            for(uint8 SpriteIdx = 0; SpriteIdx < SECOND_OAM_SPRITE_NUM; ++SpriteIdx)
            {
                oam_sprite Sprite = Ppu->PreparedSprites[SpriteIdx];

                bool32 FlippedVert = Sprite.Atrb & (1 << 7);
                
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
                    
                    Ppu->SpriteTileLow[SpriteIdx] = readPpu8(LowAddress, Ppu->MemoryBase);
                    Ppu->SpriteTileHigh[SpriteIdx] = readPpu8(HighAddress, Ppu->MemoryBase);
                }
            }
        }
        
        if(1 <= Ppu->ScanlineCycle && Ppu->ScanlineCycle <= PIXEL_WIDTH)
        {
            uint16 PixelX = Ppu->ScanlineCycle - 1;
            uint16 PixelY = Ppu->Scanline;
            
            /* *********************** */
            /* Background Calculations */
            
            uint8 NameTableValue = getNameTableValue(PixelX, PixelY, NameTableBaseAdrs, Ppu->MemoryBase);
            uint8 PatternPixelValue = getPatternValue(PixelX, PixelY, NameTableValue, BgrdPatternBase, Ppu->MemoryBase);
            uint8 AttributeValue = getAttributeValue(PixelX, PixelY, NameTableBaseAdrs + ATTRIBUTE_OFFSET, Ppu->MemoryBase);
            uint8 BgrdColourIndex = (AttributeValue << 2) | PatternPixelValue;

            /* ******************* */
            /* Sprite Calculations */

#define NO_COLOUR 0xFF
            oam_sprite *Sprite;
            uint8 SpriteColourIndex = NO_COLOUR;

            if(Sprite8x16)
                Assert(0);
                
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

                uint8 BgrdPaletteIndex = readPpu8(BGRD_PALETTE_ADRS + BgrdColourIndex, Ppu->MemoryBase);
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
                        uint8 SprtPaletteIndex = readPpu8(SPRT_PALETTE_ADRS + SpriteColourIndex, Ppu->MemoryBase);                       
                        getPaletteValue(SprtPaletteIndex, Colour);                        
                    }
                }
            }

            // Sprite 0
            if(!BgrdBaseColour && !SprtPixTransparent)
                Registers->Status = Registers->Status | (1 << 6);

            // Draw Pixel
            if(BackgroundEnabled || SpritesEnabled)
                drawPixel(Ppu, PixelX, PixelY, Colour);

        }
    }
    if(PostRenderLine)
    {
        DrawScreen = true;
        // NOTE: Ppu sits idle for this scanline
    }
    if(VBlankLine)
    {
        if(Ppu->Scanline == 241 && Ppu->ScanlineCycle == 1)
        {
            // TODO: if turning on NMI when in vblank. The nmi will be generated immediately.
            if(Registers->Ctrl1 & (1 << 7))
            {
                NmiTriggered = true;
            }
            Registers->Status = Registers->Status | (1 << 7); // Set VBlank Status
        }
    }
    if(PreRenderLine)
    {
        switch(Ppu->ScanlineCycle)
        {
            case 1:
            {
                Registers->Status = Registers->Status & ~(1 << 5); // Clear Sprite Overflow
                Registers->Status = Registers->Status & ~(1 << 6); // Clear Sprite Zero Hit
                Registers->Status = Registers->Status & ~(1 << 7); // Clear Vblank status
                break;
            }
            case 321:
            case 329:
            {
                Ppu->NameTableByte1 = Ppu->NameTableByte2;
                //Ppu->NameTableByte2 = getNameTableValue(PixelFetchX, PixelFetchY, TableBaseNum, Ppu->MemoryBase);
                break;
            }
        }
    }

    ++Ppu->ScanlineCycle;

    if(Ppu->ScanlineCycle > 341)
    {
        Ppu->Scanline += 1;
        Ppu->ScanlineCycle = 0;
    }

    if(Ppu->Scanline == 262)
    {
        Ppu->Scanline = 0;
    }
}


