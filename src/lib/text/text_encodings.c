#include "text.h"
#include <stdint.h>

#define SRC ((const uint8_t*)src)
#define DST ((uint8_t*)dst)
#define CTX32 ((const uint32_t*)ctx)

/* Index or -1.
 */
 
static int codepage_search(const uint32_t *v,int c,uint32_t q) {
  int i=0;
  for (;i<c;i++,v++) if (*v==q) return i;
  return -1;
}

/* page128: 128-entry table, >=0x80 are illegal.
 */
 
int text_page128_read(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  *codepoint=SRC[0];
  if (*codepoint>=0x80) {
    (*codepoint)-=0x100;
  } else {
    *codepoint=CTX32[*codepoint];
  }
  return 1;
}

int text_page128_unread(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  *codepoint=SRC[srcc-1];
  if (*codepoint>=0x80) {
    (*codepoint)-=0x100;
  } else {
    *codepoint=CTX32[*codepoint];
  }
  return 1;
}

int text_page128_write(void *dst,int dsta,int codepoint,const void *ctx) {
  codepoint=codepage_search(ctx,128,codepoint);
  if (codepoint<0) return -1;
  if (dsta>=1) DST[0]=codepoint;
  return 1;
}

/* page256: 256-entry table.
 */
 
int text_page256_read(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  *codepoint=CTX32[SRC[0]];
  return 1;
}

int text_page256_unread(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  *codepoint=CTX32[SRC[srcc-1]];
  return 1;
}

int text_page256_write(void *dst,int dsta,int codepoint,const void *ctx) {
  codepoint=codepage_search(ctx,256,codepoint);
  if (codepoint<0) return -1;
  if (dsta>=1) DST[0]=codepoint;
  return 1;
}

/* asciiplus: <0x80 encode as-is, >=0x80 are in this 128-entry table.
 */
 
int text_asciiplus_read(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  *codepoint=SRC[0];
  if (*codepoint<0x80) return 1;
  *codepoint=CTX32[(*codepoint)-0x80];
  return 1;
}

int text_asciiplus_unread(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  *codepoint=SRC[srcc-1];
  if (*codepoint<0x80) return 1;
  *codepoint=CTX32[(*codepoint)-0x80];
  return 1;
}

int text_asciiplus_write(void *dst,int dsta,int codepoint,const void *ctx) {
  if (codepoint<0) return -1;
  if (codepoint<0x80) {
    if (dsta>=1) DST[0]=codepoint;
    return 1;
  }
  int v=codepage_search(ctx,128,codepoint);
  if (v<0) return -1;
  if (dsta>=1) DST[0]=0x80+v;
  return 1;
}

/* utf8: The most common encoding by far, and also the most interesting encoding-wise.
 */
 
static int text_utf8_read(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  if (!(SRC[0]&0x80)) {
    *codepoint=SRC[0];
    return 1;
  }
  if (!(SRC[0]&0x40)) {
    *codepoint=SRC[0]-0x100;
    return 1;
  }
  if (!(SRC[0]&0x20)) {
    if ((srcc<2)||((SRC[1]&0xc0)!=0x80)) {
      *codepoint=SRC[0]-0x100;
      return 1;
    }
    *codepoint=((SRC[0]&0x1f)<<6)|(SRC[1]&0x3f);
    return 2;
  }
  if (!(SRC[0]&0x10)) {
    if ((srcc<3)||((SRC[1]&0xc0)!=0x80)||((SRC[2]&0xc0)!=0x80)) {
      *codepoint=SRC[0]-0x100;
      return 1;
    }
    *codepoint=((SRC[0]&0x0f)<<12)|((SRC[1]&0x3f)<<6)|(SRC[2]&0x3f);
    return 3;
  }
  if (!(SRC[0]&0x08)) {
    if ((srcc<4)||((SRC[1]&0xc0)!=0x80)||((SRC[2]&0xc0)!=0x80)||((SRC[3]&0xc0)!=0x80)) {
      *codepoint=SRC[0]-0x100;
      return 1;
    }
    *codepoint=((SRC[0]&0x07)<<18)|((SRC[1]&0x3f)<<12)|((SRC[2]&0x3f)<<6)|(SRC[3]&0x3f);
    return 4;
  }
  *codepoint=SRC[0]-0x100;
  return 1;
}

