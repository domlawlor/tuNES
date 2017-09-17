#if !defined(CPU_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */


#define NMI_VEC     0xFFFA
#define RESET_VEC   0xFFFC
#define IRQ_BRK_VEC 0xFFFE 
#define INSTRUCTION_COUNT 256
#define STACK_ADDRESS 0x100

#define CARRY_BIT     0x01
#define ZERO_BIT      0x02
#define INTERRUPT_BIT 0x04
#define DECIMAL_BIT   0x08
#define BREAK_BIT     0x10
#define BLANK_BIT     0x20
#define OVERFLOW_BIT  0x40
#define NEGATIVE_BIT  0x80

enum addressType
{
    NUL = 0,
    ACM,  IMPL, IMED, REL,
    ZERO, ZERX, ZERY,
    ABS,  ABSX, ABSY, ABSJ, 
    INDX, INDY, INDI
};

enum addressMode
{
    NL = 0,
    R, RW, W
};

struct cpu
{
    uint8 A;
    uint8 X;
    uint8 Y; 
    uint8 Flags;
    uint8 StackPtr;
    uint16 PrgCounter;
    uint64 MemoryBase;

    uint8 Cycle;
    uint8 NextCycle;
    
    uint8  MapperReg;
    uint16 MapperWriteAddress;
    bool32 MapperWrite;
    uint8 MapperWriteCount;
    
    bool32 PadStrobe; 
    
    input InputPad1;
    uint8 Pad1CurrentButton;
    input InputPad2;
    uint8 Pad2CurrentButton;

    uint8 OpInstruction;
    uint8 OpLowByte;
    uint8 OpHighByte;
    uint8 OpValue;
    uint8 OpTemp;
};

#define CPU_H
#endif

