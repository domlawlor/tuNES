#if !defined(PPU_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

const uint16 PIXEL_WIDTH = 256;
const uint16 PIXEL_HEIGHT = 240;

const uint16 PixelsPerTile = 8;

const uint16 BackgroundPaletteAddress = 0x3F00;
const uint16 SpritePaletteAddress = 0x3F10;

const uint16 SecondaryOamSpriteMax = 8;

const uint16 OamSize = 0x100;
const uint16 OamSpriteTotal = 64;

enum NameTableMirrorType
{
    SINGLE_SCREEN_BANK_A = 0,
    SINGLE_SCREEN_BANK_B,
    VERTICAL_MIRROR,
    HORIZONTAL_MIRROR,
    FOUR_SCREEN_MIRROR,   
};

struct oam_sprite
{
    uint8 Y;
    uint8 Tile;
    uint8 Atrb;
    uint8 X;
};

struct sprite
{
    oam_sprite OamData;

    bool32 Priority;
    bool32 SpriteZero;
    
    uint8 PaletteValue;
    uint8 PatternLow;
    uint8 PatternHigh;
};

enum scanlineType
{
    VISIBLE = 0,
    POST_RENDER,
    VBLANK,
    PRE_RENDER
};

struct ppu
{
    uint64 MemoryBase;
    uint32 *BasePixel;

    bool32 RenderingEnabled;

    bool32 OddFrame;
    
    uint16 Scanline;
    uint16 ScanlineCycle;

    scanlineType ScanlineType;
    
    // VRAM Address
    uint16 VRamAdrs;
    uint16 TempVRamAdrs;
    uint8 LatchWrite;
    uint8 FineX;

    // 
    uint16 LowPatternShiftReg;
    uint16 HighPatternShiftReg;
    uint8 PaletteLatchOld;
    uint8 PaletteLatchNew;

    uint8 NextLowPattern;
    uint8 NextHighPattern;
    uint8 NextAtrbByte;
    uint16 NextNametableAdrs;

    // Name table banks. 
    NameTableMirrorType mirrorType;
    // TODO: Pre allocate?
    uint8 NametableBankA[0x400];
    uint8 NametableBankB[0x400];
    uint8 NametableBankC[0x400];
    uint8 NametableBankD[0x400];
    
    // Control Reg
    uint8 NametableBase;
    uint8 VRamIncrement;
    uint16 SPRTPattenBase;
    uint16 BGPatternBase;
    bool32 SpriteSize8x16;
    bool32 PpuSlave;
    bool32 GenerateNMI;

    // Mask Reg
    bool32 GreyScale;
    bool32 ShowBGLeft8Pixels;
    bool32 ShowSPRTLeft8Pixels;
    bool32 ShowBackground;
    bool32 ShowSprites;
    bool32 EmphasizeRed;
    bool32 EmphasizeGreen;
    bool32 EmphasizeBlue;

    // Status Reg
    bool32 SpriteOverflow;
    bool32 SpriteZeroHit;
    bool32 VerticalBlank;

    bool32 SupressVbl;
    bool32 SupressNmiSet;
    
    // Oam Address Reg
    uint8 OamAddress;

    // VRam Data Read Buffering
    uint8 VRamDataBuffer;
    
    //Sprites
    uint8 *OamDma; 
    uint8 Oam[OamSize];

    uint8 SecondarySpriteCount;
    sprite SecondaryOam[SecondaryOamSpriteMax];
    
    uint8 PreparedSpriteCount;
    sprite PreparedSprites[SecondaryOamSpriteMax];
};


#define PPU_H
#endif