static int text_utf8_unread(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  if (!(SRC[srcc-1]&0x80)) {
    *codepoint=SRC[srcc-1];
    return 1;
  }
  if ((srcc<2)||(SRC[srcc-1]&0xc0)!=0x80) {
    *codepoint=SRC[srcc-1]-0x100;
    return 1;
  }
  if (!(SRC[srcc-2]&0x40)) {
    *codepoint=((SRC[srcc-2]&0x3f)<<6)|(SRC[srcc-1]&0x3f);
    return 2;
  }
  if ((srcc<3)||((SRC[srcc-2]&0xc0)!=0x80)) {
    *codepoint=SRC[srcc-1]-0x100;
    return 1;
  }
  if (!(SRC[srcc-3]&0x20)) {
    *codepoint=((SRC[srcc-3]&0x1f)<<12)|((SRC[srcc-2]&0x3f)<<6)|(SRC[srcc-2]&0x3f);
    return 3;
  }
  if ((srcc<4)||((SRC[srcc-3]&0xc0)!=0x80)) {
    *codepoint=SRC[srcc-1]-0x100;
    return 1;
  }
  if (!(SRC[srcc-4]&0x10)) {
    *codepoint=((SRC[srcc-4]&0x0f)<<18)|((SRC[srcc-3]&0x3f)<<12)|((SRC[srcc-2]&0x3f)<<6)|(SRC[srcc-1]&0x3f);
    return 4;
  }
  *codepoint=SRC[srcc-1]-0x100;
  return 1;
}

static int text_utf8_write(void *dst,int dsta,int codepoint,const void *ctx) {
  if (codepoint<0) return -1;
  if (codepoint<0x80) {
    if (dsta>=1) {
      DST[0]=codepoint;
    }
    return 1;
  }
  if (codepoint<0x800) {
    if (dsta>=2) {
      DST[0]=0xc0|(codepoint>>6);
      DST[1]=0x80|(codepoint&0x3f);
    }
    return 2;
  }
  if (codepoint<0x10000) {
    if (dsta>=3) {
      DST[0]=0xe0|(codepoint>>12);
      DST[1]=0x80|((codepoint>>6)&0x3f);
      DST[2]=0x80|(codepoint&0x3f);
    }
    return 3;
  }
  if (codepoint<0x200000) {
    // Per spec, anything over 0x10ffff is illegal. But we don't enforce that when decoding so we won't here either.
    if (dsta>=4) {
      DST[0]=0xf0|(codepoint>>18);
      DST[1]=0x80|((codepoint>>12)&0x3f);
      DST[2]=0x80|((codepoint>>6)&0x3f);
      DST[3]=0x80|(codepoint&0x3f);
    }
    return 4;
  }
  return -1;
}

const struct text_encoding text_encoding_utf8={
  .name="utf8",
  .ctx=0,
  .read=text_utf8_read,
  .unread=text_utf8_unread,
  .write=text_utf8_write,
};

/* iso88591: Each byte is a codepoint directly.
 */
 
static int text_iso88591_read(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  *codepoint=SRC[0];
  return 1;
}

static int text_iso88591_unread(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  *codepoint=SRC[srcc-1];
  return 1;
}

static int text_iso88591_write(void *dst,int dsta,int codepoint,const void *ctx) {
  if ((codepoint<0)||(codepoint>0xff)) return -1;
  if (dsta>=1) DST[0]=codepoint;
  return 1;
}

const struct text_encoding text_encoding_iso88591={
  .name="iso88591",
  .ctx=0,
  .read=text_iso88591_read,
  .unread=text_iso88591_unread,
  .write=text_iso88591_write,
};

/* utf16(le,be): What happens when you first assume that 64k is plenty of space,
 * and then spend two decades refusing to admit you were wrong.
 */
 
static int text_utf16le_read(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<2) {
    if (srcc>=1) {
      *codepoint=SRC[0]-0x100;
      return 1;
    }
    return 0;
  }
  *codepoint=SRC[0]|(SRC[1]<<8);
  if ((*codepoint>=0xd800)&&(*codepoint<0xdc00)&&(srcc>=4)) {
    int lo=SRC[2]|(SRC[3]<<8);
    if ((lo>=0xdc00)&&(lo<0xe000)) {
      *codepoint=0x10000+(((*codepoint)&0x3ff)<<10)+(lo&0x3ff);
      return 4;
    }
  }
  return 2;
}

static int text_utf16le_unread(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<2) {
    if (srcc>=1) {
      *codepoint=SRC[srcc-1]-0x100;
      return 1;
    }
    return 0;
  }
  *codepoint=SRC[srcc-2]|(SRC[srcc-1]<<8);
  if ((*codepoint>=0xdc00)&&(*codepoint<0xe000)&&(srcc>=4)) {
    int hi=SRC[srcc-4]|(SRC[srcc-3]<<8);
    if ((hi>=0xd800)&&(hi<0xdc00)) {
      *codepoint=0x10000+((hi&0x3ff)<<10)+((*codepoint)&0x3ff);
      return 4;
    }
  }
  return 2;
}

