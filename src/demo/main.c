#include "lib/wm/wm.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

static volatile int sigc=0;
static void rcvsig(int sigid) {
  switch (sigid) {
    case SIGINT: if (++sigc>=3) {
        fprintf(stderr,"Too many unprocessed signals.\n");
        exit(1);
      } break;
  }
}

/* WM callbacks.
 */

static void cb_close() {
  fprintf(stderr,"%s\n",__func__);
}

static void cb_resize(int w,int h) {
  fprintf(stderr,"%s %d,%d\n",__func__,w,h);
}

static void cb_focus(int focus) {
  fprintf(stderr,"%s %d\n",__func__,focus);
}

static void cb_expose(int x,int y,int w,int h) {
  fprintf(stderr,"%s %d,%d,%d,%d\n",__func__,x,y,w,h);
}

static int cb_key(int keycode,int value) {
  fprintf(stderr,"%s 0x%08x=%d\n",__func__,keycode,value);
  return 0; // Or nonzero to consume.
}

static void cb_text(int codepoint) {
  fprintf(stderr,"%s U+%x\n",__func__,codepoint);
}

static void cb_mmotion(int x,int y) {
  fprintf(stderr,"%s %d,%d\n",__func__,x,y);
}

static void cb_mbutton(int btnid,int value) {
  fprintf(stderr,"%s %d=%d\n",__func__,btnid,value);
}

static void cb_mwheel(int dx,int dy) {
  fprintf(stderr,"%s %+d,%+d\n",__func__,dx,dy);
}

/* Main.
 */

int main(int argc,char **argv) {

  signal(SIGINT,rcvsig);

  struct wm_delegate wmdelegate={
    .cb_close=cb_close,
    .cb_resize=cb_resize,
    .cb_focus=cb_focus,
    .cb_expose=cb_expose,
    .cb_key=cb_key,
    .cb_text=cb_text,
    .cb_mmotion=cb_mmotion,
    .cb_mbutton=cb_mbutton,
    .cb_mwheel=cb_mwheel,
  };
  if (wm_init(&wmdelegate)<0) {
    fprintf(stderr,"%s: wm_init failed\n",argv[0]);
    return 1;
  }
  
  while (!sigc) {
    if (wm_update()<0) {
      fprintf(stderr,"%s: wm_update failed\n",argv[0]);
      return 1;
    }
    usleep(100000);
  }
  
  wm_quit();
  fprintf(stderr,"%s: Normal exit.\n",argv[0]);
  return 0;
}
