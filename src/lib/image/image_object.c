#include "image_internal.h"

/* Delete.
 */
 
static void image_cleanup(struct image *image) {
  if (image->v&&image->ownv) free(image->v);
}
 
void image_del(struct image *image) {
  if (!image->refc) {
    image_cleanup(image);
    memset(image,0,sizeof(struct image));
    return;
  }
  if (image->refc-->1) return;
  image_cleanup(image);
  free(image);
}

/* Retain.
 */
 
int image_ref(struct image *image) {
  if (!image) return -1;
  if (image->refc<1) return -1;
  if (image->refc>=INT_MAX) return -1;
  image->refc++;
  return 0;
}

/* New from path.
 */
 
struct image *image_new_from_path(const char *path) {
  struct image *image=0;
  #if USE_fs
    void *serial=0;
    int serialc=file_read(&serial,path);
    if (serialc>=0) {
      image=image_new_decode(serial,serialc);
      free(serial);
    }
  #endif
  return image;
}

/* New from serial.
 */
 
struct image *image_new_decode(const void *src,int srcc) {
  if (!src||(srcc<1)) return 0;
  #if USE_png
    struct png_image *png=png_decode(src,srcc);
    if (png) {
      struct image *image=image_from_destroyable_png_image(png);
      png_image_del(png);
      return image;
    }
  #endif
  //TODO Certainly conceivable to support other formats.
  return 0;
}
