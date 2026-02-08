#ifndef GUI_INTERNAL_H
#define GUI_INTERNAL_H

#include "gui.h"
#include "lib/wm/wm.h"
#include "lib/image/image.h"
#include "lib/font/font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <unistd.h>

/* Context.
 ****************************************************************************/

struct gui_context {
  struct gui_delegate delegate;
  volatile int terminate; // Per WM or client request.
  int termstatus;
  int w,h; // From window manager.
  struct widget *root;
  int render_soon;
  int tree_changed; // Widgets set nonzero any time a widget is added, removed, or order changed.
  
  // First in the list is our default.
  struct font_entry {
    char *name;
    int namec;
    struct font *font;
  } *fontv;
  int fontc,fonta;
  
  // Focus ring.
  struct widget **focusv; // STRONG
  int focusc,focusa,focusp;
  
  uint8_t modifiers;
  struct widget *track; // STRONG
  int track_in;
  int mx,my;
};

extern struct gui_context *gui_global_context;

// If the current focus remains in the ring, it will remain focussed.
// Otherwise we blur the current and focus the first thing in the ring (or nothing, if the ring is empty).
void gui_rebuild_focus_ring(struct gui_context *ctx);

void gui_cb_close();
void gui_cb_resize(int w,int h);
void gui_cb_focus(int focus);
void gui_cb_expose(int x,int y,int w,int h);
void gui_cb_key(int keycode,int value,int codepoint);
void gui_cb_mmotion(int x,int y);
void gui_cb_mbutton(int btnid,int value);
void gui_cb_mwheel(int dx,int dy);

#endif
