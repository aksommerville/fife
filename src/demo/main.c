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
  if (++sigc>=3) {
    fprintf(stderr,"Too many unprocessed signals.\n");
    exit(1);
  }
}

static void cb_resize(int w,int h) {
  //fprintf(stderr,"%s %d,%d\n",__func__,w,h);
}

static void cb_focus(int focus) {
  fprintf(stderr,"%s %d\n",__func__,focus);
}

static void cb_expose(int x,int y,int w,int h) {
  //fprintf(stderr,"%s %d,%d,%d,%d\n",__func__,x,y,w,h);
  int fbw=0,fbh=0,fbstride=0;
  uint32_t *fb=wm_get_framebuffer(&fbw,&fbh,&fbstride);
  if (!fb) return;
  if (fbstride&3) return;
  int fbstridewords=fbstride>>2;
  uint32_t *dstrow=fb;
  int yi=fbh;
  uint32_t black=wm_pixel_from_rgbx(0x000000ff);
  uint32_t white=wm_pixel_from_rgbx(0xffffffff);
  for (;yi-->0;dstrow+=fbstridewords) {
    uint32_t *dstp=dstrow;
    int xi=fbw;
    for (;xi-->0;dstp++) {
      *dstp=((yi&1)==(xi&1))?white:black;
    }
  }
  wm_framebuffer_dirty(x,y,w,h);
}

static int cb_key(int keycode,int value) {
  fprintf(stderr,"%s 0x%08x=%d\n",__func__,keycode,value);
  return 0; // Or nonzero to consume.
}

static void cb_text(int codepoint) {
  fprintf(stderr,"%s U+%x\n",__func__,codepoint);
}

static void cb_mmotion(int x,int y) {
  //fprintf(stderr,"%s %d,%d\n",__func__,x,y);
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
