/* $Id: s_blend.c,v 1.9.2.3 2002/04/10 16:32:50 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.0.3
 *
 * Copyright (C) 1999-2002  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "glheader.h"
#include "context.h"
#include "macros.h"

#include "s_alphabuf.h"
#include "s_blend.h"
#include "s_context.h"
#include "s_pb.h"
#include "s_span.h"


#if defined(USE_MMX_ASM)
#include "X86/mmx.h"
#include "X86/common_x86_asm.h"
#define _BLENDAPI _ASMAPI
#else
#define _BLENDAPI
#endif


/*
 * Special case for glBlendFunc(GL_ZERO, GL_ONE)
 */
static void _BLENDAPI
blend_noop( GLcontext *ctx, GLuint n, const GLubyte mask[],
            GLchan rgba[][4], CONST GLchan dest[][4] )
{
   GLuint i;
   ASSERT(ctx->Color.BlendEquation==GL_FUNC_ADD_EXT);
   ASSERT(ctx->Color.BlendSrcRGB==GL_ZERO);
   ASSERT(ctx->Color.BlendDstRGB==GL_ONE);
   (void) ctx;

   for (i = 0; i < n; i++) {
      if (mask[i]) {
         rgba[i][RCOMP] = dest[i][RCOMP];
         rgba[i][GCOMP] = dest[i][GCOMP];
         rgba[i][BCOMP] = dest[i][BCOMP];
         rgba[i][ACOMP] = dest[i][ACOMP];
      }
   }
}


/*
 * Special case for glBlendFunc(GL_ONE, GL_ZERO)
 */
static void _BLENDAPI
blend_replace( GLcontext *ctx, GLuint n, const GLubyte mask[],
               GLchan rgba[][4], CONST GLchan dest[][4] )
{
   ASSERT(ctx->Color.BlendEquation==GL_FUNC_ADD_EXT);
   ASSERT(ctx->Color.BlendSrcRGB==GL_ONE);
   ASSERT(ctx->Color.BlendDstRGB==GL_ZERO);
   (void) ctx;
   (void) n;
   (void) mask;
   (void) rgba;
   (void) dest;
}


/*
 * Common transparency blending mode.
 */
