/* wm.h
 * This is an interface implemented by one opt unit, wrapping the underlying window manager.
 * Window managers are expected to provide:
 *  - 32-bit framebuffer, but not necessarily RGBA.
 *  - Keyboard events, including digested text.
 *  - Mouse events.
 *  - Clipboard, preferably shared with the OS.
 *  - Title, icon, and cursor, optional.
 */
 
#ifndef WM_H
#define WM_H

#include <stdint.h>

struct wm_delegate {
  void (*cb_close)();
  void (*cb_resize)(int w,int h);
  void (*cb_focus)(int focus);
  void (*cb_expose)(int x,int y,int w,int h);
  int (*cb_key)(int keycode,int value); // Nonzero to consume. If zero, we may fire cb_text after. (keycode) is USB-HID page 7.
  void (*cb_text)(int codepoint); // Unicode.
  void (*cb_mmotion)(int x,int y); // (x,y) in window client coords. May be OOB, or WM may never report OOBs.
  void (*cb_mbutton)(int btnid,int value); // (1,2,3)=(left,right,center)
  void (*cb_mwheel)(int dx,int dy); // One unit should be one click of the wheel.
};

void wm_quit();
int wm_init(const struct wm_delegate *delegate);
int wm_update();

void wm_set_title(const char *src,int srcc);
void wm_set_icon(const void *rgba,int w,int h); // Minimum stride.
int wm_define_cursor(const void *rgba,int w,int h); // Minimum stride. Returns >0 cursorid.
void wm_set_cursor(int cursorid);

/* Size of window's client area in pixels.
 * Will not change without a call to your delegate.
 */
void wm_get_size(int *w,int *h);

/* Get a pointer to the window manager's framebuffer that you can write to.
 * Also returns its geometry. Should be the same as wm_get_size(), but if different, the ones returned here are the ones to use.
 * Do not retain framebuffer pointers across calls to wm_update().
 */
void *wm_get_framebuffer(int *w,int *h,int *stride);

/* Notify that a region of the framebuffer has changed.
 */
void wm_framebuffer_dirty(int x,int y,int w,int h);

/* Don't make any assumptions about the framebuffer's pixel format,
 * except that it is 32 bits per pixel and aligned on 32-bit boundaries.
 * Call these to convert between opaque WM pixels and canonical RGBA (0xRRGGBBXX).
 */
uint32_t wm_pixel_from_rgbx(uint32_t rgbx);
uint32_t wm_rgbx_from_pixel(uint32_t pixel);

//TODO copy/paste

#endif
