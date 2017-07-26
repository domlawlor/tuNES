#if !defined(PPU_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */


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
    uint8 Control;
    uint8 Mask;
    uint8 Status;
    uint8 OamAddress;
    uint8 OamData;
    uint8 Scroll;
    uint8 VRamAddress;
    uint8 VRamData;
    uint8 OamDma;
};
          
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
    
    ppu_registers Registers;

    // TODO: Not just for ppu. also apu. So pull out into global
    uint8 OpenBus;

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
    bool32 Sprite0Hit;
    bool32 VerticalBlank;

    //Sprites
    uint8 *OamDma; 
    uint8 Oam[OAM_SIZE];
    oam_sprite SecondaryOam[SECOND_OAM_SPRITE_NUM];
    uint8 SpriteTileLow[SECOND_OAM_SPRITE_NUM];
    uint8 SpriteTileHigh[SECOND_OAM_SPRITE_NUM];
    bool32 EnabledSprite[SECOND_OAM_SPRITE_NUM];
    
    uint16 Scanline;
    uint16 ScanlineCycle;

    vram_io VRamIO;

    bool32 OddFrame;
};


#define PPU_H
#endif
