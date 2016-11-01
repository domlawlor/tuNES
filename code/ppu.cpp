/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#include "palette.cpp"


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

    uint16 CurrentXPixel;
    uint16 CurrentYPixel;
    uint32 *ZeroPixel;

    uint16 NameTableAddress;
    uint16 BkgrdPatAddress;
    uint16 SprtPatAddress;

    uint16 FullVRamAdrsIO;
    uint8 PrevAdrsWriteCount;
    uint8 PrevDataWriteCount;

    uint16 ScanLine;

    uint16 VRamIOAdrs;
};



/* TODO: Extract these to a struct.
   uint16 NameTableAddress;
   uint8 AddressIncrement;
   uint16 PatTblSprites;
   uint16 PatTblBackground;
   bool32 SpritesAre8x16;
   bool32 NmiOnVBlank;
   bool32 ColourMode;
   bool32 ClipBackground;
   bool32 ClipSprites;
   bool32 HideBackground;
   bool32 HideSprites;
   uint8 ColourIntensity;
*/

internal uint8 readPpuMemory8(uint16 Address, uint64 MemoryOffset)
{
    uint8 Result = readMemory8(Address, MemoryOffset);
    return(Result);
}
internal void writePpuMemory8(uint8 Byte, uint16 Address, uint64 MemoryOffset)
{
    if(0x4000 <= Address)
        Address = (Address % 0x4000);

#if 0
    char TextBuffer[256];
    _snprintf(TextBuffer, 256, "0x%X: %X\n", Address, Byte);
    OutputDebugString(TextBuffer);
#endif
    writeMemory8(Byte, Address, MemoryOffset);
}

internal void
getTempColour(uint8 TileX, uint8 TileY, uint8 *Red, uint8 *Green, uint8 *Blue)
{
    *Red = ((TileX << 4) + (TileY & 0x0F)) * 10; 
    *Green = ((TileY << 4) + (TileX & 0x0F)) * 10;
    *Blue = (TileX) * 20;
}


#define NAMETABLE_BYTE_COUNT 960

struct palette
{
    uint8 Colours[4][3];
};


#define BYTES_PER_PATTERN 16
#define TILE_COUNT_X 32
#define TILE_COUNT_Y 30

internal uint8 getColourIndex(uint8 CombinedBits, uint64 MemoryOffset)
{
    uint8 Result = {};
    #define PALETTE_ADDRESS 0x3F00
    uint8 *ColourPaletteAddress =  (uint8 *)((uint64)(PALETTE_ADDRESS + CombinedBits) + MemoryOffset);
    Result = *ColourPaletteAddress;
    return(Result);
}

internal uint8 getAttribute(ppu *PpuData, uint8 BlockX, uint8 BlockY)
{
    uint8 Result = {};
    
    uint64 AttributeAddress = PpuData->NameTableAddress + PpuData->MemoryOffset + NAMETABLE_BYTE_COUNT;

    // NOTE: Attribute data is condensed into one byte per 4x4 tiles.
    //       I have the blockX and Y which is a 2x2 Tile, Dividing this by 2 will give 4x4
    //       One byte is 0011 2233 where the attribute data is layout as following
    //       00 11         Each 2 bits represents a block
    //       22 33

    uint8 Tile4x4Width = TILE_COUNT_X / 4;
    
    uint8 Tile4x4X = BlockX / 2;
    uint8 Tile4x4Y = BlockY / 2;

    uint8 *Attribute = (uint8 *)(AttributeAddress + (Tile4x4Y * Tile4x4Width) + Tile4x4X);
    uint8 AtrbByte = *Attribute;

    uint8 TileNumX = BlockX % 2;
    uint8 TileNumY = BlockY % 2;

    if(TileNumX == 0)
    {
        if(TileNumY == 0)
        {
            Result = (AtrbByte << 6) & 3;
        }
        else if(TileNumY == 1)
        {
            Result = (AtrbByte << 2) & 3;
        }
    }
    else if(TileNumX == 1)
    {
        if(TileNumY == 0)
        {
            Result = (AtrbByte << 4) & 3;
        }
        else if(TileNumY == 1)
        {
            Result = AtrbByte & 3;
        }
    }
    return(Result);
}

struct bkgrd_pattern
{
    uint8 *Group1;
    uint8 *Group2;
};

internal bkgrd_pattern getBkgrdPattern(ppu *PpuData, uint8 PatternIndex)
{   
    bkgrd_pattern Result = {};
    
    uint64 PatTableAddress = PpuData->BkgrdPatAddress + PpuData->MemoryOffset;

    uint8 *PatAddress = (uint8 *)(PatTableAddress + (PatternIndex * BYTES_PER_PATTERN));

    Result.Group1 = PatAddress;
    Result.Group2 = PatAddress + (BYTES_PER_PATTERN / 2);
    
    return(Result);
}

