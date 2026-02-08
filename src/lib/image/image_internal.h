#ifndef IMAGE_INTERNAL_H
#define IMAGE_INTERNAL_H

#include "image.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#if USE_fs
  #include "opt/fs/fs.h"
#endif
#if USE_png
  #include "opt/png/png.h"
  struct image *image_from_destroyable_png_image(struct png_image *png);
#endif

#endif