__attribute__((hot)) static void _BLENDAPI
blend_transparency( GLcontext *ctx, GLuint n, const GLubyte mask[],
                    GLchan rgba[][4], CONST GLchan dest[][4] )
{
   GLuint i;
   ASSERT(ctx->Color.BlendEquation==GL_FUNC_ADD_EXT);
   ASSERT(ctx->Color.BlendSrcRGB==GL_SRC_ALPHA);
   ASSERT(ctx->Color.BlendDstRGB==GL_ONE_MINUS_SRC_ALPHA);
   (void) ctx;

   for (i=0;i<n;i++) {
      if (mask[i]) {
         const GLint t = rgba[i][ACOMP];  /* t in [0, CHAN_MAX] */
         if (t == 0) {
            /* 0% alpha */
            rgba[i][RCOMP] = dest[i][RCOMP];
            rgba[i][GCOMP] = dest[i][GCOMP];
            rgba[i][BCOMP] = dest[i][BCOMP];
            rgba[i][ACOMP] = dest[i][ACOMP];
         }
         else if (t == CHAN_MAX) {
            /* 100% alpha, no-op */
         }
         else {
#if 0
            /* This is pretty close, but Glean complains */
            const GLint s = CHAN_MAX - t;
            const GLint r = (rgba[i][RCOMP] * t + dest[i][RCOMP] * s + 1) >> 8;
            const GLint g = (rgba[i][GCOMP] * t + dest[i][GCOMP] * s + 1) >> 8;
            const GLint b = (rgba[i][BCOMP] * t + dest[i][BCOMP] * s + 1) >> 8;
            const GLint a = (rgba[i][ACOMP] * t + dest[i][ACOMP] * s + 1) >> 8;
#elif 0
            /* This is slower but satisfies Glean */
            const GLint s = CHAN_MAX - t;
            const GLint r = (rgba[i][RCOMP] * t + dest[i][RCOMP] * s) / 255;
            const GLint g = (rgba[i][GCOMP] * t + dest[i][GCOMP] * s) / 255;
            const GLint b = (rgba[i][BCOMP] * t + dest[i][BCOMP] * s) / 255;
            const GLint a = (rgba[i][ACOMP] * t + dest[i][ACOMP] * s) / 255;
#else
#if CHAN_BITS == 8
            /* This satisfies Glean and should be reasonably fast */
            /* Contributed by Nathan Hand */
#define DIV255(X)  (((X) << 8) + (X) + 256) >> 16
            const GLint s = CHAN_MAX - t;
            const GLint r = DIV255(rgba[i][RCOMP] * t + dest[i][RCOMP] * s);
            const GLint g = DIV255(rgba[i][GCOMP] * t + dest[i][GCOMP] * s);
            const GLint b = DIV255(rgba[i][BCOMP] * t + dest[i][BCOMP] * s);
            const GLint a = DIV255(rgba[i][ACOMP] * t + dest[i][ACOMP] * s);
#undef DIV255
#elif CHAN_BITS == 16
            const GLfloat tt = (GLfloat) t / CHAN_MAXF;
            const GLfloat s = 1.0 - tt;
            const GLint r = (GLint) (rgba[i][RCOMP] * tt + dest[i][RCOMP] * s);
            const GLint g = (GLint) (rgba[i][GCOMP] * tt + dest[i][GCOMP] * s);
            const GLint b = (GLint) (rgba[i][BCOMP] * tt + dest[i][BCOMP] * s);
            const GLint a = (GLint) (rgba[i][ACOMP] * tt + dest[i][ACOMP] * s);
#else /* CHAN_BITS == 32 */
            const GLfloat tt = (GLfloat) t / CHAN_MAXF;
            const GLfloat s = 1.0 - tt;
            const GLfloat r = rgba[i][RCOMP] * tt + dest[i][RCOMP] * s;
            const GLfloat g = rgba[i][GCOMP] * tt + dest[i][GCOMP] * s;
            const GLfloat b = rgba[i][BCOMP] * tt + dest[i][BCOMP] * s;
            const GLfloat a = rgba[i][ACOMP] * tt + dest[i][ACOMP] * s;
#endif
#endif
            ASSERT(r <= CHAN_MAX);
            ASSERT(g <= CHAN_MAX);
            ASSERT(b <= CHAN_MAX);
            ASSERT(a <= CHAN_MAX);
            rgba[i][RCOMP] = (GLchan) r;
            rgba[i][GCOMP] = (GLchan) g;
            rgba[i][BCOMP] = (GLchan) b;
            rgba[i][ACOMP] = (GLchan) a;
         }
      }
   }
}



/*
 * Add src and dest.
 */
__attribute__((hot)) static void _BLENDAPI
blend_add( GLcontext *ctx, GLuint n, const GLubyte mask[],
           GLchan rgba[][4], CONST GLchan dest[][4] )
{
   GLuint i;
   ASSERT(ctx->Color.BlendEquation==GL_FUNC_ADD_EXT);
   ASSERT(ctx->Color.BlendSrcRGB==GL_ONE);
   ASSERT(ctx->Color.BlendDstRGB==GL_ONE);
   (void) ctx;

   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLint r = rgba[i][RCOMP] + dest[i][RCOMP];
         GLint g = rgba[i][GCOMP] + dest[i][GCOMP];
         GLint b = rgba[i][BCOMP] + dest[i][BCOMP];
         GLint a = rgba[i][ACOMP] + dest[i][ACOMP];
         rgba[i][RCOMP] = (GLchan) MIN2( r, CHAN_MAX );
         rgba[i][GCOMP] = (GLchan) MIN2( g, CHAN_MAX );
         rgba[i][BCOMP] = (GLchan) MIN2( b, CHAN_MAX );
         rgba[i][ACOMP] = (GLchan) MIN2( a, CHAN_MAX );
      }
   }
}



/*
 * Blend min function  (for GL_EXT_blend_minmax)
 */
__attribute__((hot)) static void _BLENDAPI
blend_min( GLcontext *ctx, GLuint n, const GLubyte mask[],
           GLchan rgba[][4], CONST GLchan dest[][4] )
{
   GLuint i;
   ASSERT(ctx->Color.BlendEquation==GL_MIN_EXT);
   (void) ctx;

   for (i=0;i<n;i++) {
      if (mask[i]) {
         rgba[i][RCOMP] = (GLchan) MIN2( rgba[i][RCOMP], dest[i][RCOMP] );
         rgba[i][GCOMP] = (GLchan) MIN2( rgba[i][GCOMP], dest[i][GCOMP] );
         rgba[i][BCOMP] = (GLchan) MIN2( rgba[i][BCOMP], dest[i][BCOMP] );
         rgba[i][ACOMP] = (GLchan) MIN2( rgba[i][ACOMP], dest[i][ACOMP] );
      }
   }
}



