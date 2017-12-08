#if !defined(PPU_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#define PIXEL_WIDTH 256
#define PIXEL_HEIGHT 240

#define PIXEL_PER_TILE 8

#define BGRD_PALETTE_ADRS 0x3F00
#define SPRT_PALETTE_ADRS 0x3F10

#define SECONDARY_OAM_SPRITE_MAX 8

enum NT_MIRROR
{
    SINGLE_SCREEN_BANK_A = 0,
    SINGLE_SCREEN_BANK_B,
    VERTICAL_MIRROR,
    HORIZONTAL_MIRROR,
    FOUR_SCREEN_MIRROR,   
};

struct vram_io
{
    uint16 VRamAdrs;
    uint16 TempVRamAdrs;
    uint8 LatchWrite;
    uint8 FineX;
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

struct ppu
{
    uint64 MemoryBase;
    uint32 *BasePixel;

    // TODO: Not just for ppu. also apu. So pull out into global
    uint8 OpenBus;

    // Nametable mirror type
    NT_MIRROR MirrorType;

    // Name table banks. Mirror type selects which one is used
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
    uint8 Oam[OAM_SIZE];

    uint8 SecondarySpriteCount;
    sprite SecondaryOam[SECONDARY_OAM_SPRITE_MAX];
    
    uint8 PreparedSpriteCount;
    sprite PreparedSprites[SECONDARY_OAM_SPRITE_MAX];
    
    uint16 Scanline;
    uint16 ScanlineCycle;

    vram_io VRamIO;

    bool32 OddFrame;

    bool32 SpriteZeroDelaySet;
    
    screen_buffer *BackBuffer;

    uint64 CycleCount;
    uint64 StartupClocks;
};


#define PPU_H
#endif
