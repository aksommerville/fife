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
  
  struct widget_args_textedit args={
  };
  struct widget *root=gui_context_create_root(gui,&widget_type_textedit,&args,sizeof(args));
  if (!root) {
    fprintf(stderr,"%s: gui_context_create_root(textedit) failed\n",argv[0]);
    return 1;
  }
  
  int result=gui_main(gui);
  fprintf(stderr,"%s: Result %d from gui_main.\n",argv[0],result);
  
  gui_context_del(gui);
  return result;
}