/*
 * Blend max function  (for GL_EXT_blend_minmax)
 */
__attribute__((hot)) static void _BLENDAPI
blend_max( GLcontext *ctx, GLuint n, const GLubyte mask[],
           GLchan rgba[][4], CONST GLchan dest[][4] )
{
   GLuint i;
   ASSERT(ctx->Color.BlendEquation==GL_MAX_EXT);
   (void) ctx;

   for (i=0;i<n;i++) {
      if (mask[i]) {
         rgba[i][RCOMP] = (GLchan) MAX2( rgba[i][RCOMP], dest[i][RCOMP] );
         rgba[i][GCOMP] = (GLchan) MAX2( rgba[i][GCOMP], dest[i][GCOMP] );
         rgba[i][BCOMP] = (GLchan) MAX2( rgba[i][BCOMP], dest[i][BCOMP] );
         rgba[i][ACOMP] = (GLchan) MAX2( rgba[i][ACOMP], dest[i][ACOMP] );
      }
   }
}



/*
 * Modulate:  result = src * dest
 */
__attribute__((hot)) static void _BLENDAPI
blend_modulate( GLcontext *ctx, GLuint n, const GLubyte mask[],
                GLchan rgba[][4], CONST GLchan dest[][4] )
{
   GLuint i;
   (void) ctx;

   for (i=0;i<n;i++) {
      if (mask[i]) {
#if CHAN_TYPE == GL_FLOAT
         rgba[i][RCOMP] = rgba[i][RCOMP] * dest[i][RCOMP];
         rgba[i][GCOMP] = rgba[i][GCOMP] * dest[i][GCOMP];
         rgba[i][BCOMP] = rgba[i][BCOMP] * dest[i][BCOMP];
         rgba[i][ACOMP] = rgba[i][ACOMP] * dest[i][ACOMP];
#else
         GLint r = (rgba[i][RCOMP] * dest[i][RCOMP]) >> 8;
         GLint g = (rgba[i][GCOMP] * dest[i][GCOMP]) >> 8;
         GLint b = (rgba[i][BCOMP] * dest[i][BCOMP]) >> 8;
         GLint a = (rgba[i][ACOMP] * dest[i][ACOMP]) >> 8;
         rgba[i][RCOMP] = (GLchan) r;
         rgba[i][GCOMP] = (GLchan) g;
         rgba[i][BCOMP] = (GLchan) b;
         rgba[i][ACOMP] = (GLchan) a;
#endif
      }
   }
}



/*
 * General case blend pixels.
 * Input:  n - number of pixels
 *         mask - the usual write mask
 * In/Out:  rgba - the incoming and modified pixels
 * Input:  dest - the pixels from the dest color buffer
 */
