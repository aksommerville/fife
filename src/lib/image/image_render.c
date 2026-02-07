#include "image_internal.h"

/* Fill rectangle.
 */
 
void image_fill_rect(struct image *image,int x,int y,int w,int h,uint32_t pixel) {
  if (!image||!image->writeable) return;
  x+=image->x0;
  y+=image->y0;
  if (x<0) { w+=x; x=0; }
  if (y<0) { h+=y; y=0; }
  if (x>image->w-w) w=image->w-x;
  if (y>image->h-h) h=image->h-y;
  if ((w<1)||(h<1)) return;
  if (image->pixelsize==32) {
    if (image->stride&3) return;
    int stridewords=image->stride>>2;
    uint32_t *dstrow=image->v;
    dstrow+=y*stridewords+x;
    for (;h-->0;dstrow+=stridewords) {
      uint32_t *dstp=dstrow;
      int xi=w;
      for (;xi-->0;dstp++) *dstp=pixel;
    }
  }
}
