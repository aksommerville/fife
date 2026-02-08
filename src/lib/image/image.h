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

/* Only works if the "fs" unit is enabled, and the appropriate decoder unit, eg "png".
 */
struct image *image_new_from_path(const char *path);

/* eg png.
 */
struct image *image_new_decode(const void *src,int srcc);

void image_fill_rect(struct image *image,int x,int y,int w,int h,uint32_t pixel);
void image_fill_rect_halftone(struct image *image,int x,int y,int w,int h,uint32_t pixel);
void image_frame_rect(struct image *image,int x,int y,int w,int h,uint32_t pixel);
void image_frame_rect_dotted(struct image *image,int x,int y,int w,int h,uint32_t pixel);

#endif
