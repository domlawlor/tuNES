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

#define ATTRIBUTE_OFFSET 0x3C0

struct ppu_registers
{
    uint8 Ctrl1;
    uint8 Ctrl2;
    uint8 Status;
    uint8 SprAddress;
    uint8 SprIO;
    uint8 ScrollAddress;
    uint8 VRamAddress;
    uint8 VRamIO;
};

struct ppu
{
    ppu_registers *Registers; // NOTE: Pointer because points to cpu memory
    uint64 MemoryOffset;

    uint16 Scanline;
    uint16 ScanlineCycle;

    uint16 VRamIOAddress;
    
    uint32 *BasePixel;

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
    // TODO: Finish memory mirror
    if(0x3000 <= Address && Address < 0x3F00)
    {
        Assert(0);
    }
    if(0x3F20 <= Address && Address < 0x4000)
    {
        Assert(0);
    }
    if(Address >= 0x4000)
        Assert(0);
    uint8 Result = read8(Address, MemoryOffset);
    return(Result);
}
internal void writePpu8(uint8 Byte, uint16 Address, uint64 MemoryOffset)
{
    // TODO: Finish memory mirror
    if(0x3000 <= Address && Address < 0x3F00)
    {
        Assert(0);
    }
    if(0x3F20 <= Address && Address < 0x4000)
    {
        Assert(0);
    }
    if(Address >= 0x4000)
        Assert(0);
    
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

void ppuTick(screen_buffer *BackBuffer, ppu *Ppu)
{    
    ppu_registers *Registers = Ppu->Registers;
    
    // NOTE: This is where data is transferred from Cpu via IO registers
    if(VRamAdrsChange)
    {
        Ppu->VRamIOAddress = (Ppu->VRamIOAddress << 8) | (uint16)Registers->VRamAddress;
        
        // NOTE: If address is on the pallette. Then IO register is updated immediately
        if(0x3F00 <= Ppu->VRamIOAddress && Ppu->VRamIOAddress <= 0x3FFF)
            Registers->VRamIO = readPpu8(Ppu->VRamIOAddress, Ppu->MemoryOffset);

        VRamAdrsChange = false;
    }
    if(VRamIOChange)
    {
#if 0
        char TextBuffer[256];
        _snprintf(TextBuffer, 256, "Address: %X Data: %X \n", Ppu->VRamIOAddress, Registers->VRamIO);
        OutputDebugString(TextBuffer);
#endif   
        writePpu8(Registers->VRamIO, Ppu->VRamIOAddress, Ppu->MemoryOffset);
        if(Registers->Ctrl1 & (1 << 2))
            Ppu->VRamIOAddress += 32;
        else
            ++Ppu->VRamIOAddress;
        VRamIOChange = false;
    }

    VRamAdrsOnPalette = (0x3F00 <= Ppu->VRamIOAddress && Ppu->VRamIOAddress <= 0x3FFF);
    
    if(IOReadFromCpu)
    {
        if(VRamAdrsOnPalette)
        {
            if(Registers->Ctrl1 & (1 << 2))
                Ppu->VRamIOAddress += 32;
            else
                ++Ppu->VRamIOAddress;
            Registers->VRamIO = readPpu8(Ppu->VRamIOAddress, Ppu->MemoryOffset);
        }
        else
        {
            Registers->VRamIO = readPpu8(Ppu->VRamIOAddress, Ppu->MemoryOffset);
            if(Registers->Ctrl1 & (1 << 2))
                Ppu->VRamIOAddress += 32;
            else
                ++Ppu->VRamIOAddress;
            
        }
        IOReadFromCpu = false;
    }
    
    if(ResetVRamIOAdrs)
    {
        Ppu->VRamIOAddress = 0;
        ResetVRamIOAdrs = false;
    }
    if(ResetScrollIOAdrs)
    {
        // TODO:
        ResetScrollIOAdrs = false;
    }

    
    
    uint16 PatternBase = 0x0000;
    if(Registers->Ctrl1 & (1 << 4))
        PatternBase = 0x1000;

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

    
    
    bool32 VisibleLine = (0 <= Ppu->Scanline && Ppu->Scanline <= 239);
    bool32 PostRenderLine = (Ppu->Scanline == 240);
    bool32 VBlankLine = (241 <= Ppu->Scanline && Ppu->Scanline <= 260);
    bool32 PreRenderLine = (Ppu->Scanline == 261);
    
    if(VisibleLine)
    {
        // NOTE: At the moment I am going to just produce a pixel per cycle with fetching happening each time.
        // TODO: Timing of fetches so less loading happens, using fine X scrolling to move through bytes

        if(1 <= Ppu->ScanlineCycle && Ppu->ScanlineCycle <= PIXEL_WIDTH)
        {
            uint16 PixelX = Ppu->ScanlineCycle - 1;
            uint16 PixelY = Ppu->Scanline;

            
            uint8 NameTableValue = getNameTableValue(PixelX, PixelY, NameTableBaseAdrs, Ppu->MemoryOffset);
            uint8 PatternPixelValue = getPatternValue(PixelX, PixelY, NameTableValue, PatternBase, Ppu->MemoryOffset);
            uint8 AttributeValue = getAttributeValue(PixelX, PixelY, NameTableBaseAdrs + ATTRIBUTE_OFFSET, Ppu->MemoryOffset);

            uint8 PixelColourIndex = (AttributeValue << 2) | PatternPixelValue;

            uint8 PaletteIndex = readPpu8(BGRD_PALETTE_ADRS + PixelColourIndex, Ppu->MemoryOffset);
            
            uint8 Colour[3] = {};
            getPaletteValue(PaletteIndex, Colour);
           
            drawPixel(Ppu, PixelX, PixelY, Colour);
            
#if 0 
            char TextBuffer[256];
            _snprintf(TextBuffer, 256, "PixelPattern: %X AtrbValue: %X Complete: %X PaletteIndex: %X RGB: %d, %d, %d\n",
                      PatternPixelValue, AttributeValue, PixelColourIndex, PaletteIndex, Colour[0], Colour[1], Colour[2]);
            OutputDebugString(TextBuffer);

#endif
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
            NmiTriggered = true;
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
                //Ppu->NameTableByte2 = getNameTableValue(PixelFetchX, PixelFetchY, TableBaseNum, Ppu->MemoryOffset);
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


