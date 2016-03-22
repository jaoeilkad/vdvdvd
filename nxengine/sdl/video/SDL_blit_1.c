/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "LRSDL_config.h"

#include "LRSDL_video.h"
#include "SDL_blit.h"
#include "SDL_sysvideo.h"
#include "LRSDL_endian.h"

/* Functions to blit from 8-bit surfaces to other surfaces */

static void Blit1to1(SDL_BlitInfo *info)
{
	int width     = info->d_width;
	int height    = info->d_height;
	uint8_t *src  = info->s_pixels;
	int srcskip   = info->s_skip;
	uint8_t *dst  = info->d_pixels;
	int dstskip   = info->d_skip;
	uint8_t *map  = info->table;

	while ( height-- )
   {
      int c;
      for ( c=width; c; --c )
      {
         *dst = map[*src];
         dst++;
         src++;
      }
      src += srcskip;
      dst += dstskip;
   }
}

#ifdef MSB_FIRST
#define HI	0
#define LO	1
#else
#define HI	1
#define LO	0
#endif

static void Blit1to2(SDL_BlitInfo *info)
{
   int c;
	int width     = info->d_width;
	int height    = info->d_height;
	uint8_t *src  = info->s_pixels;
	int srcskip   = info->s_skip;
	uint8_t *dst  = info->d_pixels;
	int dstskip   = info->d_skip;
	uint16_t *map = (uint16_t *)info->table;

	/* Memory align at 4-byte boundary, if necessary */
	if ( (long)dst & 0x03 )
   {
      /* Don't do anything if width is 0 */
      if ( width == 0 )
         return;
      --width;

      while ( height-- )
      {

         /* Perform copy alignment */
         *(uint16_t*)dst = map[*src++];
         dst += 2;

         /* Copy in 4 pixel chunks */
         for ( c=width/4; c; --c )
         {
            *(uint32_t*)dst =
               (map[src[HI]]<<16)|(map[src[LO]]);
            src += 2;
            dst += 4;
            *(uint32_t*)dst =
               (map[src[HI]]<<16)|(map[src[LO]]);
            src += 2;
            dst += 4;
         }

         /* Get any leftovers */
         switch (width & 3)
         {
            case 3:
               *(uint16_t *)dst = map[*src++];
               dst += 2;
            case 2:
               *(uint32_t*)dst =
                  (map[src[HI]]<<16)|(map[src[LO]]);
               src += 2;
               dst += 4;
               break;
            case 1:
               *(uint16_t *)dst = map[*src++];
               dst += 2;
               break;
         }
         src += srcskip;
         dst += dstskip;
      }
   }
   else
   { 
		while ( height-- )
      {
         /* Copy in 4 pixel chunks */
         for ( c=width/4; c; --c )
         {
            *(uint32_t*)dst =
               (map[src[HI]]<<16)|(map[src[LO]]);
            src += 2;
            dst += 4;
            *(uint32_t*)dst =
               (map[src[HI]]<<16)|(map[src[LO]]);
            src += 2;
            dst += 4;
         }

         /* Get any leftovers */
         switch (width & 3)
         {
            case 3:
               *(uint16_t *)dst = map[*src++];
               dst += 2;
            case 2:
               *(uint32_t*)dst =
                  (map[src[HI]]<<16)|(map[src[LO]]);
               src += 2;
               dst += 4;
               break;
            case 1:
               *(uint16_t *)dst = map[*src++];
               dst += 2;
               break;
         }

         src += srcskip;
         dst += dstskip;
      }
	}
}
static void Blit1to3(SDL_BlitInfo *info)
{
   int width    = info->d_width;
	int height   = info->d_height;
	uint8_t *src = info->s_pixels;
	int srcskip  = info->s_skip;
	uint8_t *dst = info->d_pixels;
	int dstskip  = info->d_skip;
	uint8_t *map = info->table;

	while ( height-- )
   {
      int c;
		for ( c=width; c; --c )
      {
			int  o = *src * 4;
			dst[0] = map[o++];
			dst[1] = map[o++];
			dst[2] = map[o++];
			src++;
			dst += 3;
		}
		src += srcskip;
		dst += dstskip;
	}
}
static void Blit1to4(SDL_BlitInfo *info)
{
	int width     = info->d_width;
	int height    = info->d_height;
	uint8_t *src  = info->s_pixels;
	int srcskip   = info->s_skip;
	uint32_t *dst = (uint32_t*)info->d_pixels;
	int dstskip   = info->d_skip/4;
	uint32_t *map = (uint32_t*)info->table;

	while ( height-- )
   {
      int c;
		for ( c=width/4; c; --c )
      {
			*dst++ = map[*src++];
			*dst++ = map[*src++];
			*dst++ = map[*src++];
			*dst++ = map[*src++];
		}

      switch ( width & 3 )
      {
         case 3:
            *dst++ = map[*src++];
         case 2:
            *dst++ = map[*src++];
         case 1:
            *dst++ = map[*src++];
      }

		src += srcskip;
		dst += dstskip;
	}
}

