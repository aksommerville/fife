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
  if (ctx==gui_global_context) gui_global_context=0;
  if (ctx->deferredv) {
    struct deferred *deferred=ctx->deferredv+ctx->deferredc;
    while (ctx->deferredc-->0) {
      deferred--;
      if (deferred->cb_cleanup) deferred->cb_cleanup(deferred->widget,deferred->userdata);
      widget_del(deferred->widget);
    }
    free(ctx->deferredv);
  }
  if (ctx->focusv) {
    while (ctx->focusc-->0) widget_del(ctx->focusv[ctx->focusc]);
    free(ctx->focusv);
  }
  widget_del(ctx->track);
  widget_del(ctx->root);
  if (ctx->fontv) {
    while (ctx->fontc-->0) font_entry_cleanup(ctx->fontv+ctx->fontc);
    free(ctx->fontv);
  }
  free(ctx);
}

/* New.
 */

struct gui_context *gui_context_new(const struct gui_delegate *delegate) {
  if (gui_global_context) return 0;
  
  struct gui_context *ctx=calloc(1,sizeof(struct gui_context));
  if (!ctx) return 0;
  gui_global_context=ctx;
  
  if (delegate) ctx->delegate=*delegate;
  if (ctx->delegate.update_rate<1.0) ctx->delegate.update_rate=60.0;
  ctx->focusp=-1;
  ctx->encoding=&text_encoding_utf8;
  
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
  widget_render(ctx->root,&image);
  wm_framebuffer_dirty(0,0,fbw,fbh);
}

/* Any deferred ready to run, run and remove it.
 */
 
static int gui_check_deferred_tasks(struct gui_context *ctx) {
  /* Careful here. Do not keep a pointer to the list, iterate backward, and remove the entry before calling it.
   * New entries go on the back, regardless of timestamps, so it's safe to iterate even when new things are being added.
   */
  int i=ctx->deferredc;
  while (i-->0) {
    struct deferred *deferred=ctx->deferredv+i;
    if (deferred->when>ctx->totalclock) continue;
    struct deferred tmp=*deferred;
    ctx->deferredc--;
    memmove(deferred,deferred+1,sizeof(struct deferred)*(ctx->deferredc-i));
    tmp.cb(tmp.widget,tmp.userdata);
    if (tmp.cb_cleanup) tmp.cb_cleanup(tmp.widget,tmp.userdata);
    widget_del(tmp.widget);
  }
  return 0;
}

/* Routine update.
 */
 
int gui_update(struct gui_context *ctx,double elapsed) {
  ctx->totalclock+=elapsed;
  if (gui_check_deferred_tasks(ctx)<0) return -1;
  if (ctx->tree_changed) {
    ctx->tree_changed=0;
    widget_pack(ctx->root);
    gui_rebuild_focus_ring(ctx);
    ctx->render_soon=1;
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
  struct font *font=font_new_from_path(path,ctx->encoding);
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

/* Schedule deferred task.
 */
 
int gui_defer_widget_task(struct widget *widget,double delay_s,void (*cb)(struct widget *widget,void *userdata),void *userdata) {
  if (!widget||!widget->ctx||!cb) return -1;
  if (delay_s<0.0) delay_s=0.0; // Zero is fine (I guess even negative would be). That means "defer to the next cycle", a sane thing to ask.
  else if (delay_s>60.0) return -1; // ...but we're not delaying all day. 60 seconds is ridiculous.
  if (widget->ctx->deferredc>=widget->ctx->deferreda) {
    int na=widget->ctx->deferreda+8;
    if (na>INT_MAX/sizeof(struct deferred)) return -1;
    void *nv=realloc(widget->ctx->deferredv,sizeof(struct deferred)*na);
    if (!nv) return -1;
    widget->ctx->deferredv=nv;
    widget->ctx->deferreda=na;
  }
  if (widget_ref(widget)<0) return -1;
  struct deferred *deferred=widget->ctx->deferredv+widget->ctx->deferredc++;
  memset(deferred,0,sizeof(struct deferred));
  deferred->widget=widget;
  deferred->when=widget->ctx->totalclock+delay_s;
  deferred->cb=cb;
  deferred->userdata=userdata;
  if (widget->ctx->taskid_next<1) widget->ctx->taskid_next=1;
  deferred->taskid=widget->ctx->taskid_next++;
  return deferred->taskid;
}

/* Cancel deferred task.
 */

void gui_cancel_task(struct gui_context *ctx,int taskid) {
  if (!ctx) return;
  if (taskid<1) return;
  struct deferred *deferred=ctx->deferredv;
  int i=0;
  for (;i<ctx->deferredc;i++,deferred++) {
    if (deferred->taskid!=taskid) continue;
    struct deferred tmp=*deferred;
    ctx->deferredc--;
    memmove(deferred,deferred+1,sizeof(struct deferred)*(ctx->deferredc-i));
    if (tmp.cb_cleanup) tmp.cb_cleanup(tmp.widget,tmp.userdata);
    widget_del(tmp.widget);
    return;
  }
}

/* Set task cleanup.
 */
 
int gui_set_task_cleanup(struct gui_context *ctx,int taskid,void (*cb_cleanup)(struct widget *widget,void *userdata)) {
  if (!ctx) return -1;
  struct deferred *deferred=ctx->deferredv;
  int i=ctx->deferredc;
  for (;i-->0;deferred++) {
    if (deferred->taskid!=taskid) continue;
    deferred->cb_cleanup=cb_cleanup;
    return 0;
  }
  return -1;
}
