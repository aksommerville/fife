#ifndef WM_X11_INTERNAL_H
#define WM_X11_INTERNAL_H

#include "wm_x11.h"
#include "lib/wm/wm.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

#define WM_X11_TITLE_LIMIT  256 /* bytes */
#define WM_X11_ICON_LIMIT   256 /* pixels per axis */
#define WM_X11_CURSOR_LIMIT  64 /* pixels per axis */

#define KeyRepeat (LASTEvent+2)
#define WM_X11_KEY_REPEAT_INTERVAL 10

// Framebuffer pixel format is always 32 bits, with each of (R,G,B) occupying 8 bits.
// We optimize for likely cases, but do tolerate anything meeting those ^ criteria.
// Formats are named big-endianly, not by storage order!
#define WM_X11_PIXFMT_OTHER 0 /* Use (rshift,gshift,bshift). */
#define WM_X11_PIXFMT_RGBX  1 /* Pixels are the native format, lovely! */
#define WM_X11_PIXFMT_XRGB  2 /* Right order, just shift the whole thing to convert. */
#define WM_X11_PIXFMT_BGRX  3
#define WM_X11_PIXFMT_XBGR  4

extern struct wm_x11 {
  int init;
  struct wm_delegate delegate;
  
  Display *dpy;
  int screen;
  Window win;
  GC gc;
  XImage *fb; // Doesn't exist until someone asks for it.
  int pixfmt; // WM_X11_PIXFMT_*
  int rshift,gshift,bshift; // Relevant only for WM_X11_PIXFMT_OTHER.
  
  Atom atom_WM_PROTOCOLS;
  Atom atom_WM_DELETE_WINDOW;
  Atom atom__NET_WM_STATE;
  Atom atom__NET_WM_STATE_FULLSCREEN;
  Atom atom__NET_WM_STATE_ADD;
  Atom atom__NET_WM_STATE_REMOVE;
  Atom atom__NET_WM_ICON;
  Atom atom__NET_WM_ICON_NAME;
  Atom atom__NET_WM_NAME;
  Atom atom_WM_CLASS;
  Atom atom_STRING;
  Atom atom_UTF8_STRING;
  
  int w,h;
  int focus;
  
  struct wm_x11_cursor {
    int cursorid;
    void *rgba; // Minimum stride.
    int w,h;
  } *cursorv;
  int cursorc,cursora;
  
} wm_x11;

int wm_x11_usb_usage_from_keysym(int keysym);
int wm_x11_codepoint_from_keysym(int keysym);

#endif