static int text_utf16le_write(void *dst,int dsta,int codepoint,const void *ctx) {
  if (codepoint<0) return -1;
  if (codepoint<0x10000) {
    if (dsta>=2) {
      DST[0]=codepoint;
      DST[1]=codepoint>>8;
    }
    return 2;
  }
  if (codepoint<0x110000) {
    if (dsta>=4) {
      int hi=0xd800|(codepoint>>10);
      int lo=0xdc00|(codepoint&0x3ff);
      DST[0]=hi;
      DST[1]=hi>>8;
      DST[2]=lo;
      DST[3]=lo>>8;
    }
    return 4;
  }
  return -1;
}

const struct text_encoding text_encoding_utf16le={
  .name="utf16le",
  .ctx=0,
  .read=text_utf16le_read,
  .unread=text_utf16le_unread,
  .write=text_utf16le_write,
};
 
static int text_utf16be_read(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<2) {
    if (srcc>=1) {
      *codepoint=SRC[0]-0x100;
      return 1;
    }
    return 0;
  }
  *codepoint=(SRC[0]<<8)|SRC[1];
  if ((*codepoint>=0xd800)&&(*codepoint<0xdc00)&&(srcc>=4)) {
    int lo=(SRC[2]<<8)|SRC[3];
    if ((lo>=0xdc00)&&(lo<0xe000)) {
      *codepoint=0x10000+(((*codepoint)&0x3ff)<<10)+(lo&0x3ff);
      return 4;
    }
  }
  return 2;
}

static int text_utf16be_unread(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<2) {
    if (srcc>=1) {
      *codepoint=SRC[srcc-1]-0x100;
      return 1;
    }
    return 0;
  }
  *codepoint=(SRC[srcc-2]<<8)|SRC[srcc-1];
  if ((*codepoint>=0xdc00)&&(*codepoint<0xe000)&&(srcc>=4)) {
    int hi=(SRC[srcc-4]<<8)|SRC[srcc-3];
    if ((hi>=0xd800)&&(hi<0xdc00)) {
      *codepoint=0x10000+((hi&0x3ff)<<10)+((*codepoint)&0x3ff);
      return 4;
    }
  }
  return 2;
}

static int text_utf16be_write(void *dst,int dsta,int codepoint,const void *ctx) {
  if (codepoint<0) return -1;
  if (codepoint<0x10000) {
    if (dsta>=2) {
      DST[0]=codepoint>>8;
      DST[1]=codepoint;
    }
    return 2;
  }
  if (codepoint<0x110000) {
    if (dsta>=4) {
      int hi=0xd800|(codepoint>>10);
      int lo=0xdc00|(codepoint&0x3ff);
      DST[0]=hi>>8;
      DST[1]=hi;
      DST[2]=lo>>8;
      DST[3]=lo;
    }
    return 4;
  }
  return -1;
}

const struct text_encoding text_encoding_utf16be={
  .name="utf16be",
  .ctx=0,
  .read=text_utf16be_read,
  .unread=text_utf16be_unread,
  .write=text_utf16be_write,
};

/* ucs(2,3,4)(le,be): Codepoints stored directly.
 */
 
static int text_ucs2le_read(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  if (srcc<2) {
    *codepoint=SRC[0]-0x100;
    return 1;
  }
  *codepoint=SRC[0]|(SRC[1]<<8);
  return 2;
}

static int text_ucs2le_unread(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  if (srcc<2) {
    *codepoint=SRC[srcc-1]-0x100;
    return 1;
  }
  *codepoint=SRC[srcc-2]|(SRC[srcc-1]<<8);
  return 2;
}

static int text_ucs2le_write(void *dst,int dsta,int codepoint,const void *ctx) {
  if ((codepoint<0)||(codepoint>0xffff)) return -1;
  if (dsta>=2) {
    DST[0]=codepoint;
    DST[1]=codepoint>>8;
  }
  return 2;
}

const struct text_encoding text_encoding_ucs2le={
  .name="ucs2le",
  .ctx=0,
  .read=text_ucs2le_read,
  .unread=text_ucs2le_unread,
  .write=text_ucs2le_write,
};
 
static int text_ucs2be_read(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  if (srcc<2) {
    *codepoint=SRC[0]-0x100;
    return 1;
  }
  *codepoint=(SRC[0]<<8)|SRC[1];
  return 2;
}

static int text_ucs2be_unread(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  if (srcc<2) {
    *codepoint=SRC[srcc-1]-0x100;
    return 1;
  }
  *codepoint=(SRC[srcc-2]<<8)|SRC[srcc-1];
  return 2;
}

static int text_ucs2be_write(void *dst,int dsta,int codepoint,const void *ctx) {
  if ((codepoint<0)||(codepoint>0xffff)) return -1;
  if (dsta>=2) {
    DST[0]=codepoint>>8;
    DST[1]=codepoint;
  }
  return 2;
}

