#include "font_internal.h"

/* Delete.
 */
 
void font_del(struct font *font) {
  if (!font) return;
  if (font->img) free(font->img);
  free(font);
}

/* New, from image.
 */

struct font *font_new(struct image *image) {
  if (!image) return 0;
  
  // Confirm it produces sane dimensions.
  if ((image->w<1)||(image->w%16)) return 0;
  if ((image->h<1)||(image->h%7)) return 0;
  
  struct font *font=calloc(1,sizeof(struct font));
  if (!font) return 0;
  
  font->imgw=image->w;
  font->imgh=image->h;
  font->imgstride=(font->imgw+7)>>3;
  if (!(font->img=calloc(font->imgstride,font->imgh))) {
    font_del(font);
    return 0;
  }
  font->w=image->w/16;
  font->h=image->h/7;
  
  font_copy_image(font,image);
  
  font->decode=font_utf8_decode;
  font->color_normal=0xffffffff;
  font->color_missing=0xff0000ff;
  font->color_misencode=0xff0000ff;
  
  return font;
}

/* New, from path.
 */
 
struct font *font_new_from_path(const char *path) {
  struct image *image=image_new_from_path(path);
  if (!image) return 0;
  struct font *font=font_new(image);
  image_del(image);
  return font;
}

/* Trivial accessors.
 */

int font_get_width(const struct font *font) {
  if (!font) return 0;
  return font->w;
}

int font_get_height(const struct font *font) {
  if (!font) return 0;
  return font->h;
}

void font_set_color_normal(struct font *font,uint32_t color) {
  if (!font) return;
  font->color_normal=color;
}

void font_set_color_missing(struct font *font,uint32_t color) {
  if (!font) return;
  font->color_missing=color;
}

void font_set_color_misencode(struct font *font,uint32_t color) {
  if (!font) return;
  font->color_misencode=color;
}

void font_set_decoder(struct font *font,int (*decode)(int *codepoint,const char *src,int srcc)) {
  if (!font||!decode) return;
  font->decode=decode;
}
