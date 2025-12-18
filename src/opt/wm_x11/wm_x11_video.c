#include "wm_x11_internal.h"

/* Call after replacing (wm_x11.fb), to cheaply assess its pixel format.
 */
 
static int wm_x11_shift_from_mask(uint32_t mask) {
  if (!mask) return 0; // Error, channel not defined.
  int shift=0;
  while (!(mask&1)) { mask>>=1; shift++; }
  if (mask!=0xff) ; // Error, channel not 8 bits.
  return shift;
}
 
static void wm_x11_reassess_pixel_format() {
  uint32_t r=wm_x11.fb->red_mask;
  uint32_t g=wm_x11.fb->green_mask;
  uint32_t b=wm_x11.fb->blue_mask;
       if ((r==0xff000000)&&(g==0x00ff0000)&&(b==0x0000ff00)) wm_x11.pixfmt=WM_X11_PIXFMT_RGBX;
  else if ((r==0x00ff0000)&&(g==0x0000ff00)&&(b==0x000000ff)) wm_x11.pixfmt=WM_X11_PIXFMT_XRGB;
  else if ((r==0x0000ff00)&&(g==0x00ff0000)&&(b==0xff000000)) wm_x11.pixfmt=WM_X11_PIXFMT_BGRX;
  else if ((r==0x000000ff)&&(g==0x0000ff00)&&(b==0x00ff0000)) wm_x11.pixfmt=WM_X11_PIXFMT_XBGR;
  else {
    wm_x11.pixfmt=WM_X11_PIXFMT_OTHER;
    wm_x11.rshift=wm_x11_shift_from_mask(r);
    wm_x11.gshift=wm_x11_shift_from_mask(g);
    wm_x11.bshift=wm_x11_shift_from_mask(b);
  }
}

/* Recreate framebuffer if necessary.
 * If we succeed, (wm_x11.fb) is valid and its size matches (wm_x11.(w,h)).
 */
 
static int wm_x11_require_fb() {
  if (!wm_x11.init) return -1;
  if (!wm_x11.fb||(wm_x11.fb->width!=wm_x11.w)||(wm_x11.fb->height!=wm_x11.h)) {
    // In resize cases, we blank the whole framebuffer and trust that a full exposure will be sent.
    // Should we work harder to guarantee that, or copy the existing pixels?
    void *pixels=calloc(wm_x11.w<<2,wm_x11.h);
    if (!pixels) return -1;
    XImage *image=XCreateImage(
      wm_x11.dpy,DefaultVisual(wm_x11.dpy,wm_x11.screen),
      24,ZPixmap,0,pixels,wm_x11.w,wm_x11.h,32,wm_x11.w<<2
    );
    if (!image) {
      free(pixels);
      return -1;
    }
    if (wm_x11.fb) XDestroyImage(wm_x11.fb);
    wm_x11.fb=image;
    wm_x11_reassess_pixel_format();
  }
  return 0;
}

/* Get framebuffer.
 */
 
void *wm_get_framebuffer(int *w,int *h,int *stride) {
  if (wm_x11_require_fb()<0) return 0;
  *w=wm_x11.fb->width;
  *h=wm_x11.fb->height;
  *stride=(*w)<<2;
  return wm_x11.fb->data;
}

/* Send framebuffer.
 */

void wm_framebuffer_dirty(int x,int y,int w,int h) {
  if (!wm_x11.init) return;
  if (!wm_x11.fb) return;
  if ((x<0)||(y<0)||(x>wm_x11.fb->width-w)||(y>wm_x11.fb->height-h)) return;
  XPutImage(
    wm_x11.dpy,wm_x11.win,wm_x11.gc,wm_x11.fb,
    x,y,x,y,w,h
  );
}

/* Pixel format.
 */
 
uint32_t wm_pixel_from_rgbx(uint32_t rgbx) {
  switch (wm_x11.pixfmt) {
    case WM_X11_PIXFMT_OTHER: {
        uint8_t r=rgbx>>24,g=rgbx>>16,b=rgbx>>8;
        return (r<<wm_x11.rshift)|(g<<wm_x11.gshift)|(b<<wm_x11.bshift);
      }
    case WM_X11_PIXFMT_RGBX: return rgbx;
    case WM_X11_PIXFMT_XRGB: return rgbx>>8;
    case WM_X11_PIXFMT_BGRX: return ((rgbx&0xff00)<<16)|(rgbx&0xff0000)|((rgbx&0xff000000)>>16);
    case WM_X11_PIXFMT_XBGR: return ((rgbx&0xff00)<<8)|((rgbx&0xff0000)>>8)|(rgbx>>24);
  }
  return rgbx;
}

uint32_t wm_rgbx_from_pixel(uint32_t pixel) {
  switch (wm_x11.pixfmt) {
    case WM_X11_PIXFMT_OTHER: {
        uint8_t r=pixel>>wm_x11.rshift,g=pixel>>wm_x11.gshift,b=pixel>>wm_x11.bshift;
        return (r<<24)|(g<<16)|(b<<8);
      }
    case WM_X11_PIXFMT_RGBX: return pixel;
    case WM_X11_PIXFMT_XRGB: return pixel<<8;
    case WM_X11_PIXFMT_BGRX: return ((pixel&0xff00)<<16)|(pixel&0xff0000)|((pixel&0xff000000)>>16);
    case WM_X11_PIXFMT_XBGR: return ((pixel&0xff0000)>>8)|((pixel&0xff00)<<8)|(pixel<<24);
  }
  return pixel;
}
