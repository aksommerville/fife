#include "lib/gui/gui_internal.h"

#define EXTRA_PAD_TOP 2

struct widget_button {
  struct widget hdr;
  struct font *font;
  char *text;
  int textc;
  int stringw,stringh;
  int focus;
  int tracking;
  void (*cb)(struct widget *widget,void *userdata);
  void *userdata;
};

#define WIDGET ((struct widget_button*)widget)

/* Cleanup.
 */
 
static void _button_del(struct widget *widget) {
  font_del(WIDGET->font);
  if (WIDGET->text) free(WIDGET->text);
}

/* Init.
 */
 
static int _button_init(struct widget *widget,const void *args,int argslen) {
  if (argslen==sizeof(struct widget_args_button)) {
    const struct widget_args_button *ARGS=args;
    if (ARGS->textc&&(widget_button_set_text(widget,ARGS->text,ARGS->textc)<0)) return -1;
    if (ARGS->font&&(widget_button_set_font(widget,ARGS->font)<0)) return -1;
    WIDGET->cb=ARGS->cb;
    WIDGET->userdata=ARGS->userdata;
  }
  widget->bgcolor=0xc0c0c0c0;
  if (!WIDGET->font) {
    if (widget_button_set_font(widget,gui_get_default_font(widget->ctx))<0) return -1;
  }
  widget->padx=5;
  widget->pady=2;
  widget->focusable=1;
  widget->clickable=1;
  return 0;
}

/* Measure.
 */
 
static void _button_measure(int *w,int *h,struct widget *widget,int maxw,int maxh) {
  *w=WIDGET->stringw+(widget->padx<<1);
  *h=WIDGET->stringh+(widget->pady<<1)+EXTRA_PAD_TOP;
}

/* Render.
 */
 
static void _button_render(struct widget *widget,struct image *dst) {

  // Change background color while tracking.
  if (WIDGET->tracking) {
    image_fill_rect(dst,0,0,widget->w,widget->h,0x20202020);
  }
  
  // Text label.
  if (WIDGET->textc) {
    int stringx=(widget->w>>1)-(WIDGET->stringw>>1);
    int stringy=(widget->h>>1)-(WIDGET->stringh>>1)+EXTRA_PAD_TOP;
    font_set_color_normal(WIDGET->font,WIDGET->tracking?0xffffffff:0x00000000);
    font_render_string(dst,stringx,stringy,WIDGET->font,WIDGET->text,WIDGET->textc);
  }
  
  // Border.
  image_frame_rect(dst,0,0,widget->w,widget->h,0x00000000);
  
  // Focus indicator.
  if (WIDGET->focus) {
    image_frame_rect_dotted(dst,2,2,widget->w-4,widget->h-4,0x00000000);
  }
}

/* Gain or lose focus.
 */
 
static void _button_focus(struct widget *widget,int focus) {
  WIDGET->focus=focus;
  widget->ctx->render_soon=1;
}

/* Activate.
 */
 
static void _button_activate(struct widget *widget) {
  if (WIDGET->cb) WIDGET->cb(widget,WIDGET->userdata);
}

/* Track.
 */
 
static int _button_track(struct widget *widget,int state) {
  fprintf(stderr,"%s %d\n",__func__,state);
  switch (state) {
    case GUI_TRACK_BEGIN:
    case GUI_TRACK_REENTER: {
        WIDGET->tracking=1;
        widget->ctx->render_soon=1;
      } return 1;
    case GUI_TRACK_EXIT: {
        WIDGET->tracking=0;
        widget->ctx->render_soon=1;
      } return 0;
    case GUI_TRACK_END_OUT: return 0;
    case GUI_TRACK_END_IN: {
        WIDGET->tracking=0;
        widget->ctx->render_soon=1;
      } return 0; // 1 to suppress default activation. But let that happen.
  }
  return 0;
}

/* Type definition.
 */
 
const struct widget_type widget_type_button={
  .name="button",
  .objlen=sizeof(struct widget_button),
  .del=_button_del,
  .init=_button_init,
  .measure=_button_measure,
  .render=_button_render,
  .autorender=1,
  .focus=_button_focus,
  .activate=_button_activate,
  .track=_button_track,
};

/* Trivial public accessors.
 */
 
int widget_button_set_font(struct widget *widget,struct font *font) {
  if (!widget||(widget->type!=&widget_type_button)) return -1;
  if (font_ref(font)<0) return -1;
  font_del(WIDGET->font);
  WIDGET->font=font;
  WIDGET->stringw=font_measure_string(WIDGET->font,WIDGET->text,WIDGET->textc);
  WIDGET->stringh=font_get_height(WIDGET->font);
  return 0;
}

int widget_button_set_text(struct widget *widget,const char *src,int srcc) {
  if (!widget||(widget->type!=&widget_type_button)) return -1;
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  char *nv=0;
  if (srcc) {
    if (!(nv=malloc(srcc+1))) return -1;
    memcpy(nv,src,srcc);
    nv[srcc]=0;
  }
  if (WIDGET->text) free(WIDGET->text);
  WIDGET->text=nv;
  WIDGET->textc=srcc;
  WIDGET->stringw=font_measure_string(WIDGET->font,WIDGET->text,WIDGET->textc);
  return 0;
}

int widget_button_set_callback(struct widget *widget,void (*cb)(struct widget *widget,void *userdata),void *userdata) {
  if (!widget||(widget->type!=&widget_type_button)) return -1;
  WIDGET->cb=cb;
  WIDGET->userdata=userdata;
  return 0;
}