static void Blit1to1Key(SDL_BlitInfo *info)
{
	int width       = info->d_width;
	int height      = info->d_height;
	uint8_t *src    = info->s_pixels;
	int srcskip     = info->s_skip;
	uint8_t *dst    = info->d_pixels;
	int dstskip     = info->d_skip;
	uint8_t *palmap = info->table;
	uint32_t ckey   = info->src->colorkey;
        
	if ( palmap )
   {
      while ( height-- )
      {
         DUFFS_LOOP(
               {
               if ( *src != ckey ) {
               *dst = palmap[*src];
               }
               dst++;
               src++;
               },
               width);
         src += srcskip;
         dst += dstskip;
      }
   }
   else
   {
      while ( height-- )
      {
         DUFFS_LOOP(
               {
               if ( *src != ckey ) {
               *dst = *src;
               }
               dst++;
               src++;
               },
               width);
         src += srcskip;
         dst += dstskip;
      }
   }
}

static void Blit1to2Key(SDL_BlitInfo *info)
{
	int width        = info->d_width;
	int height       = info->d_height;
	uint8_t *src     = info->s_pixels;
	int srcskip      = info->s_skip;
	uint16_t *dstp   = (uint16_t*)info->d_pixels;
	int dstskip      = info->d_skip / 2;
	uint16_t *palmap = (uint16_t*)info->table;
	uint32_t ckey    = info->src->colorkey;

	while ( height-- )
   {
      DUFFS_LOOP(
            {
            if ( *src != ckey ) {
            *dstp=palmap[*src];
            }
            src++;
            dstp++;
            },
            width);

      src  += srcskip;
      dstp += dstskip;
   }
}

static void Blit1to3Key(SDL_BlitInfo *info)
{
	int o;
	int width       = info->d_width;
	int height      = info->d_height;
	uint8_t *src    = info->s_pixels;
	int srcskip     = info->s_skip;
	uint8_t *dst    = info->d_pixels;
	int dstskip     = info->d_skip;
	uint8_t *palmap = info->table;
	uint32_t ckey   = info->src->colorkey;

	while ( height-- )
   {
      DUFFS_LOOP(
            {
            if ( *src != ckey ) {
            o = *src * 4;
            dst[0] = palmap[o++];
            dst[1] = palmap[o++];
            dst[2] = palmap[o++];
            }
            src++;
            dst += 3;
            },
            width);
      src += srcskip;
      dst += dstskip;
   }
}

static void Blit1to4Key(SDL_BlitInfo *info)
{
	int width        = info->d_width;
	int height       = info->d_height;
	uint8_t *src     = info->s_pixels;
	int srcskip      = info->s_skip;
	uint32_t *dstp   = (uint32_t *)info->d_pixels;
	int dstskip      = info->d_skip / 4;
	uint32_t *palmap = (uint32_t *)info->table;
	uint32_t ckey    = info->src->colorkey;

	while ( height-- )
   {
      DUFFS_LOOP(
            {
            if ( *src != ckey ) {
            *dstp = palmap[*src];
            }
            src++;
            dstp++;
            },
            width);

      src  += srcskip;
      dstp += dstskip;
   }
}