internal uint8 getNametableValue(ppu *PpuData, uint8 TileX, uint8 TileY)
{
    uint64 NameTableAddress = PpuData->NameTableAddress + PpuData->MemoryOffset;
    uint8 *Value = (uint8 *)(NameTableAddress + (TileY * TILE_COUNT_X) + TileX);    
    return(*Value);
}

struct prevDraw
{
    bkgrd_pattern Pattern;
    uint8 PatternIndex;
    uint8 TileX, TileY;
    uint8 BlockX, BlockY;
    bool32 Initialised;
};

prevDraw Prev = {};

void ppuTick(screen_buffer *BackBuffer, ppu *PpuData)
{
    ppu_registers *Registers = PpuData->Registers; 
    
    uint8 NameTableFlag = Registers->Ctrl1 & 0x03;
    switch(NameTableFlag)
    {
        case 0:
        {
            PpuData->NameTableAddress = 0x2000;
            break;
        }
        case 1:
        {
            PpuData->NameTableAddress = 0x2400;
            break;
        }
        case 2:
        {
            PpuData->NameTableAddress = 0x2800;
            break;
        }
        case 3:
        {
            PpuData->NameTableAddress = 0x2C00;
            break;
        }
    }
    
    uint8 AddressIncrement = 1;
    if(Registers->Ctrl1 & (1 << 2))
        AddressIncrement = 32;

    
    if(Registers->Ctrl1 & (1 << 3))
        PpuData->SprtPatAddress = 0x1000;
    else
        PpuData->SprtPatAddress = 0x0000;
    
    if(Registers->Ctrl1 & (1 << 4))
        PpuData->BkgrdPatAddress = 0x1000;
    else
        PpuData->BkgrdPatAddress = 0x0000;

    
    bool32 SpritesAre8x16 = false;
    if(Registers->Ctrl1 & (1 << 5))
        SpritesAre8x16 = true;

    bool32 NmiOnVBlank = false;
    if(Registers->Ctrl1 & (1 << 7))
        NmiOnVBlank = true;


    bool32 ColourMode = true;
    if(Registers->Ctrl2 & 1)
        ColourMode = false;

    bool32 ClipBackground = false;
    if(Registers->Ctrl2 & (1 << 1))
        ClipBackground = true;

    bool32 ClipSprites = false;
    if(Registers->Ctrl2 & (1 << 2))
        ClipSprites = true;

    bool32 HideBackground = true;
    if(Registers->Ctrl2 & (1 << 3))
        HideBackground = false;

    bool32 HideSprites = true;
    if(Registers->Ctrl2 & (1 << 4))
        HideSprites = false;

    uint8 ColourIntensity = Registers->Ctrl2 >> 5;    

    if(VRamIOAdrsCount > PrevVRamIOAdrsCount)
    {
        PpuData->VRamIOAdrs = (PpuData->VRamIOAdrs << 8) | Registers->VRamAddress;
    }
    PrevVRamIOAdrsCount = VRamIOAdrsCount;
    
    if(VRamIOWriteCount > PrevVRamIOWriteCount)
    {
        uint8 Value = Registers->VRamIO;        
        writePpuMemory8(Value, PpuData->VRamIOAdrs, PpuData->MemoryOffset);

        PpuData->VRamIOAdrs += AddressIncrement;
    }
    PrevVRamIOWriteCount = VRamIOWriteCount;

    
    if(ResetScrollIOAdrs) // TODO: Finish this
    {
        ResetScrollIOAdrs = false;
    }
    if(ResetVRamIOAdrs)
    {
        PpuData->VRamIOAdrs = 0;
        VRamIOAdrsCount = PrevVRamIOAdrsCount = 0;
        VRamIOWriteCount = PrevVRamIOWriteCount = 0;        
        ResetVRamIOAdrs = false;
    }
    
    

    
    if(!Prev.Initialised)
    {
        Prev.Pattern.Group1 = (uint8 *)255;
        Prev.Pattern.Group2 = (uint8 *)255;
        Prev.PatternIndex = 255;
        Prev.TileX = 255;
        Prev.TileY = 255;
        Prev.BlockX = 255;
        Prev.BlockY = 255;
        Prev.Initialised = true;
    }
        
    /*
      Palette is stored at 0x3F00 to 0x3F20
      0x3F00 - 0x3F0F is Image Palette
      0x3F10 - 0x3F1F is Sprie Palette

      This is mirrored from 0x3F20 to 0x4000

      A palette is 16 indexes to colours stored
      in the colour palette. This does not mean 16
      seperate colours however as a shared colour is
      found every 4 bytes. So 0x3F00 = 0x3F04 = 0x3F08 = 0x3F0C

      Each group of palette is 4 colours.
      There are 4 groups.
      Attribute tables will name one of the four groups
      This is the 4 colours used by a 2x2 tile section
      Tiles are 8x8 pixels.
      Pattern Tables hold the 8x8 pixel data. Each pattern is
      16 bytes.
      The nametable points to which pattern is used for a 8x8 pixeled
      section.
      
     */

    // 341 PPU cycles in a scanline, and 262 scanlines in a frame
    
    if(PpuData->ScanLine == 0)
    {
        Registers->Status = Registers->Status & ~(1 << 7); // clear vblank
        Registers->Status = Registers->Status & ~(1 << 6); // clear sprite0
        Registers->Status = Registers->Status & ~(1 << 5); // clear spriteOverflow
    }
    else if(PpuData->ScanLine > 0 && PpuData->ScanLine < 240)
    {
        uint8 Red, Green, Blue;
        palette Palette = {};
        
        // NOTE: Pattern Tile
        uint8 TileX = PpuData->CurrentXPixel / 8;
        uint8 TileY = PpuData->CurrentYPixel / 8; 
        // NOTE: Pixel relative to a tile
        uint8 TilePixelX = PpuData->CurrentXPixel % 8;
        uint8 TilePixelY = PpuData->CurrentYPixel % 8;
        // NOTE: Attribute Block
        uint8 BlockX = PpuData->CurrentXPixel / 16; 
        uint8 BlockY = PpuData->CurrentYPixel / 16;

        // NOTE: Retrieve the pattern index for this tile 
        uint8 PatternIndex;
        if((TileX != Prev.TileX) || (TileY != Prev.TileY))
            PatternIndex = getNametableValue(PpuData, TileX, TileY);
        else
            PatternIndex = Prev.PatternIndex;
        
        Prev.TileX = TileX;
        Prev.TileY = TileY;

        bkgrd_pattern Pattern;

        // Grab the Pattern data based on the index given
        if(PatternIndex != Prev.PatternIndex)
            Pattern = getBkgrdPattern(PpuData, PatternIndex);
        else
            Pattern = Prev.Pattern;
        
        Prev.PatternIndex = PatternIndex;
        Prev.Pattern = Pattern;

        uint8 Attribute;
        if((BlockX != Prev.BlockX) || (BlockX != Prev.BlockY))
            Attribute = getAttribute(PpuData, BlockX, BlockY);
  
        Prev.BlockX = BlockX;
        Prev.BlockY = BlockY;
    
        // NOTE: Each pixel for a pattern has 2 bits for colour. The other 3 bits
        //       for colour are stored in the attribute table. Each bit is stored
        //       seperately in two seperate groups. This calculation will retrieve the
        //       required bit for a specific pixel and combine them to make a 2 bits
        uint8 Group1Bit = (Pattern.Group1[TilePixelY] >> (7 - TilePixelX)) & 1;
        uint8 Group2Bit = (Pattern.Group2[TilePixelY] >> (7 - TilePixelX)) & 1;
        uint8 CombinedBits = Group2Bit << 1 | Group1Bit;

        uint8 FullAttribute = (Attribute << 2) & CombinedBits;

        // TODO: This should fetch the Colour index from the image palette at 0x3F00
        uint8 ColourIndex = getColourIndex(CombinedBits, PpuData->MemoryOffset);

        getPaletteValue(ColourIndex, &Red, &Green, &Blue);
        uint32 *CurrentPixel = (PpuData->ZeroPixel + (PpuData->CurrentYPixel * BackBuffer->Width)) + PpuData->CurrentXPixel;
         
        *CurrentPixel  = ((Blue << 16) | (Green << 8) | Red);

        uint32 *Test = (PpuData->ZeroPixel);
        
        
        // Advance to the next pixel,
        // TODO: Make this a loop that updates once before vblank?
        if((PpuData->CurrentXPixel + 1) >= BackBuffer->Width)
            PpuData->CurrentYPixel = (PpuData->CurrentYPixel + 1) % BackBuffer->Height;
        PpuData->CurrentXPixel = (PpuData->CurrentXPixel + 1) % BackBuffer->Width;
    }
    else if(PpuData->ScanLine == 240) // Idle
    {
        
    }
    else if(PpuData->ScanLine == 241) // VBlank Start
    {
        
        Registers->Status = Registers->Status | (1 << 7);
        /*if(NmiOnVBlank)
            NMICalled = true;
        */
    }
    
    

    // 1 pre-render scanline,               0
    // 240 scanlines of picture,            1   - 240
    // 20 scanlines of VBlank               241 - 261   (70 for PAL)
    // and finally, 1 dummy scanline        262         (51 for Dendy)
    
    ++PpuData->ScanLine;
    PpuData->ScanLine = PpuData->ScanLine % 262;
}
