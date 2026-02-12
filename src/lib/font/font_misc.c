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
