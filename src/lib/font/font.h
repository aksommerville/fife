/* font.h
 * Quirky and opinionated pure-software text rendering.
 *  - All text is monospaced.
 *  - Only G0 displays normally.
 *  - We can produce tofu for non-G0 codepoints.
 *  - Glyphs come from a 16x7-cell image that you provide.
 *  - Only 32-bit images can be rendered to. We could add more if needed; see font_render.c.
 * Source images:
 *  - Top 6 rows are 0x20..0x7f.
 *  - Bottom row starts with tofu frames. Must align to column boundaries. A rectangular outline, filled with boxes of uniform size.
 *  - Right of the frames, 16 glyphs for the tofu hex digits.
 */
 
#ifndef FONT_H
#define FONT_H

#include <stdint.h>

struct font;
struct image;

void font_del(struct font *font);

/* We will digest and copy (image), you can free it once the font is instantiated.
 * We can only read from a path if the "fs" and "png" units are present.
 */
struct font *font_new(struct image *image);
struct font *font_new_from_path(const char *path);

/* Retrieve size of one glyph.
 */
int font_get_width(const struct font *font);
int font_get_height(const struct font *font);

/* Render one glyph with top-left corner at (dstx,dsty).
 * Both return the horizontal advancement on success.
 * font_render_glyph fails for anything outside G0; you should then call font_render_tofu.
 */
int font_render_glyph(struct image *dst,int dstx,int dsty,struct font *font,int codepoint,uint32_t color);
int font_render_tofu(struct image *dst,int dstx,int dsty,struct font *font,int codepoint,uint32_t color);

/* Set color for normal glyphs, and for tofus.
 * These are not used by font_render_glyph or font_render_tofu, but are by all the higher-level renders.
 */
void font_set_color_normal(struct font *font,uint32_t color);
void font_set_color_missing(struct font *font,uint32_t color);
void font_set_color_misencode(struct font *font,uint32_t color);

/* By default we expect UTF-8.
 * Any other encoding, you must provide a no-context decoder.
 * Decoder returns the length of the sequence, and puts its Unicode codepoint in (*codepoint).
 * If the decoder returns <=0, we consume one byte and print it as "misencoded".
 */
void font_set_decoder(struct font *font,int (*decode)(int *codepoint,const char *src,int srcc));

/* How wide will this be if you give it to font_render_string?
 * Usually (srcc*glyph_width), but we do account for tofu.
 */
int font_measure_string(struct font *font,const char *src,int srcc);

/* How wide would a tofu be for this codepoint?
 * Always a multiple of the glyph width.
 */
int font_measure_tofu(struct font *font,int codepoint);

/* Render one line of text starting at (dstx,dsty).
 * Returns horizontal advancement.
 */
int font_render_string(struct image *dst,int dstx,int dsty,struct font *font,const char *src,int srcc);

/* Since we need it internally, might as well expose our UTF-8 decoder.
 */
int font_utf8_decode(int *codepoint,const char *src,int srcc);

#endif