const struct text_encoding text_encoding_ucs2be={
  .name="ucs2be",
  .ctx=0,
  .read=text_ucs2be_read,
  .unread=text_ucs2be_unread,
  .write=text_ucs2be_write,
};
 
static int text_ucs3le_read(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  if (srcc<3) {
    *codepoint=SRC[0]-0x100;
    return 1;
  }
  *codepoint=SRC[0]|(SRC[1]<<8)|(SRC[2]<<16);
  return 3;
}

static int text_ucs3le_unread(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  if (srcc<3) {
    *codepoint=SRC[srcc-1]-0x100;
    return 1;
  }
  *codepoint=SRC[srcc-3]|(SRC[srcc-2]<<8)|(SRC[srcc-1]<<16);
  return 3;
}

static int text_ucs3le_write(void *dst,int dsta,int codepoint,const void *ctx) {
  if ((codepoint<0)||(codepoint>0xffffff)) return 0;
  if (dsta>=3) {
    DST[0]=codepoint;
    DST[1]=codepoint>>8;
    DST[2]=codepoint>>16;
  }
  return 3;
}

const struct text_encoding text_encoding_ucs3le={
  .name="ucs3le",
  .ctx=0,
  .read=text_ucs3le_read,
  .unread=text_ucs3le_unread,
  .write=text_ucs3le_write,
};
 
static int text_ucs3be_read(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  if (srcc<3) {
    *codepoint=SRC[0]-0x100;
    return 1;
  }
  *codepoint=(SRC[0]<<16)|(SRC[1]<<8)|SRC[2];
  return 3;
}

static int text_ucs3be_unread(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  if (srcc<3) {
    *codepoint=SRC[srcc-1]-0x100;
    return 1;
  }
  *codepoint=(SRC[srcc-3]<<16)|(SRC[srcc-2]<<8)|SRC[srcc-1];
  return 3;
}

static int text_ucs3be_write(void *dst,int dsta,int codepoint,const void *ctx) {
  if ((codepoint<0)||(codepoint>0xffffff)) return -1;
  if (dsta>=3) {
    DST[0]=codepoint>>16;
    DST[1]=codepoint>>8;
    DST[2]=codepoint;
  }
  return 3;
}

const struct text_encoding text_encoding_ucs3be={
  .name="ucs3be",
  .ctx=0,
  .read=text_ucs3be_read,
  .unread=text_ucs3be_unread,
  .write=text_ucs3be_write,
};
 
static int text_ucs4le_read(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  if (srcc<4) {
    *codepoint=SRC[0]-0x100;
    return 1;
  }
  *codepoint=SRC[0]|(SRC[1]<<8)|(SRC[2]<<16)|(SRC[3]<<24);
  return 4;
}

static int text_ucs4le_unread(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  if (srcc<4) {
    *codepoint=SRC[srcc-1]-0x100;
    return 1;
  }
  *codepoint=SRC[srcc-4]|(SRC[srcc-3]<<8)|(SRC[srcc-2]<<16)|(SRC[srcc-3]<<24);
  return 4;
}

static int text_ucs4le_write(void *dst,int dsta,int codepoint,const void *ctx) {
  if (dsta>=4) {
    DST[0]=codepoint;
    DST[1]=codepoint>>8;
    DST[2]=codepoint>>16;
    DST[3]=codepoint>>24;
  }
  return 4;
}

const struct text_encoding text_encoding_ucs4le={
  .name="ucs4le",
  .ctx=0,
  .read=text_ucs4le_read,
  .unread=text_ucs4le_unread,
  .write=text_ucs4le_write,
};
 
static int text_ucs4be_read(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  if (srcc<4) {
    *codepoint=SRC[0]-0x100;
    return 1;
  }
  *codepoint=(SRC[0]<<24)|(SRC[1]<<16)|(SRC[2]<<8)|SRC[3];
  return 4;
}

static int text_ucs4be_unread(int *codepoint,const void *src,int srcc,const void *ctx) {
  if (srcc<1) return 0;
  if (srcc<4) {
    *codepoint=SRC[srcc-1]-0x100;
    return 1;
  }
  *codepoint=(SRC[srcc-4]<<24)|(SRC[srcc-3]<<16)|(SRC[srcc-2]<<8)|SRC[srcc-3];
  return 4;
}

static int text_ucs4be_write(void *dst,int dsta,int codepoint,const void *ctx) {
  if (dsta>=4) {
    DST[0]=codepoint>>24;
    DST[1]=codepoint>>16;
    DST[2]=codepoint>>8;
    DST[3]=codepoint;
  }
  return 4;
}

const struct text_encoding text_encoding_ucs4be={
  .name="ucs4be",
  .ctx=0,
  .read=text_ucs4be_read,
  .unread=text_ucs4be_unread,
  .write=text_ucs4be_write,
};
