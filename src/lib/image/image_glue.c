#include "image_internal.h"

/* Image from PNG image, allowing to yoink pixels if possible.
 * (it's always possible).
 */
#if USE_png

struct image *image_from_destroyable_png_image(struct png_image *png) {
  if (!png||!png->v) return 0;
  struct image *image=calloc(1,sizeof(struct image));
  if (!image) return 0;
  image->refc=1;
  image->v=png->v; png->v=0; // HANDOFF
  image->w=png->w; png->w=0;
  image->h=png->h; png->h=0;
  image->stride=png->stride; png->stride=0;
  image->pixelsize=png->pixelsize;
  image->ownv=1;
  image->writeable=1;
  return image;
}

#endif
