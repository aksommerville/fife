#include "serial.h"
#include <string.h>
#include <limits.h>

/* UTF-8.
 */
 
int sr_utf8_decode(int *dst,const void *src,int srcc) {
  if (!src||(srcc<1)) return -1;
  const uint8_t *SRC=src;
  if (!(SRC[0]&0x80)) {
    *dst=SRC[0];
    return 1;
  }
  if (!(SRC[0]&0x40)) {
    return -1;
  }
  if (!(SRC[0]&0x20)) {
    if (srcc<2) return -1;
    if ((SRC[1]&0xc0)!=0x80) return -1;
    *dst=((SRC[0]&0x1f)<<6)|(SRC[1]&0x3f);
    return 2;
  }
  if (!(SRC[0]&0x10)) {
    if (srcc<3) return -1;
    if ((SRC[1]&0xc0)!=0x80) return -1;
    if ((SRC[2]&0xc0)!=0x80) return -1;
    *dst=((SRC[0]&0x0f)<<12)|((SRC[1]&0x3f)<<6)|(SRC[2]&0x3f);
    return 3;
  }
  if (!(SRC[0]&0x08)) {
    if (srcc<4) return -1;
    if ((SRC[1]&0xc0)!=0x80) return -1;
    if ((SRC[2]&0xc0)!=0x80) return -1;
    if ((SRC[3]&0xc0)!=0x80) return -1;
    *dst=((SRC[0]&0x07)<<18)|((SRC[1]&0x3f)<<12)|((SRC[2]&0x3f)<<6)|(SRC[3]&0x3f);
    return 4;
  }
  return 1;
}

int sr_utf8_encode(void *dst,int dsta,int src) {
  if (src<0) return -1;
  uint8_t *DST=dst;
  if (src<0x80) {
    if (dsta>=1) {
      DST[0]=src;
    }
    return 1;
  }
  if (src<0x800) {
    if (dsta>=2) {
      DST[0]=0xc0|(src>>6);
      DST[1]=0x80|(src&0x3f);
    }
    return 2;
  }
  if (src<0x10000) {
    if (dsta>=3) {
      DST[0]=0xe0|(src>>12);
      DST[1]=0x80|((src>>6)&0x3f);
      DST[2]=0x80|(src&0x3f);
    }
    return 3;
  }
  if (src<0x110000) { // Fiat limit; technically it can to up to 0x200000
    if (dsta>=4) {
      DST[0]=0xf0|(src>>18);
      DST[1]=0x80|((src>>12)&0x3f);
      DST[2]=0x80|((src>>6)&0x3f);
      DST[3]=0x80|(src&0x3f);
    }
    return 4;
  }
  return -1;
}