static void Blit1toNAlpha(SDL_BlitInfo *info)
{
	int width               = info->d_width;
	int height              = info->d_height;
	uint8_t *src            = info->s_pixels;
	int srcskip             = info->s_skip;
	uint8_t *dst            = info->d_pixels;
	int dstskip             = info->d_skip;
	SDL_PixelFormat *dstfmt = info->dst;
	const SDL_Color *srcpal	= info->src->palette->colors;
	const int A             = info->src->alpha;
   int dstbpp              = dstfmt->BytesPerPixel;

	while ( height-- )
   {
      int sR, sG, sB;
      int dR, dG, dB;

      DUFFS_LOOP(
            {
            uint32_t pixel;
            sR = srcpal[*src].r;
            sG = srcpal[*src].g;
            sB = srcpal[*src].b;
            DISEMBLE_RGB(dst, dstbpp, dstfmt,
                  pixel, dR, dG, dB);
            ALPHA_BLEND(sR, sG, sB, A, dR, dG, dB);
            ASSEMBLE_RGB(dst, dstbpp, dstfmt, dR, dG, dB);
            src++;
            dst += dstbpp;
            },
            width);

      src += srcskip;
      dst += dstskip;
   }
}

static void Blit1toNAlphaKey(SDL_BlitInfo *info)
{
	int width               = info->d_width;
	int height              = info->d_height;
	uint8_t *src            = info->s_pixels;
	int srcskip             = info->s_skip;
	uint8_t *dst            = info->d_pixels;
	int dstskip             = info->d_skip;
	SDL_PixelFormat *srcfmt = info->src;
	SDL_PixelFormat *dstfmt = info->dst;
	const SDL_Color *srcpal	= info->src->palette->colors;
	uint32_t ckey           = srcfmt->colorkey;
	const int A             = srcfmt->alpha;
	int dstbpp              = dstfmt->BytesPerPixel;

	while ( height-- )
   {
      int sR, sG, sB;
      int dR, dG, dB;
      DUFFS_LOOP(
            {
            if ( *src != ckey ) {
            uint32_t pixel;
            sR = srcpal[*src].r;
            sG = srcpal[*src].g;
            sB = srcpal[*src].b;
            DISEMBLE_RGB(dst, dstbpp, dstfmt,
                  pixel, dR, dG, dB);
            ALPHA_BLEND(sR, sG, sB, A, dR, dG, dB);
            ASSEMBLE_RGB(dst, dstbpp, dstfmt, dR, dG, dB);
            }
            src++;
            dst += dstbpp;
            },
            width);
      src += srcskip;
      dst += dstskip;
   }
}

static SDL_loblit one_blit[] = {
	NULL, Blit1to1, Blit1to2, Blit1to3, Blit1to4
};

static SDL_loblit one_blitkey[] = {
        NULL, Blit1to1Key, Blit1to2Key, Blit1to3Key, Blit1to4Key
};

SDL_loblit LRSDL_CalculateBlit1(SDL_Surface *surface, int blit_index)
{
   int               which = 0;
   SDL_PixelFormat *dstfmt = surface->map->dst->format;

   if ( dstfmt->BitsPerPixel >= 8 )
      which = dstfmt->BytesPerPixel;

   switch(blit_index)
   {
      case 0:			/* copy */
         return one_blit[which];

      case 1:			/* colorkey */
         return one_blitkey[which];

      case 2:			/* alpha */
         /* Supporting 8bpp->8bpp alpha is doable but requires lots of
            tables which consume space and takes time to precompute,
            so is better left to the user */
         if (which >= 2)
            return Blit1toNAlpha;
         break;
      case 3:			/* alpha + colorkey */
         if (which >= 2)
            return Blit1toNAlphaKey;
         break;
   }

   return NULL;
}
