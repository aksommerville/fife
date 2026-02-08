#include "font_internal.h"

/* Copy image into font, forcing to A1.
 * Caller must initialize font to image's size, and leave (img) clear initially.
 */
 
void font_copy_image(struct font *font,struct image *image) {
  if (!font||!image) return;
  if ((font->imgw!=image->w)||(font->imgh!=image->h)) return;
  
  // Assume that 1-bit images are positive==opaque.
  if (image->pixelsize==1) {
    if (font->imgstride==image->stride) {
      memcpy(font->img,image->v,image->stride*image->h);
    } else {
      uint8_t *dstrow=font->img;
      const uint8_t *srcrow=image->v;
      int yi=image->h;
      for (;yi-->0;dstrow+=font->imgstride,srcrow+=image->stride) {
        memcpy(dstrow,srcrow,font->imgstride);
      }
    }
    return;
  }
  
  // If not 1-bit, we require 8, 16, or 32.
  // Only natural zeroes are transparent, all else is uniformly opaque.
  uint8_t *dstrow=font->img;
  int yi=image->h;
  if (image->pixelsize==8) {
    const uint8_t *srcrow=image->v;
    for (;yi-->0;dstrow+=font->imgstride,srcrow+=image->stride) {
      uint8_t *dstp=dstrow;
      uint8_t dstmask=0x80;
      const uint8_t *srcp=srcrow;
      int xi=image->w; for (;xi-->0;srcp++) {
        if (*srcp) (*dstp)|=dstmask;
        if (dstmask==1) {
          dstmask=0x80;
          dstp++;
        } else {
          dstmask>>=1;
        }
      }
    }
  } else if (image->pixelsize==16) {
    int srcstridewords=image->stride>>1;
    const uint16_t *srcrow=image->v;
    for (;yi-->0;dstrow+=font->imgstride,srcrow+=srcstridewords) {
      uint8_t *dstp=dstrow;
      uint8_t dstmask=0x80;
      const uint16_t *srcp=srcrow;
      int xi=image->w; for (;xi-->0;srcp++) {
        if (*srcp) (*dstp)|=dstmask;
        if (dstmask==1) {
          dstmask=0x80;
          dstp++;
        } else {
          dstmask>>=1;
        }
      }
    }
  } else if (image->pixelsize==32) {
    int srcstridewords=image->stride>>2;
    const uint32_t *srcrow=image->v;
    for (;yi-->0;dstrow+=font->imgstride,srcrow+=srcstridewords) {
      uint8_t *dstp=dstrow;
      uint8_t dstmask=0x80;
      const uint32_t *srcp=srcrow;
      int xi=image->w; for (;xi-->0;srcp++) {
        if (*srcp) (*dstp)|=dstmask;
        if (dstmask==1) {
          dstmask=0x80;
          dstp++;
        } else {
          dstmask>>=1;
        }
      }
    }
  }
}

/* Render glyph.
 */

int font_render_glyph(struct image *dst,int dstx,int dsty,struct font *font,int codepoint,uint32_t color) {
  if (!dst||!dst->writeable||(dst->pixelsize!=32)) return -1;
  if (!font||(codepoint<0x20)||(codepoint>0x7f)) return -1;
  dstx+=dst->x0;
  dsty+=dst->y0;
  int srcx=(codepoint&15)*font->w;
  int srcy=((codepoint-0x20)>>4)*font->h;
  int w=font->w; // Will be the amount to actually copy. Return value is always (font->w).
  int h=font->h;
  if (dstx<0) { w+=dstx; srcx-=dstx; dstx=0; }
  if (dsty<0) { h+=dsty; srcy-=dsty; dsty=0; }
  if (dstx>dst->w-w) w=dst->w-dstx;
  if (dsty>dst->h-h) h=dst->h-dsty;
  if ((w<1)||(h<1)) return font->w;
  
  int dststridewords=dst->stride>>2;
  uint32_t *dstrow=((uint32_t*)dst->v)+dsty*dststridewords+dstx;
  const uint8_t *srcrow=font->img+srcy*font->imgstride+(srcx>>3);
  uint8_t srcmask0=0x80>>(srcx&7);
  int yi=h; for (;yi-->0;dstrow+=dststridewords,srcrow+=font->imgstride) {
    uint32_t *dstp=dstrow;
    const uint8_t *srcp=srcrow;
    uint8_t srcmask=srcmask0;
    int xi=w; for (;xi-->0;dstp++) {
      if ((*srcp)&srcmask) *dstp=color;
      if (srcmask==1) {
        srcmask=0x80;
        srcp++;
      } else {
        srcmask>>=1;
      }
    }
  }
  return font->w;
}

/* Render tofu.
 */
 
int font_render_tofu(struct image *dst,int dstx,int dsty,struct font *font,int codepoint,uint32_t color) {
  if (!dst||!dst->writeable||(dst->pixelsize!=32)) return -1;
  if (!font) return -1;
  dstx+=dst->x0;
  dsty+=dst->y0;
  //TODO tofu
  return 0;
}