__attribute__((hot)) static void _BLENDAPI
blend_general( GLcontext *ctx, GLuint n, const GLubyte mask[],
               GLchan rgba[][4], CONST GLchan dest[][4] )
{
   GLfloat rscale = 1.0F / CHAN_MAXF;
   GLfloat gscale = 1.0F / CHAN_MAXF;
   GLfloat bscale = 1.0F / CHAN_MAXF;
   GLfloat ascale = 1.0F / CHAN_MAXF;
   GLuint i;

   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLint Rs, Gs, Bs, As;  /* Source colors */
         GLint Rd, Gd, Bd, Ad;  /* Dest colors */
         GLfloat sR, sG, sB, sA;  /* Source scaling */
         GLfloat dR, dG, dB, dA;  /* Dest scaling */
         GLfloat r, g, b, a;

         /* Source Color */
         Rs = rgba[i][RCOMP];
         Gs = rgba[i][GCOMP];
         Bs = rgba[i][BCOMP];
         As = rgba[i][ACOMP];

         /* Frame buffer color */
         Rd = dest[i][RCOMP];
         Gd = dest[i][GCOMP];
         Bd = dest[i][BCOMP];
         Ad = dest[i][ACOMP];

         /* Source RGB factor */
         switch (ctx->Color.BlendSrcRGB) {
            case GL_ZERO:
               sR = sG = sB = 0.0F;
               break;
            case GL_ONE:
               sR = sG = sB = 1.0F;
               break;
            case GL_DST_COLOR:
               sR = (GLfloat) Rd * rscale;
               sG = (GLfloat) Gd * gscale;
               sB = (GLfloat) Bd * bscale;
               break;
            case GL_ONE_MINUS_DST_COLOR:
               sR = 1.0F - (GLfloat) Rd * rscale;
               sG = 1.0F - (GLfloat) Gd * gscale;
               sB = 1.0F - (GLfloat) Bd * bscale;
               break;
            case GL_SRC_ALPHA:
               sR = sG = sB = (GLfloat) As * ascale;
               break;
            case GL_ONE_MINUS_SRC_ALPHA:
               sR = sG = sB = (GLfloat) 1.0F - (GLfloat) As * ascale;
               break;
            case GL_DST_ALPHA:
               sR = sG = sB = (GLfloat) Ad * ascale;
               break;
            case GL_ONE_MINUS_DST_ALPHA:
               sR = sG = sB = 1.0F - (GLfloat) Ad * ascale;
               break;
            case GL_SRC_ALPHA_SATURATE:
               if (As < CHAN_MAX - Ad) {
                  sR = sG = sB = (GLfloat) As * ascale;
               }
               else {
                  sR = sG = sB = 1.0F - (GLfloat) Ad * ascale;
               }
               break;
            case GL_CONSTANT_COLOR:
               sR = ctx->Color.BlendColor[0];
               sG = ctx->Color.BlendColor[1];
               sB = ctx->Color.BlendColor[2];
               break;
            case GL_ONE_MINUS_CONSTANT_COLOR:
               sR = 1.0F - ctx->Color.BlendColor[0];
               sG = 1.0F - ctx->Color.BlendColor[1];
               sB = 1.0F - ctx->Color.BlendColor[2];
               break;
            case GL_CONSTANT_ALPHA:
               sR = sG = sB = ctx->Color.BlendColor[3];
               break;
            case GL_ONE_MINUS_CONSTANT_ALPHA:
               sR = sG = sB = 1.0F - ctx->Color.BlendColor[3];
               break;
            case GL_SRC_COLOR: /* GL_NV_blend_square */
               sR = (GLfloat) Rs * rscale;
               sG = (GLfloat) Gs * gscale;
               sB = (GLfloat) Bs * bscale;
               break;
            case GL_ONE_MINUS_SRC_COLOR: /* GL_NV_blend_square */
               sR = 1.0F - (GLfloat) Rs * rscale;
               sG = 1.0F - (GLfloat) Gs * gscale;
               sB = 1.0F - (GLfloat) Bs * bscale;
               break;
            default:
               /* this should never happen */
               _mesa_problem(ctx, "Bad blend source RGB factor in do_blend");
               return;
         }

         /* Source Alpha factor */
         switch (ctx->Color.BlendSrcA) {
            case GL_ZERO:
               sA = 0.0F;
               break;
            case GL_ONE:
               sA = 1.0F;
               break;
            case GL_DST_COLOR:
               sA = (GLfloat) Ad * ascale;
               break;
            case GL_ONE_MINUS_DST_COLOR:
               sA = 1.0F - (GLfloat) Ad * ascale;
               break;
            case GL_SRC_ALPHA:
               sA = (GLfloat) As * ascale;
               break;
            case GL_ONE_MINUS_SRC_ALPHA:
               sA = (GLfloat) 1.0F - (GLfloat) As * ascale;
               break;
            case GL_DST_ALPHA:
               sA =(GLfloat) Ad * ascale;
               break;
            case GL_ONE_MINUS_DST_ALPHA:
               sA = 1.0F - (GLfloat) Ad * ascale;
               break;
            case GL_SRC_ALPHA_SATURATE:
               sA = 1.0;
               break;
            case GL_CONSTANT_COLOR:
               sA = ctx->Color.BlendColor[3];
               break;
            case GL_ONE_MINUS_CONSTANT_COLOR:
               sA = 1.0F - ctx->Color.BlendColor[3];
               break;
            case GL_CONSTANT_ALPHA:
               sA = ctx->Color.BlendColor[3];
               break;
            case GL_ONE_MINUS_CONSTANT_ALPHA:
               sA = 1.0F - ctx->Color.BlendColor[3];
               break;
            case GL_SRC_COLOR: /* GL_NV_blend_square */
               sA = (GLfloat) As * ascale;
               break;
            case GL_ONE_MINUS_SRC_COLOR: /* GL_NV_blend_square */
               sA = 1.0F - (GLfloat) As * ascale;
               break;
            default:
               /* this should never happen */
               sA = 0.0F;
               _mesa_problem(ctx, "Bad blend source A factor in do_blend");
         }

         /* Dest RGB factor */
         switch (ctx->Color.BlendDstRGB) {
            case GL_ZERO:
               dR = dG = dB = 0.0F;
               break;
            case GL_ONE:
               dR = dG = dB = 1.0F;
               break;
            case GL_SRC_COLOR:
               dR = (GLfloat) Rs * rscale;
               dG = (GLfloat) Gs * gscale;
               dB = (GLfloat) Bs * bscale;
               break;
            case GL_ONE_MINUS_SRC_COLOR:
               dR = 1.0F - (GLfloat) Rs * rscale;
               dG = 1.0F - (GLfloat) Gs * gscale;
               dB = 1.0F - (GLfloat) Bs * bscale;
               break;
            case GL_SRC_ALPHA:
               dR = dG = dB = (GLfloat) As * ascale;
               break;
            case GL_ONE_MINUS_SRC_ALPHA:
               dR = dG = dB = (GLfloat) 1.0F - (GLfloat) As * ascale;
               break;
            case GL_DST_ALPHA:
               dR = dG = dB = (GLfloat) Ad * ascale;
               break;
            case GL_ONE_MINUS_DST_ALPHA:
               dR = dG = dB = 1.0F - (GLfloat) Ad * ascale;
               break;
            case GL_CONSTANT_COLOR:
               dR = ctx->Color.BlendColor[0];
               dG = ctx->Color.BlendColor[1];
               dB = ctx->Color.BlendColor[2];
               break;
            case GL_ONE_MINUS_CONSTANT_COLOR:
               dR = 1.0F - ctx->Color.BlendColor[0];
               dG = 1.0F - ctx->Color.BlendColor[1];
               dB = 1.0F - ctx->Color.BlendColor[2];
               break;
            case GL_CONSTANT_ALPHA:
               dR = dG = dB = ctx->Color.BlendColor[3];
               break;
            case GL_ONE_MINUS_CONSTANT_ALPHA:
               dR = dG = dB = 1.0F - ctx->Color.BlendColor[3];
               break;
            case GL_DST_COLOR: /* GL_NV_blend_square */
               dR = (GLfloat) Rd * rscale;
               dG = (GLfloat) Gd * gscale;
               dB = (GLfloat) Bd * bscale;
               break;
            case GL_ONE_MINUS_DST_COLOR: /* GL_NV_blend_square */
               dR = 1.0F - (GLfloat) Rd * rscale;
               dG = 1.0F - (GLfloat) Gd * gscale;
               dB = 1.0F - (GLfloat) Bd * bscale;
               break;
            default:
               /* this should never happen */
               dR = dG = dB = 0.0F;
               _mesa_problem(ctx, "Bad blend dest RGB factor in do_blend");
         }

         /* Dest Alpha factor */
         switch (ctx->Color.BlendDstA) {
            case GL_ZERO:
               dA = 0.0F;
               break;
            case GL_ONE:
               dA = 1.0F;
               break;
            case GL_SRC_COLOR:
               dA = (GLfloat) As * ascale;
               break;
            case GL_ONE_MINUS_SRC_COLOR:
               dA = 1.0F - (GLfloat) As * ascale;
               break;
            case GL_SRC_ALPHA:
               dA = (GLfloat) As * ascale;
               break;
            case GL_ONE_MINUS_SRC_ALPHA:
               dA = (GLfloat) 1.0F - (GLfloat) As * ascale;
               break;
            case GL_DST_ALPHA:
               dA = (GLfloat) Ad * ascale;
               break;
            case GL_ONE_MINUS_DST_ALPHA:
               dA = 1.0F - (GLfloat) Ad * ascale;
               break;
            case GL_CONSTANT_COLOR:
               dA = ctx->Color.BlendColor[3];
               break;
            case GL_ONE_MINUS_CONSTANT_COLOR:
               dA = 1.0F - ctx->Color.BlendColor[3];
               break;
            case GL_CONSTANT_ALPHA:
               dA = ctx->Color.BlendColor[3];
               break;
            case GL_ONE_MINUS_CONSTANT_ALPHA:
               dA = 1.0F - ctx->Color.BlendColor[3];
               break;
            case GL_DST_COLOR: /* GL_NV_blend_square */
               dA = (GLfloat) Ad * ascale;
               break;
            case GL_ONE_MINUS_DST_COLOR: /* GL_NV_blend_square */
               dA = 1.0F - (GLfloat) Ad * ascale;
               break;
            default:
               /* this should never happen */
               dA = 0.0F;
               _mesa_problem(ctx, "Bad blend dest A factor in do_blend");
	       return;
         }

         /* Due to round-off problems we have to clamp against zero. */
         /* Optimization: we don't have to do this for all src & dst factors */
         if (dA < 0.0F)  dA = 0.0F;
         if (dR < 0.0F)  dR = 0.0F;
         if (dG < 0.0F)  dG = 0.0F;
         if (dB < 0.0F)  dB = 0.0F;
         if (sA < 0.0F)  sA = 0.0F;
         if (sR < 0.0F)  sR = 0.0F;
         if (sG < 0.0F)  sG = 0.0F;
         if (sB < 0.0F)  sB = 0.0F;

         ASSERT( sR <= 1.0 );
         ASSERT( sG <= 1.0 );
         ASSERT( sB <= 1.0 );
         ASSERT( sA <= 1.0 );
         ASSERT( dR <= 1.0 );
         ASSERT( dG <= 1.0 );
         ASSERT( dB <= 1.0 );
         ASSERT( dA <= 1.0 );

         /* compute blended color */
         if (ctx->Color.BlendEquation==GL_FUNC_ADD_EXT) {
            r = Rs * sR + Rd * dR + 0.5F;
            g = Gs * sG + Gd * dG + 0.5F;
            b = Bs * sB + Bd * dB + 0.5F;
            a = As * sA + Ad * dA + 0.5F;
         }
         else if (ctx->Color.BlendEquation==GL_FUNC_SUBTRACT_EXT) {
            r = Rs * sR - Rd * dR + 0.5F;
            g = Gs * sG - Gd * dG + 0.5F;
            b = Bs * sB - Bd * dB + 0.5F;
            a = As * sA - Ad * dA + 0.5F;
         }
         else if (ctx->Color.BlendEquation==GL_FUNC_REVERSE_SUBTRACT_EXT) {
            r = Rd * dR - Rs * sR + 0.5F;
            g = Gd * dG - Gs * sG + 0.5F;
            b = Bd * dB - Bs * sB + 0.5F;
            a = Ad * dA - As * sA + 0.5F;
         }
         else {
            /* should never get here */
            r = g = b = a = 0.0F;  /* silence uninitialized var warning */
            _mesa_problem(ctx, "unexpected BlendEquation in blend_general()");
         }

         /* final clamping */
         rgba[i][RCOMP] = (GLchan) (GLint) CLAMP( r, 0.0F, CHAN_MAXF );
         rgba[i][GCOMP] = (GLchan) (GLint) CLAMP( g, 0.0F, CHAN_MAXF );
         rgba[i][BCOMP] = (GLchan) (GLint) CLAMP( b, 0.0F, CHAN_MAXF );
         rgba[i][ACOMP] = (GLchan) (GLint) CLAMP( a, 0.0F, CHAN_MAXF );
      }
   }
}





