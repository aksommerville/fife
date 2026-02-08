#include "gui_internal.h"

/* Global singleton.
 */

struct gui_context *gui_global_context=0;

struct gui_context *gui_get_context() {
  return gui_global_context;
}

/* Delete.
 */
 
static void font_entry_cleanup(struct font_entry *entry) {
  if (entry->name) free(entry->name);
  font_del(entry->font);
}
 
void gui_context_del(struct gui_context *ctx) {
  if (!ctx) return;
  wm_quit();
  widget_del(ctx->root);
  if (ctx->fontv) {
    while (ctx->fontc-->0) font_entry_cleanup(ctx->fontv+ctx->fontc);
    free(ctx->fontv);
  }
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
  if (ctx->delegate.update_rate<1.0) ctx->delegate.update_rate=60.0;
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
 
void gui_render(struct gui_context *ctx) {
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

/* Routine update.
 */
 
int gui_update(struct gui_context *ctx,double elapsed) {
  if (ctx->tree_changed) {
    ctx->tree_changed=0;
    //TODO Rebuild focus ring.
  }
  if (ctx->render_soon) {
    ctx->render_soon=0;
    gui_render(ctx);
  }
  return 0;
}

/* Main.
 */

int gui_main(struct gui_context *ctx) {
  if (!ctx||(ctx!=gui_global_context)) return 1;
  struct gui_clock clock;
  gui_clock_init(&clock,ctx->delegate.update_rate);
  while (!ctx->terminate) {
    if (wm_update()<0) {
      if (ctx->delegate.log_clock_at_quit>1) gui_clock_report(&clock);
      return 1;
    }
    double elapsed=gui_clock_tick(&clock);
    if (gui_update(ctx,elapsed)<0) {
      if (ctx->delegate.log_clock_at_quit>1) gui_clock_report(&clock);
      return 1;
    }
  }
  if (ctx->delegate.log_clock_at_quit) gui_clock_report(&clock);
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
  widget_pack(widget);
  return widget;
}

/* Font repository.
 */
 
struct font *gui_get_default_font(struct gui_context *ctx) {
  if (!ctx) return 0;
  if (ctx->fontc<1) return gui_get_named_font(ctx,"font_bold_g0_8x16",-1);//TODO
  return ctx->fontv[0].font;
}

struct font *gui_get_named_font(struct gui_context *ctx,const char *name,int namec) {
  if (!ctx) return 0;
  if (!name) return 0;
  if (namec<0) { namec=0; while (name[namec]) namec++; }
  if (!namec) return 0;
  
  struct font_entry *entry=ctx->fontv;
  int i=ctx->fontc;
  for (;i-->0;entry++) {
    if (entry->namec!=namec) continue;
    if (memcmp(entry->name,name,namec)) continue;
    return entry->font;
  }
  
  if (ctx->fontc>=ctx->fonta) {
    int na=ctx->fonta+8;
    if (na>INT_MAX/sizeof(struct font_entry)) return 0;
    void *nv=realloc(ctx->fontv,sizeof(struct font_entry)*na);
    if (!nv) return 0;
    ctx->fontv=nv;
    ctx->fonta=na;
  }
  
  char path[1024];
  int pathc=snprintf(path,sizeof(path),"src/lib/gui/img/%.*s.png",namec,name);//TODO
  if ((pathc<1)||(pathc>=sizeof(path))) return 0;
  struct font *font=font_new_from_path(path);
  if (!font) return 0;
  
  char *nname=malloc(namec+1);
  if (!nname) {
    font_del(font);
    return 0;
  }
  memcpy(nname,name,namec);
  nname[namec]=0;
  
  entry=ctx->fontv+ctx->fontc++;
  entry->name=nname;
  entry->namec=namec;
  entry->font=font;
  return font;
}
