#include "lib/gui/gui_internal.h"

struct widget_label {
  struct widget hdr;
  char *text;
  int textc;
  struct font *font;
  int stringw,stringh;
  uint32_t fgcolor;
};

#define WIDGET ((struct widget_label*)widget)

/* Cleanup.
 */
 
static void _label_del(struct widget *widget) {
  if (WIDGET->text) free(WIDGET->text);
  font_del(WIDGET->font);
}

/* Init.
 */
 
static int _label_init(struct widget *widget,const void *args,int argslen) {
  if (argslen==sizeof(struct widget_args_label)) {
    const struct widget_args_label *ARGS=args;
    if (ARGS->text) widget_label_set_text(widget,ARGS->text,ARGS->textc);
    if (ARGS->font) widget_label_set_font(widget,ARGS->font);
    WIDGET->fgcolor=ARGS->fgcolor;
  }
  if (!WIDGET->font) {
    if (widget_label_set_font(widget,gui_get_default_font(widget->ctx))<0) return -1;
  }
  return 0;
}

/* Measure.
 */
 
static void _label_measure(int *w,int *h,struct widget *widget,int maxw,int maxh) {
  *w=WIDGET->stringw+(widget->padx<<1);
  *h=WIDGET->stringh+(widget->pady<<1);
}

/* Render.
 */
 
static void _label_render(struct widget *widget,struct image *dst) {
  int dstx=(widget->w>>1)-(WIDGET->stringw>>1);
  int dsty=(widget->h>>1)-(WIDGET->stringh>>1);
  font_set_color_normal(WIDGET->font,WIDGET->fgcolor);
  font_render_string(dst,dstx,dsty,WIDGET->font,WIDGET->text,WIDGET->textc);
}

/* Type definition.
 */
 
const struct widget_type widget_type_label={
  .name="label",
  .objlen=sizeof(struct widget_label),
  .autorender=1,
  .del=_label_del,
  .init=_label_init,
  .measure=_label_measure,
  .render=_label_render,
};

/* Public accessors.
 */
 
int widget_label_set_text(struct widget *widget,const char *src,int srcc) {
  if (!widget||(widget->type!=&widget_type_label)) return -1;
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

int widget_label_set_font(struct widget *widget,struct font *font) {
  if (!widget||(widget->type!=&widget_type_label)) return -1;
  if (font_ref(font)<0) return -1;
  font_del(WIDGET->font);
  WIDGET->font=font;
  WIDGET->stringh=font_get_height(font);
  WIDGET->stringw=font_measure_string(WIDGET->font,WIDGET->text,WIDGET->textc);
  return 0;
}

int widget_label_set_fgcolor(struct widget *widget,uint32_t pixel) {
  if (!widget||(widget->type!=&widget_type_label)) return -1;
  WIDGET->fgcolor=pixel;
  widget->ctx->render_soon=1;
  return 0;
}
