#include "font_internal.h"

/* Measure string.
 */

int font_measure_string(struct font *font,const char *src,int srcc) {
  if (!font) return 0;
  if (!src) return 0;
  if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  struct text_decoder decoder={.v=src,.c=srcc,.encoding=font->encoding};
  int w=0,codepoint;
  while (text_decoder_read(&codepoint,&decoder)>0) {
    if (codepoint<0) {
      codepoint+=0x100;
      w+=font_measure_tofu(font,codepoint);
    } else {
      if ((codepoint>=0x20)&&(codepoint<=0x7f)) w+=font->w;
      else w+=font_measure_tofu(font,codepoint);
    }
  }
  return w;
}

/* Reverse measure string.
 */
 
int font_locate_point(struct font *font,const char *src,int srcc,int x) {
  if (!font) return 0;
  if (x<=0) return 0;
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  struct text_decoder decoder={.v=src,.c=srcc,.encoding=font->encoding};
  int w=0,codepoint,pvp=0;
  while (text_decoder_read(&codepoint,&decoder)>0) {
    int pvw=w;
    if (codepoint<0) {
      codepoint+=0x100;
      w+=font_measure_tofu(font,codepoint);
    } else {
      if ((codepoint>=0x20)&&(codepoint<=0x7f)) w+=font->w;
      else w+=font_measure_tofu(font,codepoint);
    }
    if (w>=x) { // Crossed the target. Return either (pvp) or (decoder.p).
      int midx=(pvw+w)>>1;
      if (x>=midx) return decoder.p;
      return pvp;
    }
    pvp=decoder.p;
  }
  return decoder.p;
}

/* Measure tofu.
 */
 
int font_measure_tofu(struct font *font,int codepoint) {
  if (!font) return 0;
  //TODO tofu
  return 0;
}

/* Render multiple glyphs, allowing embedded tofu.
 */

int font_render_string(struct image *dst,int dstx,int dsty,struct font *font,const char *src,int srcc) {
  if (!font) return 0;
  if (!src) return 0;
  
  // A wee optimization: If we can tell the whole row is oob, don't bother.
  int adjy=dsty+dst->y0;
  if ((adjy>=dst->h)||(adjy+font->h<=0)) return font_measure_string(font,src,srcc);
  
  if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  struct text_decoder decoder={.v=src,.c=srcc,.encoding=font->encoding};
  int subx=0,codepoint;
  while (text_decoder_read(&codepoint,&decoder)>0) {
    if (codepoint<0) {
      codepoint+=0x100;
      subx+=font_render_tofu(dst,dstx+subx,dsty,font,codepoint,font->color_misencode);
    } else {
      if ((codepoint<0x20)||(codepoint>0x7f)) {
        subx+=font_render_tofu(dst,dstx+subx,dsty,font,codepoint,font->color_missing);
      } else {
        subx+=font_render_glyph(dst,dstx+subx,dsty,font,codepoint,font->color_normal);
      }
    }
  }
  return subx;
}
