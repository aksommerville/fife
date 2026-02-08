#include "lib/wm/wm.h"
#include "lib/gui/gui.h"
#include "lib/image/image.h"
#include "lib/font/font.h"
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

/* Main.
 */
 
int main(int argc,char **argv) {

  signal(SIGINT,rcvsig);
  
  struct gui_delegate delegate={
    .update_rate=60.0,
    .log_clock_at_quit=1,
    //TODO
  };
  struct gui_context *gui=gui_context_new(&delegate);
  if (!gui) {
    fprintf(stderr,"%s: gui_context_new failed\n",argv[0]);
    return 1;
  }
  
  /* textedit *
  struct widget_args_textedit args={
  };
  struct widget *root=gui_context_create_root(gui,&widget_type_textedit,&args,sizeof(args));
  if (!root) {
    fprintf(stderr,"%s: gui_context_create_root(textedit) failed\n",argv[0]);
    return 1;
  }
  /**/
  
  /* Simple set of widgets for testing. */
  struct widget *root=0,*child;
  {
    struct widget_args_packer args={
      .orientation='x',
      .reverse=0,
      .majoralign=0,
      .minoralign=0,
      .spacing=5,
    };
    root=gui_context_create_root(gui,&widget_type_packer,&args,sizeof(args));
    if (!root) {
      fprintf(stderr,"%s: Failed to create root widget.\n",argv[0]);
      return 1;
    }
    root->bgcolor=0x80808080;
    root->padx=5;
    root->pady=5;
  }
  if (child=widget_spawn(root,&widget_type_packer,0,0)) {
    child->bgcolor=0xff000000;
    child->padx=20;
    child->pady=20;
  }
  if (child=widget_spawn(root,&widget_type_packer,0,0)) {
    child->bgcolor=0x00ff0000;
    child->padx=20;
    child->pady=40;
    //widget_packer_flex_child(root,child,-1);
  }
  if (child=widget_spawn(root,&widget_type_packer,0,0)) {
    child->bgcolor=0x0000ff00;
    child->padx=40;
    child->pady=20;
  }
  if (child=widget_spawn(root,&widget_type_packer,0,0)) {
    child->bgcolor=0x000000ff;
    child->padx=40;
    child->pady=40;
  }
  /**/
  
  int result=gui_main(gui);
  fprintf(stderr,"%s: Result %d from gui_main.\n",argv[0],result);
  
  gui_context_del(gui);
  return result;
}
