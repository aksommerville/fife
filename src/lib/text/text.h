/* text.h
 * Structured text encoders and decoders.
 * In addition to wanting to support every realistic text encoding, we have two unusual requirements:
 *  - Tolerate misencoded text.
 *  - Read backward.
 * Reading backward is especially tricky, because it must treat misencoded text the same way it would forward.
 */
 
#ifndef TEXT_H
#define TEXT_H

struct text_encoding {
  const char *name; // Lowercase C identifier, matches the object's name.
  const void *ctx; // eg table of codepoints, so table-based encodings can share their hooks.
  
  /* Read one codepoint starting at (src) and return the consumed length.
   * If misencoded, consume one byte and subtract 0x100 from it.
   * Must never return <=0.
   */
  int (*read)(int *codepoint,const void *src,int srcc,const void *ctx);
  
  /* Read one codepoint ending at (src+srcc) and return the positive consumed length.
   * Same constraints as (read).
   */
  int (*unread)(int *codepoint,const void *src,int srcc,const void *ctx);
  
  /* Encode (codepoint) at (dst) and return produced length.
   * Return <=0 if (codepoint) is not encodable.
   * If (dsta) is insufficient, return the required length and don't write anything.
   */
  int (*write)(void *dst,int dsta,int codepoint,const void *ctx);
};

/* Registry of encodings.
 ************************************************************************/
 
#define TEXT_ENCODING_utf8     1
#define TEXT_ENCODING_iso88591 2 /* aka ucs1, and passes for ascii. */
#define TEXT_ENCODING_utf16le  3
#define TEXT_ENCODING_utf16be  4
#define TEXT_ENCODING_ucs2le   5
#define TEXT_ENCODING_ucs2be   6
#define TEXT_ENCODING_ucs3le   7 /* Unicode doesn't define this, but it feels right. */
#define TEXT_ENCODING_ucs3be   8 /* '' */
#define TEXT_ENCODING_ucs4le   9
#define TEXT_ENCODING_ucs4be  10
//TODO Legacy Windows and MacOS page-based encodings.
//TODO Other 8859 page-based encodings, eg 15 is popular.

#define FOR_EACH_TEXT_ENCODING \
  _(utf8) \
  _(iso88591) \
  _(utf16le) \
  _(utf16be) \
  _(ucs2le) \
  _(ucs2be) \
  _(ucs3le) \
  _(ucs3be) \
  _(ucs4le) \
  _(ucs4be)
  
#define _(tag) extern const struct text_encoding text_encoding_##tag;
FOR_EACH_TEXT_ENCODING
#undef _
  
const struct text_encoding *text_encoding_by_id(int id); // TEXT_ENCODING_*. Mind that you can also refer to them directly eg `&text_encoding_utf8`.
const struct text_encoding *text_encoding_by_name(const char *name,int namec);
const struct text_encoding *text_encoding_by_index(int p);

/* A few encoding templates for page-based ones.
 * These all take an array of uint32_t Unicode code points as (ctx).
 * For "asciiplus", the low 128 values are ASCII, and the 128-entry page is for values 0x80..0xff.
 */
int text_page128_read(int *codepoint,const void *src,int srcc,const void *ctx);
int text_page128_unread(int *codepoint,const void *src,int srcc,const void *ctx);
int text_page128_write(void *dst,int dsta,int codepoint,const void *ctx);
int text_page256_read(int *codepoint,const void *src,int srcc,const void *ctx);
int text_page256_unread(int *codepoint,const void *src,int srcc,const void *ctx);
int text_page256_write(void *dst,int dsta,int codepoint,const void *ctx);
int text_asciiplus_read(int *codepoint,const void *src,int srcc,const void *ctx);
int text_asciiplus_unread(int *codepoint,const void *src,int srcc,const void *ctx);
int text_asciiplus_write(void *dst,int dsta,int codepoint,const void *ctx);

/* Structured decoder.
 * Decoders are dirt cheap and don't require any cleanup.
 * Safe to copy directly.
 **********************************************************************/
 
struct text_decoder {
  const struct text_encoding *encoding;
  const char *v;
  int c;
  int p; // Must be in (0..c).
};

/* It's normal to initialize a text_decoder directly.
 * But we also offer these conveniences.
 * These fail if the encoding isn't found.
 * Readhead initializes to zero.
 */
int text_decoder_init_object(struct text_decoder *decoder,const char *src,int srcc,const struct text_encoding *encoding);
int text_decoder_init_id(struct text_decoder *decoder,const char *src,int srcc,int encoding_id);
int text_decoder_init_name(struct text_decoder *decoder,const char *src,int srcc,const char *name,int namec);

/* Both return nonzero if something was consumed, or zero at the end.
 * If misencoded, (*codepoint) will be (RAW_BYTE-0x100), ie always negative.
 */
int text_decoder_read(int *codepoint,struct text_decoder *decoder);
int text_decoder_unread(int *codepoint,struct text_decoder *decoder);

/* Structured encoder.
 * These do require cleanup and are not safe to copy.
 * You can yoink the text from an encoder, and then either set it null or just don't clean up.
 **********************************************************************/
 
struct text_encoder {
  const struct text_encoding *encoding;
  char *v;
  int c,a;
};

void text_encoder_cleanup(struct text_encoder *encoder);
int text_encoder_require(struct text_encoder *encoder,int addc);

// Like decoder, a few optional conveniences for initialization.
int text_encoder_init_object(struct text_encoder *encoder,const struct text_encoding *encoding);
int text_encoder_init_id(struct text_encoder *encoder,int encoding_id);
int text_encoder_init_name(struct text_encoder *encoder,const char *name,int namec);

/* Remove the range at (p,c) and replace with (src,srcc).
 * We do not assert encoding, you're allowed to break it with this call.
 */
int text_encoder_replace_raw(struct text_encoder *encoder,int p,int c,const void *src,int srcc);

// Modify text, one character at a time.
int text_encoder_append(struct text_encoder *encoder,int codepoint); // => length appended
int text_encoder_insert(struct text_encoder *encoder,int p,int codepoint); // => (p) after insertion
int text_encoder_delete_forward(struct text_encoder *encoder,int p); // => (p)
int text_encoder_delete_backward(struct text_encoder *encoder,int p); // => (p) after the removal

#endif
