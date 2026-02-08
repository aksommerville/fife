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
  
  // First in the list is our default.
  struct font_entry {
    char *name;
    int namec;
    struct font *font;
  } *fontv;
  int fontc,fonta;
};

extern struct gui_context *gui_global_context;

void gui_cb_close();
void gui_cb_resize(int w,int h);
void gui_cb_focus(int focus);
void gui_cb_expose(int x,int y,int w,int h);
int gui_cb_key(int keycode,int value);
void gui_cb_text(int codepoint);
void gui_cb_mmotion(int x,int y);
void gui_cb_mbutton(int btnid,int value);
void gui_cb_mwheel(int dx,int dy);

#endif
