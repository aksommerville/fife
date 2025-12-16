/* serial.h
 * Helpers for working with serial data.
 */
 
#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

/* Primitive text tokens.
 *******************************************************************************/

static inline int sr_digit_eval(char src) {
  if ((src>='0')&&(src<='9')) return src-'0';
  if ((src>='a')&&(src<='z')) return src-'a'+10;
  if ((src>='A')&&(src<='Z')) return src-'A'+10;
  return -1;
}

/* Consume cut tokens like: [SIGN] [RADIX] [DIGITS]
 * SIGN is '+' or '-'.
 * RADIX is "0b", "0o", "0d", or "0x"; or any of those upper-case.
 * Returns:
 *   <0: Malformed token.
 *    0: Well formed but significant overflow.
 *    1: Overflow into sign bit. (input looked positive but output is negative)
 *    2: Unqualified success.
 */
int sr_int_eval(int *dst,const char *src,int srcc);

/* Represent decimal integer.
 */
int sr_decsint_repr(char *dst,int dsta,int src);
int sr_decuint_repr(char *dst,int dsta,int src);

//TODO floats?
//TODO strings?

/* Well-known encodings.
 *****************************************************************************/
 
//TODO base64?
//TODO urlencode?
//TODO sha?

int sr_utf8_decode(int *dst,const void *src,int srcc);
int sr_utf8_encode(void *dst,int dsta,int src);

/* Structured decoder.
 * Owner must hold the buffer constant.
 * No cleanup necessary.
 *****************************************************************************/
 
struct sr_decoder {
  const void *v;
  int c,p;
  int ctx;
};

/* Trigger (cb) for each line, separated by LF.
 * The LF is included in each reported line (STRIP_TAIL will remove it).
 * (lineno) starts at 1.
 * Stops when you return nonzero and returns the same.
 */
int sr_decode_lines(
  struct sr_decoder *decoder,
  int (*cb)(const char *src,int srcc,int lineno,void *userdata),
  void *userdata,
  int flags
);
#define SR_DECODE_LINES_STRIP_HEAD       0x0001
#define SR_DECODE_LINES_STRIP_TAIL       0x0002
#define SR_DECODE_LINES_STRIP (SR_DECODE_LINES_STRIP_HEAD|SR_DECODE_LINES_STRIP_TAIL)
#define SR_DECODE_LINES_STRIP_COMMENT    0x0004 /* Stop at '#'. Beware that we're not savvy to your tokenization. Don't use if you have string tokens. */
#define SR_DECODE_LINES_INCLUDE_EMPTY    0x0008 /* By default we skip ones that are empty after stripping. Empties are not possible without stripping. */

/* Structured encoder.
 * Initialize to all zeroes.
 * It's safe to yoink (v) and skip cleanup.
 *****************************************************************************/
 
struct sr_encoder {
  void *v;
  int c,a;
  int ctx; // <0 for sticky errors. >0 reserved for future context-sensitive operations.
};

void sr_encoder_cleanup(struct sr_encoder *encoder);
int sr_encoder_require(struct sr_encoder *encoder,int addc);
int sr_encoder_assert(struct sr_encoder *encoder); // Basically just returns (ctx). Do we have a sticky error?
int sr_encoder_terminate(struct sr_encoder *encoder); // Adds a zero without counting it.
int sr_encoder_replace(struct sr_encoder *encoder,int p,int c,const void *src,int srcc); // Remove and insert.

int sr_encode_raw(struct sr_encoder *encoder,const void *src,int srcc); // (srcc<0) ok if nul-terminated.
int sr_encode_fmt(struct sr_encoder *encoder,const char *fmt,...);
int sr_encode_u8(struct sr_encoder *encoder,uint8_t src);
int sr_encode_intbe(struct sr_encoder *encoder,int v,int size_bytes); // 1,2,3,4
int sr_encode_intle(struct sr_encoder *encoder,int v,int size_bytes); // 1,2,3,4

#endif
