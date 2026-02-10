#include "text.h"
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>

/* I'm not aware of any construction in any encoding that could produce a sequence longer than 4 bytes.
 * But if we find one, bump this up.
 */
#define LONGEST_CONCEIVABLE_SEQUENCE 4

/* Get encodings from our registry.
 */
  
const struct text_encoding *text_encoding_by_id(int id) {
  switch (id) {
    #define _(tag) case TEXT_ENCODING_##tag: return &text_encoding_##tag;
    FOR_EACH_TEXT_ENCODING
    #undef _
  }
  return 0;
}

const struct text_encoding *text_encoding_by_name(const char *name,int namec) {
  if (!name) return 0;
  if (namec<0) { namec=0; while (name[namec]) namec++; }
  char norm[32];
  int normc=0;
  for (;namec-->0;name++) {
    if (normc>=sizeof(norm)) return 0;
    if ((*name>='A')&&(*name<='Z')) norm[normc++]=(*name)+0x20;
    else if ((*name>='a')&&(*name<='z')) norm[normc++]=*name;
    else if ((*name>='0')&&(*name<='9')) norm[normc++]=*name;
  }
  #define _(tag) if ((normc==sizeof(#tag)-1)&&!memcmp(norm,#tag,normc)) return &text_encoding_##tag;
  FOR_EACH_TEXT_ENCODING
  #undef _
  return 0;
}
  
const struct text_encoding *text_encoding_by_index(int p) {
  if (p<0) return 0;
  #define _(tag) if (!p--) return &text_encoding_##tag;
  FOR_EACH_TEXT_ENCODING
  #undef _
  return 0;
}

/* Decoder.
 */

int text_decoder_init_object(struct text_decoder *decoder,const char *src,int srcc,const struct text_encoding *encoding) {
  if (!decoder||!encoding) return -1;
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  decoder->encoding=encoding;
  decoder->v=src;
  decoder->c=srcc;
  decoder->p=0;
  return 0;
}

int text_decoder_init_id(struct text_decoder *decoder,const char *src,int srcc,int encoding_id) {
  return text_decoder_init_object(decoder,src,srcc,text_encoding_by_id(encoding_id));
}

int text_decoder_init_name(struct text_decoder *decoder,const char *src,int srcc,const char *name,int namec) {
  return text_decoder_init_object(decoder,src,srcc,text_encoding_by_name(name,namec));
}

int text_decoder_read(int *codepoint,struct text_decoder *decoder) {
  if (decoder->p>=decoder->c) return 0;
  int err=decoder->encoding->read(codepoint,decoder->v+decoder->p,decoder->c-decoder->p,decoder->encoding->ctx);
  if (err<1) {
    *codepoint=((unsigned char)decoder->v[decoder->p])-0x100;
    decoder->p+=1;
    return 1;
  }
  decoder->p+=err;
  return err;
}

int text_decoder_unread(int *codepoint,struct text_decoder *decoder) {
  if (decoder->p<=0) return 0;
  int err=decoder->encoding->unread(codepoint,decoder->v,decoder->p,decoder->encoding->ctx);
  if (err<1) {
    decoder->p-=1;
    *codepoint=((unsigned char)decoder->v[decoder->p])-0x100;
    return 1;
  }
  decoder->p-=err;
  return err;
}

/* Encoder.
 */

void text_encoder_cleanup(struct text_encoder *encoder) {
  if (encoder->v) free(encoder->v);
}

int text_encoder_require(struct text_encoder *encoder,int addc) {
  if (addc<=0) return 0;
  if (encoder->c>INT_MAX-addc) return -1;
  int na=encoder->c+addc;
  if (na<=encoder->a) return 0;
  if (na<INT_MAX-1024) na=(na+1024)&~1023;
  void *nv=realloc(encoder->v,na);
  if (!nv) return -1;
  encoder->v=nv;
  encoder->a=na;
  return 0;
}

int text_encoder_init_object(struct text_encoder *encoder,const struct text_encoding *encoding) {
  if (!encoder||!encoding) return -1;
  encoder->encoding=encoding;
  encoder->v=0;
  encoder->c=0;
  encoder->a=0;
  return 0;
}

int text_encoder_init_id(struct text_encoder *encoder,int encoding_id) {
  return text_encoder_init_object(encoder,text_encoding_by_id(encoding_id));
}

int text_encoder_init_name(struct text_encoder *encoder,const char *name,int namec) {
  return text_encoder_init_object(encoder,text_encoding_by_name(name,namec));
}

int text_encoder_replace_raw(struct text_encoder *encoder,int p,int c,const void *src,int srcc) {
  if ((p<0)||(c<0)||(p>encoder->c-c)) return -1;
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (((char*)src)[srcc]) srcc++; }
  if (c==srcc) {
    memcpy(encoder->v+p,src,srcc);
  } else {
    if (text_encoder_require(encoder,srcc-c)<0) return -1;
    memmove(encoder->v+p+srcc,encoder->v+p+c,encoder->c-(p+c));
    memcpy(encoder->v+p,src,srcc);
    encoder->c+=srcc-c;
  }
  return 0;
}

int text_encoder_append(struct text_encoder *encoder,int codepoint) {
  int err=encoder->encoding->write(encoder->v+encoder->c,encoder->a-encoder->c,codepoint,encoder->encoding->ctx);
  if (err<1) return -1;
  if (encoder->c>encoder->a-err) {
    if (text_encoder_require(encoder,err)<0) return -1;
    if (encoder->encoding->write(encoder->v+encoder->c,encoder->a-encoder->c,codepoint,encoder->encoding->ctx)!=err) return -1;
  }
  encoder->c+=err;
  return err;
}

int text_encoder_insert(struct text_encoder *encoder,int p,int codepoint) {
  if ((p<0)||(p>encoder->c)) return -1;
  char tmp[LONGEST_CONCEIVABLE_SEQUENCE];
  int tmpc=encoder->encoding->write(tmp,sizeof(tmp),codepoint,encoder->encoding->ctx);
  if (tmpc<1) return -1;
  if (tmpc>sizeof(tmp)) return -1;
  if (text_encoder_replace_raw(encoder,p,0,tmp,tmpc)<0) return -1;
  return p+tmpc;
}

int text_encoder_delete_forward(struct text_encoder *encoder,int p) {
  if ((p<0)||(p>=encoder->c)) return -1;
  int dummy;
  int len=encoder->encoding->read(&dummy,encoder->v+encoder->c,encoder->a-encoder->c,encoder->encoding->ctx);
  if (len<1) len=1;
  if (text_encoder_replace_raw(encoder,p,len,0,0)<0) return -1;
  return p;
}

int text_encoder_delete_backward(struct text_encoder *encoder,int p) {
  if ((p<=0)||(p>encoder->c)) return -1;
  int dummy;
  int len=encoder->encoding->unread(&dummy,encoder->v,p,encoder->encoding->ctx);
  if (len<1) len=1;
  p-=len;
  if (text_encoder_replace_raw(encoder,p,len,0,0)<0) return -1;
  return p;
}
