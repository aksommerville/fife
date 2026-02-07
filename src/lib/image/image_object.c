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