/*
 * Analyze current blending parameters to pick fastest blending function.
 * Result: the ctx->Color.BlendFunc pointer is updated.
 */
void _swrast_choose_blend_func( GLcontext *ctx )
{
   const GLenum eq = ctx->Color.BlendEquation;
   const GLenum srcRGB = ctx->Color.BlendSrcRGB;
   const GLenum dstRGB = ctx->Color.BlendDstRGB;
   const GLenum srcA = ctx->Color.BlendSrcA;
   const GLenum dstA = ctx->Color.BlendDstA;

   if (srcRGB != srcA || dstRGB != dstA) {
      SWRAST_CONTEXT(ctx)->BlendFunc = blend_general;
   }
   else if (eq==GL_FUNC_ADD_EXT && srcRGB==GL_SRC_ALPHA
	    && dstRGB==GL_ONE_MINUS_SRC_ALPHA) {
#if defined(USE_MMX_ASM)
      if ( cpu_has_mmx ) {
	 SWRAST_CONTEXT(ctx)->BlendFunc = _mesa_mmx_blend_transparency;
      }
      else
#endif
	 SWRAST_CONTEXT(ctx)->BlendFunc = blend_transparency;
   }
   else if (eq==GL_FUNC_ADD_EXT && srcRGB==GL_ONE && dstRGB==GL_ONE) {
      SWRAST_CONTEXT(ctx)->BlendFunc = blend_add;
   }
   else if (((eq==GL_FUNC_ADD_EXT || eq==GL_FUNC_REVERSE_SUBTRACT_EXT)
	     && (srcRGB==GL_ZERO && dstRGB==GL_SRC_COLOR))
	    ||
	    ((eq==GL_FUNC_ADD_EXT || eq==GL_FUNC_SUBTRACT_EXT)
	     && (srcRGB==GL_DST_COLOR && dstRGB==GL_ZERO))) {
      SWRAST_CONTEXT(ctx)->BlendFunc = blend_modulate;
   }
   else if (eq==GL_MIN_EXT) {
      SWRAST_CONTEXT(ctx)->BlendFunc = blend_min;
   }
   else if (eq==GL_MAX_EXT) {
      SWRAST_CONTEXT(ctx)->BlendFunc = blend_max;
   }
   else if (eq==GL_FUNC_ADD_EXT && srcRGB == GL_ZERO && dstRGB == GL_ONE) {
      SWRAST_CONTEXT(ctx)->BlendFunc = blend_noop;
   }
   else if (eq==GL_FUNC_ADD_EXT && srcRGB == GL_ONE && dstRGB == GL_ZERO) {
      SWRAST_CONTEXT(ctx)->BlendFunc = blend_replace;
   }
   else {
      SWRAST_CONTEXT(ctx)->BlendFunc = blend_general;
   }
}



