/* image.h
 */
 
#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

struct image {
  int refc; // 0 if immortal or allocated on stack; ref will fail.
  void *v;
  int w,h,stride;
  int pixelsize; // bits
  int ownv;
  int writeable;
  int x0,y0; // Coordinate adjustment. All render requests, we first add these to the requested coordinates.
};

void image_del(struct image *image);
int image_ref(struct image *image);

void image_fill_rect(struct image *image,int x,int y,int w,int h,uint32_t pixel);

#endif
