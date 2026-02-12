#ifndef FONT_INTERNAL_H
#define FONT_INTERNAL_H

#include "font.h"
#include "lib/image/image.h"
#include "lib/text/text.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

struct font {
  int refc;
  int w,h; // Of one glyph.
  uint8_t *img; // A1 big-endian, the way PNG does it.
  int imgw,imgh;
  int imgstride;
  uint32_t color_normal,color_missing,color_misencode;
  const struct text_encoding *encoding;
  //TODO Whatever bookkeeping we need around the tofu.
};

void font_copy_image(struct font *font,struct image *image);

#endif
