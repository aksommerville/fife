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

/* Button callbacks.
 */

static void cb_ok(struct widget *widget,void *userdata) {
  fprintf(stderr,"%s\n",__func__);
}

static void cb_cancel(struct widget *widget,void *userdata) {
  fprintf(stderr,"%s\n",__func__);
}

static int cb_preedit(struct widget *widget,const char *text,int textc,int p,int c,const char *src,int srcc) {
  // Return nonzero to reject the change.
  //fprintf(stderr,"%s: Replacing '%.*s'(%d@%d) with '%.*s'\n",__func__,c,text+p,c,p,srcc,src);
  return 0;
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
      .orientation='y',
      .reverse=0,
      .majoralign=0,
      .minoralign=-2,
      .spacing=5,
    };
    root=gui_context_create_root(gui,&widget_type_packer,&args,sizeof(args));
    if (!root) {
      fprintf(stderr,"%s: Failed to create root widget.\n",argv[0]);
      return 1;
    }
    root->bgcolor=0x40404040;
    root->padx=5;
    root->pady=5;
  }
  {
    struct widget_args_label args={
      .text="What is your name?",
      .textc=-1,
      .fgcolor=0xffffffff,
    };
    if (child=widget_spawn(root,&widget_type_label,&args,sizeof(args))) {
    }
  }
  {
    struct widget_args_field args={
      .text="Untitled-1",
      .textc=-1,
      .cb_preedit=cb_preedit,
    };
    if (child=widget_spawn(root,&widget_type_field,&args,sizeof(args))) {
      widget_field_set_selection(child,0,-1);
    }
  }
  {
    struct widget_args_button args={
      .text="OK",
      .textc=-1,
      .cb=cb_ok,
    };
    if (child=widget_spawn(root,&widget_type_button,&args,sizeof(args))) {
    }
  }
  {
    struct widget_args_button args={
      .text="Cancel",
      .textc=-1,
      .cb=cb_cancel,
    };
    if (child=widget_spawn(root,&widget_type_button,&args,sizeof(args))) {
    }
  }
  /**/
  
  int result=gui_main(gui);
  fprintf(stderr,"%s: Result %d from gui_main.\n",argv[0],result);
  
  gui_context_del(gui);
  return result;
}