/*
 * Apply the blending operator to a span of pixels.
 * Input:  n - number of pixels in span
 *         x, y - location of leftmost pixel in span in window coords.
 *         mask - boolean mask indicating which pixels to blend.
 * In/Out:  rgba - pixel values
 */
__attribute__((hot)) void
_mesa_blend_span( GLcontext *ctx, GLuint n, GLint x, GLint y,
                  GLchan rgba[][4], const GLubyte mask[] )
{
   GLchan dest[MAX_WIDTH][4];

   /* Check if device driver can do the work */
   if (ctx->Color.BlendEquation==GL_LOGIC_OP &&
       !ctx->Color.ColorLogicOpEnabled) {
      return;
   }

   /* Read span of current frame buffer pixels */
   _mesa_read_rgba_span( ctx, ctx->DrawBuffer, n, x, y, dest );

   SWRAST_CONTEXT(ctx)->BlendFunc( ctx, n, mask, rgba,
				   (const GLchan (*)[4]) dest );
}



/*
 * Apply the blending operator to an array of pixels.
 * Input:  n - number of pixels in span
 *         x, y - array of pixel locations
 *         mask - boolean mask indicating which pixels to blend.
 * In/Out:  rgba - pixel values
 */
__attribute__((hot)) void
_mesa_blend_pixels( GLcontext *ctx,
                    GLuint n, const GLint x[], const GLint y[],
                    GLchan rgba[][4], const GLubyte mask[] )
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);
   GLchan dest[PB_SIZE][4];

   /* Check if device driver can do the work */
   if (ctx->Color.BlendEquation==GL_LOGIC_OP &&
       !ctx->Color.ColorLogicOpEnabled) {
      return;
   }

   /* Read pixels from current color buffer */
   (*swrast->Driver.ReadRGBAPixels)( ctx, n, x, y, dest, mask );
   if (swrast->_RasterMask & ALPHABUF_BIT) {
      _mesa_read_alpha_pixels( ctx, n, x, y, dest, mask );
   }

   swrast->BlendFunc( ctx, n, mask, rgba, (const GLchan (*)[4])dest );
}
