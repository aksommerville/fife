#include "font_internal.h"

/* Measure string.
 */

int font_measure_string(struct font *font,const char *src,int srcc) {
  if (!font) return 0;
  if (!src) return 0;
  if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  int w=0,srcp=0,err,codepoint;
  while (srcp<srcc) {
    if ((err=font->decode(&codepoint,src+srcp,srcc-srcp))<1) {
      w+=font_measure_tofu(font,(unsigned char)src[srcp]);
      srcp++;
    } else {
      srcp+=err;
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
  if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  int subx=0,srcp=0,err,codepoint;
  while (srcp<srcc) {
    if ((err=font->decode(&codepoint,src+srcp,srcc-srcp))>0) {
      srcp+=err;
      if ((codepoint<0x20)||(codepoint>0x7f)) {
        subx+=font_render_tofu(dst,dstx+subx,dsty,font,codepoint,font->color_missing);
      } else {
        subx+=font_render_glyph(dst,dstx+subx,dsty,font,codepoint,font->color_normal);
      }
    } else {
      codepoint=(unsigned char)src[srcp];
      srcp++;
      subx+=font_render_tofu(dst,dstx+subx,dsty,font,codepoint,font->color_misencode);
    }
  }
  return subx;
}

/* Decode UTF-8.
 */

int font_utf8_decode(int *codepoint,const char *src,int srcc) {
  if (!src) return -1;
  if (srcc<1) return -1;
  const uint8_t *SRC=(const uint8_t*)src;
  if (!(SRC[0]&0x80)) {
    *codepoint=SRC[0];
    return 1;
  }
  if ((SRC[0]&0xc0)==0x80) {
    return -1;
  }
  if ((SRC[0]&0xe0)==0xc0) {
    if (srcc<2) return -1;
    if ((SRC[1]&0xc0)!=0x80) return -1;
    *codepoint=((SRC[0]&0x1f)<<6)|(SRC[1]&0x3f);
    return 2;
  }
  if ((SRC[0]&0xf0)==0xe0) {
    if (srcc<3) return -1;
    if ((SRC[1]&0xc0)!=0x80) return -1;
    if ((SRC[2]&0xc0)!=0x80) return -1;
    *codepoint=((SRC[0]&0x0f)<<12)|((SRC[1]&0x3f)<<6)|(SRC[2]&0x3f);
    return 3;
  }
  if ((SRC[0]&0xf8)==0xf0) {
    if (srcc<4) return -1;
    if ((SRC[1]&0xc0)!=0x80) return -1;
    if ((SRC[2]&0xc0)!=0x80) return -1;
    if ((SRC[3]&0xc0)!=0x80) return -1;
    *codepoint=((SRC[0]&0x07)<<18)|((SRC[1]&0x3f)<<12)|((SRC[2]&0x3f)<<6)|(SRC[3]&0x3f);
    return 4;
  }
  return -1;
}
