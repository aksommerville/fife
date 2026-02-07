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
  widget_del(ctx->root);
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
  
  // Acquire the framebuffer, to force it to exist. X11 can't properly convert colors until this happens.
  int _w,_h,_stride;
  if (!wm_get_framebuffer(&_w,&_h,&_stride)) {
    gui_context_del(ctx);
    return 0;
  }
  
  wm_get_size(&ctx->w,&ctx->h);
  
  return ctx;
}

/* Render (not in response to an exposure).
 */
 
static void gui_render(struct gui_context *ctx) {
  if (!ctx->root) return;
  int fbw=0,fbh=0,stride=0;
  void *fb=wm_get_framebuffer(&fbw,&fbh,&stride);
  if (!fb) return;
  if ((ctx->root->w!=fbw)||(ctx->root->h!=fbh)) return; // This shouldn't happen, and I don't know what to make of it.
  struct image image={
    .v=fb,
    .w=fbw,
    .h=fbh,
    .stride=stride,
    .pixelsize=32,
    .writeable=1,
  };
  ctx->root->type->render(ctx->root,&image);
  wm_framebuffer_dirty(0,0,fbw,fbh);
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
    if (ctx->render_soon) {
      ctx->render_soon=0;
      gui_render(ctx);
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

/* Create root widget.
 */
 
struct widget *gui_context_create_root(
  struct gui_context *ctx,
  const struct widget_type *type,
  const void *args,int argslen
) {
  if (!ctx) return 0;
  if (ctx->root) return 0;
  struct widget *widget=widget_new(ctx,type,args,argslen);
  if (!widget) return 0;
  ctx->root=widget;
  widget->x=0;
  widget->y=0;
  widget->w=ctx->w;
  widget->h=ctx->h;
  return widget;
}
