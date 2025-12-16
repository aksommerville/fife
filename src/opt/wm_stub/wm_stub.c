/* wm_stub.c
 * Noop stub implementation of our "wm" interface, so there's a generic fallback.
 * Anything built against this will of course not be usable.
 */
 
#include "lib/wm/wm.h"

void wm_quit() {
}

int wm_init(const struct wm_delegate *delegate) {
  return 0;
}

void wm_set_title(const char *src,int srcc) {
}

void wm_set_icon(const void *rgba,int w,int h) {
}

int wm_define_cursor(const void *rgba,int w,int h) {
  return -1;
}

void wm_set_cursor(int cursorid) {
}

int wm_update() {
  return 0;
}

void wm_get_size(int *w,int *h) {
  *w=*h=1;
}

void wm_set_pixels(
  int x,int y,int w,int h,
  const void *rgbx,int stride
) {
}
