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
 
static void cb_1(struct widget *widget,void *userdata) {
  fprintf(stderr,"Thou hast done well in clicking the button.\n");
}

static void cb_2(struct widget *widget,void *userdata) {
  fprintf(stderr,"%s\n",__func__);
}

static void cb_3(struct widget *widget,void *userdata) {
  fprintf(stderr,"%s\n",__func__);
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
    struct widget_args_button args={
      .text="Click me!",
      .textc=-1,
      .cb=cb_1,
    };
    if (child=widget_spawn(root,&widget_type_button,&args,sizeof(args))) {
    }
  }
  {
    struct widget_args_button args={
      .text="No, me!",
      .textc=-1,
      .cb=cb_2,
    };
    if (child=widget_spawn(root,&widget_type_button,&args,sizeof(args))) {
    }
  }
  {
    struct widget_args_button args={
      .text="Click somebody else.",
      .textc=-1,
      .cb=cb_3,
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
