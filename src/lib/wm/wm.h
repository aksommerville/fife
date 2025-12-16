/* wm.h
 * This is an interface implemented by one opt unit, wrapping the underlying window manager.
 */
 
#ifndef WM_H
#define WM_H

struct wm_delegate {
  void (*cb_close)();
  void (*cb_resize)(int w,int h);
  void (*cb_focus)(int focus);
  void (*cb_expose)(int x,int y,int w,int h);
  int (*cb_key)(int keycode,int value); // Nonzero to consume. If zero, we may fire cb_text after. (keycode) is USB-HID page 7.
  void (*cb_text)(int codepoint);
  void (*cb_mmotion)(int x,int y); // (x,y) in window client coords. May be OOB, or WM may never report OOBs.
  void (*cb_mbutton)(int btnid,int value);
  void (*cb_mwheel)(int dx,int dy);
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

/* Replace some portion of the framebuffer.
 * OOB is illegal.
 */
void wm_set_pixels(
  int x,int y,int w,int h,
  const void *rgbx,int stride
);

//TODO copy/paste

#endif
