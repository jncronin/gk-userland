#if 0

#include "gk.h"

static void dma2d_handler();

/* File: D2D_resize.c

#########################################################################################
#                                                                                       #
#         DMA2D bilinear bitmap resize (C)2015-2016 Alessandro Rocchegiani              #
#                                                                                       #
#########################################################################################
*/

#define CR_MASK     ((uint32_t)0xFFFCE0FC)  /* DMA2D CR Mask */

typedef enum
{
  D2D_STAGE_IDLE=0,
  D2D_STAGE_FIRST_LOOP=1,
  D2D_STAGE_2ND_LOOP=2,
  D2D_STAGE_DONE=3,
  D2D_STAGE_ERROR=4,
  D2D_STAGE_SETUP_BUSY=5,
  D2D_STAGE_SETUP_DONE=6
}D2D_Stage_Typedef;

/* Setup FG/BG address and FGalpha for linear blend, start DMA2D */
void D2D_Blend_Line(void);

/* resize loop parameter */
typedef struct
{
  uint32_t Counter;           /* Loop Counter */
  uint32_t BaseAddress;       /* Loop Base Address */
  uint32_t BlendIndex;        /* Loop Blend index (Q21) */
  uint32_t BlendCoeff;        /* Loop Blend coefficient (Q21) */
  uint16_t SourcePitchBytes;  /* Loop Source pitch bytes */
  uint16_t OutputPitchBytes;  /* Loop Output pitch bytes */
}D2D_Loop_Typedef;  

/*Current resize stage*/
D2D_Stage_Typedef  D2D_Loop_Stage = D2D_STAGE_IDLE;

/*First loop parameter*/
D2D_Loop_Typedef   D2D_First_Loop;
/*2nd loop parameter*/
D2D_Loop_Typedef   D2D_2nd_Loop;

/*current parameter pointer*/
D2D_Loop_Typedef*  D2D_Loop = &D2D_First_Loop;


/* storage of misc. parameter for 2nd loop DMA2D register setup */
struct
{
  uint32_t OutputBaseAddress; /* output bitmap Base Address */
  uint16_t OutputColorMode;   /* output color mode */
  uint16_t OutputHeight;      /* output height */
  uint16_t OutputPitch;       /* output pixel pitch */
  uint16_t SourceWidth;       /* source width */ 
}D2D_Misc_Param;



void blit(void *SourceBaseAddress, uint16_t SourcePitch, uint16_t SourceColorMode,
    uint16_t SourceWidth, uint16_t SourceHeight,
    void *OutputBaseAddress,
    uint16_t OutputPitch, uint16_t OutputColorMode,
    uint16_t SourceX, uint16_t SourceY,
    uint16_t OutputX, uint16_t OutputY,
    uint16_t OutputWidth, uint16_t OutputHeight)
{
    // TODO: should we run this on a particular core?
    NVIC_SetVector(DMA2D_IRQn, (uint32_t)(uintptr_t)dma2d_handler);

    // handle defaults
    if(OutputPitch == 0xffff)
        OutputPitch = SourcePitch;
    if(OutputColorMode == 0xffff)
        OutputColorMode = SourceColorMode;
    if(OutputWidth == 0xffff)
        OutputWidth = SourceWidth;
    if(OutputHeight == 0xffff)
        OutputHeight = SourceHeight;

    uint16_t PixelBytes,PitchBytes;
    uint32_t BaseAddress;

    const uint16_t BitsPerPixel[6]={32,24,16,16,16,8};

    /* Test for loop already in progress */
    if(D2D_Loop_Stage != D2D_STAGE_IDLE)
    return (D2D_STAGE_SETUP_BUSY);

    /* DMA2D operation mode */
    DMA2D->CR &= (uint32_t)CR_MASK;
    DMA2D->CR |= DMA2D_M2M_BLEND;

    /* DMA2D operation mode */ 
    DMA2D_ITConfig(DMA2D_IT_TC | DMA2D_IT_TE | DMA2D_IT_CE , ENABLE);

    /* first loop parameter init */
    PixelBytes                      = BitsPerPixel[R->SourceColorMode]>>3;
    PitchBytes                      = R->SourcePitch * PixelBytes;
    BaseAddress                     = (uint32_t)R->SourceBaseAddress + 
                                R->SourceY * PitchBytes + R->SourceX * PixelBytes;

    D2D_First_Loop.Counter          = R->OutputHeight;
    D2D_First_Loop.SourcePitchBytes = PitchBytes;
    D2D_First_Loop.OutputPitchBytes = R->SourceWidth<<2;  
    D2D_First_Loop.BaseAddress      = BaseAddress;
    D2D_First_Loop.BlendCoeff       = ((R->SourceHeight-1)<<21) / R->OutputHeight;
    D2D_First_Loop.BlendIndex       = D2D_First_Loop.BlendCoeff>>1; 

    DMA2D->FGPFCCR                  = (REPLACE_ALPHA_VALUE<<16) | R->SourceColorMode; 
    DMA2D->BGPFCCR                  = (REPLACE_ALPHA_VALUE<<16) | R->SourceColorMode | 0xff000000; 
    DMA2D->OPFCCR                   = DMA2D_ARGB8888;
    DMA2D->NLR                      = (1 | (R->SourceWidth<<16));
    DMA2D->OMAR                     = (uint32_t)R->WorkBuffer;

    /* 2nd loop parameter init */
    PixelBytes                      = BitsPerPixel[R->OutputColorMode]>>3;
    PitchBytes                      = R->OutputPitch * PixelBytes;
    BaseAddress                     = (uint32_t)R->OutputBaseAddress +
                                    R->OutputY * PitchBytes + R->OutputX * PixelBytes;

    D2D_2nd_Loop.Counter            = R->OutputWidth;
    D2D_2nd_Loop.SourcePitchBytes   = 4;
    D2D_2nd_Loop.OutputPitchBytes   = PixelBytes;  
    D2D_2nd_Loop.BaseAddress        = (uint32_t)R->WorkBuffer;
    D2D_2nd_Loop.BlendCoeff         = ((R->SourceWidth-1)<<21) / R->OutputWidth;
    D2D_2nd_Loop.BlendIndex         = D2D_2nd_Loop.BlendCoeff>>1; 

    D2D_Misc_Param.OutputBaseAddress= BaseAddress;
    D2D_Misc_Param.OutputColorMode  = R->OutputColorMode;
    D2D_Misc_Param.OutputHeight     = R->OutputHeight;
    D2D_Misc_Param.OutputPitch      = R->OutputPitch;
    D2D_Misc_Param.SourceWidth      = R->SourceWidth;

    /* start first loop stage */
    D2D_Loop = &D2D_First_Loop;
    D2D_Loop_Stage = D2D_STAGE_FIRST_LOOP;

    D2D_Blend_Line();
    return(D2D_STAGE_SETUP_DONE);


}

#endif
