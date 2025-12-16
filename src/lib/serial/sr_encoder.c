#include "serial.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>

#define DST ((uint8_t*)(encoder->v))

/* Cleanup.
 */
 
void sr_encoder_cleanup(struct sr_encoder *encoder) {
  if (encoder->v) free(encoder->v);
}

/* Grow buffer.
 */
 
int sr_encoder_require(struct sr_encoder *encoder,int addc) {
  if (encoder->ctx<0) return encoder->ctx;
  if (addc<1) return 0;
  if (encoder->c>INT_MAX-addc) return encoder->ctx=-1;
  int na=encoder->c+addc;
  if (na<=encoder->a) return 0;
  if (na<INT_MAX-1024) na=(na+1024)&~1023;
  void *nv=realloc(encoder->v,na);
  if (!nv) return encoder->ctx=-1;
  encoder->v=nv;
  encoder->a=na;
  return 0;
}

/* Assert no errors.
 */

int sr_encoder_assert(struct sr_encoder *encoder) {
  if (encoder->ctx<0) return encoder->ctx; // Sticky error.
  if (encoder->ctx) return encoder->ctx=-1; // Unfinished thing.
  return 0;
}

/* Add non-counted terminator.
 */
 
int sr_encoder_terminate(struct sr_encoder *encoder) {
  if (sr_encoder_require(encoder,1)<0) return encoder->ctx;
  DST[encoder->c]=0;
  return 0;
}

/* Remove and insert.
 */
 
int sr_encoder_replace(struct sr_encoder *encoder,int p,int c,const void *src,int srcc) {
  if (encoder->ctx<0) return encoder->ctx;
  if ((p<0)||(p>encoder->c)) return encoder->ctx=-1;
  if ((c<0)||(p>encoder->c-c)) return encoder->ctx=-1;
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (((char*)src)[srcc]) srcc++; }
  if (c==srcc) {
    memcpy(DST+p,src,srcc);
  } else {
    if (sr_encoder_require(encoder,srcc-c)<0) return encoder->ctx;
    memmove(DST+p+srcc,DST+p+c,encoder->c-c-p);
    memcpy(DST+p,src,srcc);
    encoder->c+=srcc-c;
  }
  return 0;
}

/* Append raw data.
 */

int sr_encode_raw(struct sr_encoder *encoder,const void *src,int srcc) {
  if (encoder->ctx<0) return encoder->ctx;
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (((char*)src)[srcc]) srcc++; }
  if (sr_encoder_require(encoder,srcc)<0) return encoder->ctx;
  memcpy(DST+encoder->c,src,srcc);
  encoder->c+=srcc;
  return 0;
}

/* Append formatted text.
 */
 
int sr_encode_fmt(struct sr_encoder *encoder,const char *fmt,...) {
  if (encoder->ctx<0) return encoder->ctx;
  if (!fmt||!fmt[0]) return 0;
  for (;;) {
    va_list vargs;
    va_start(vargs,fmt);
    int err=vsnprintf(DST+encoder->c,encoder->a-encoder->c,fmt,vargs);
    if ((err<0)||(err>=INT_MAX)) return encoder->ctx=-1;
    if (encoder->c<=encoder->a-err) { // sic '<=', need room for vsnprintf's terminator.
      encoder->c+=err;
      return 0;
    }
    if (sr_encoder_require(encoder,err+1)<0) return encoder->ctx; // +1 for the terminator.
  }
}

/* Append binary integers.
 */
 
int sr_encode_u8(struct sr_encoder *encoder,uint8_t src) {
  if (sr_encoder_require(encoder,1)<0) return encoder->ctx;
  DST[encoder->c++]=src;
  return 0;
}

int sr_encode_intbe(struct sr_encoder *encoder,int v,int size_bytes) {
  if (encoder->ctx<0) return encoder->ctx;
  if ((size_bytes<1)||(size_bytes>4)) return -1;
  if (sr_encoder_require(encoder,size_bytes)<0) return encoder->ctx;
  int i=size_bytes;
  for (;i-->0;v>>=8) DST[encoder->c+i]=v;
  encoder->c+=size_bytes;
  return 0;
}

int sr_encode_intle(struct sr_encoder *encoder,int v,int size_bytes) {
  if (encoder->ctx<0) return encoder->ctx;
  if ((size_bytes<1)||(size_bytes>4)) return -1;
  if (sr_encoder_require(encoder,size_bytes)<0) return encoder->ctx;
  int i=0;
  for (;i<size_bytes;i++,v>>=8) DST[encoder->c+i]=v;
  encoder->c+=size_bytes;
  return 0;
}
