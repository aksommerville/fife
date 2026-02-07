#include "lib/wm/wm.h"
#include "lib/gui/gui.h"
#include "lib/image/image.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

static volatile int sigc=0;
static void rcvsig(int sigid) {
  switch (sigid) {
    case SIGINT: gui_terminate_soon(gui_get_context(),1); if (++sigc>=3) {
        fprintf(stderr,"Too many unprocessed signals.\n");
        exit(1);
      } break;
  }
}

struct widget_colorbox {
  struct widget hdr;
  uint32_t color;
};

#define WIDGET ((struct widget_colorbox*)widget)

static int _colorbox_init(struct widget *widget,const void *args,int argslen) {
  if (argslen==sizeof(uint32_t)) {
    WIDGET->color=*(uint32_t*)args;
  }
  return 0;
}

static void _colorbox_render(struct widget *widget,struct image *dst) {
  image_fill_rect(dst,0,0,widget->w,widget->h,WIDGET->color);
  image_fill_rect(dst,widget->w-10,widget->h-10,10,10,0xffffffff);
  widget_render_children(widget,dst);
}

static const struct widget_type widget_type_colorbox={
  .name="colorbox",
  .objlen=sizeof(struct widget_colorbox),
  .init=_colorbox_init,
  .render=_colorbox_render,
};

/* Main.
 */
 
int main(int argc,char **argv) {

  signal(SIGINT,rcvsig);
  
  struct gui_delegate delegate={
    //TODO
  };
  struct gui_context *gui=gui_context_new(&delegate);
  if (!gui) {
    fprintf(stderr,"%s: gui_context_new failed\n",argv[0]);
    return 1;
  }
  
  uint32_t bgcolor=0x80808080;
  struct widget *root=gui_context_create_root(gui,&widget_type_colorbox,&bgcolor,sizeof(bgcolor));
  if (!root) {
    fprintf(stderr,"%s: gui_context_create_root failed\n",argv[0]);
    return 1;
  }
  
  // Four boxes. From left to right: Black, Red, Green, Blue.
  struct widget *child;
  bgcolor=wm_pixel_from_rgbx(0x000000ff);
  if (child=widget_spawn(root,&widget_type_colorbox,&bgcolor,sizeof(bgcolor))) {
    child->x=20;
    child->y=100;
    child->w=50;
    child->h=50;
  }
  bgcolor=wm_pixel_from_rgbx(0xff0000ff);
  if (child=widget_spawn(root,&widget_type_colorbox,&bgcolor,sizeof(bgcolor))) {
    child->x=80;
    child->y=100;
    child->w=50;
    child->h=50;
  }
  bgcolor=wm_pixel_from_rgbx(0x00ff00ff);
  if (child=widget_spawn(root,&widget_type_colorbox,&bgcolor,sizeof(bgcolor))) {
    child->x=140;
    child->y=100;
    child->w=50;
    child->h=50;
  }
  bgcolor=wm_pixel_from_rgbx(0x0000ffff);
  if (child=widget_spawn(root,&widget_type_colorbox,&bgcolor,sizeof(bgcolor))) {
    child->x=200;
    child->y=100;
    child->w=50;
    child->h=50;
  }
  
  int result=gui_main(gui);
  fprintf(stderr,"%s: Result %d from gui_main.\n",argv[0],result);
  
  gui_context_del(gui);
  return result;
}
