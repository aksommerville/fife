#include "gui_internal.h"

/* Global singleton.
 */

struct gui_context *gui_global_context=0;

struct gui_context *gui_get_context() {
  return gui_global_context;
}

/* Delete.
 */
 
void gui_context_del(struct gui_context *ctx) {
  if (!ctx) return;
  wm_quit();
  free(ctx);
  if (ctx==gui_global_context) gui_global_context=0;
}

/* New.
 */

struct gui_context *gui_context_new(const struct gui_delegate *delegate) {
  if (gui_global_context) return 0;
  
  struct gui_context *ctx=calloc(1,sizeof(struct gui_context));
  if (!ctx) return 0;
  if (delegate) ctx->delegate=*delegate;
  gui_global_context=ctx;
  
  struct wm_delegate wmdelegate={
    .cb_close=gui_cb_close,
    .cb_resize=gui_cb_resize,
    .cb_focus=gui_cb_focus,
    .cb_expose=gui_cb_expose,
    .cb_key=gui_cb_key,
    .cb_mmotion=gui_cb_mmotion,
    .cb_mbutton=gui_cb_mbutton,
    .cb_mwheel=gui_cb_mwheel,
  };
  if (wm_init(&wmdelegate)<0) {
    gui_context_del(ctx);
    return 0;
  }
  
  return ctx;
}

/* Main.
 */

int gui_main(struct gui_context *ctx) {
  fprintf(stderr,"%s...\n",__func__);
  if (!ctx||(ctx!=gui_global_context)) return 1;
  while (!ctx->terminate) {
    usleep(100000);//TODO Get smarter, with a poll or something.
    if (wm_update()<0) {
      return 1;
    }
  }
  return 0;
}

/* Request termination.
 */
 
void gui_terminate_soon(struct gui_context *ctx,int status) {
  if (!ctx) return;
  ctx->termstatus=status;
  ctx->terminate=1;
}
