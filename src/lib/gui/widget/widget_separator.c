#include "lib/gui/gui_internal.h"

struct widget_separator {
  struct widget hdr;
  char orient; // 'x' or 'y'
  int size; // pixels
  int pct;
  int barw; // or height, if orient=='y'
  uint32_t linecolor;
};

#define WIDGET ((struct widget_separator*)widget)

/* Cleanup.
 */
 
static void _separator_del(struct widget *widget) {
}

/* Init.
 */
 
static int _separator_init(struct widget *widget,const void *args,int argslen) {
  if (args&&(argslen==sizeof(struct widget_args_separator))) {
    const struct widget_args_separator *ARGS=args;
    WIDGET->orient=ARGS->orient;
    WIDGET->size=ARGS->size;
    WIDGET->pct=ARGS->pct;
  }
  if (!WIDGET->orient) WIDGET->orient='x';
  if (!WIDGET->size&&!WIDGET->pct) WIDGET->pct=50;
  if ((WIDGET->orient!='x')&&(WIDGET->orient!='y')) return -1;
  if (WIDGET->size&&WIDGET->pct) return -1;
  if ((WIDGET->size<0)||(WIDGET->pct<0)) return -1;
  if (!widget_spawn(widget,&widget_type_dummy,0,0)) return -1;
  if (!widget_spawn(widget,&widget_type_dummy,0,0)) return -1;
  WIDGET->barw=5;
  WIDGET->linecolor=wm_pixel_from_rgbx(0x404040ff);
  return 0;
}

/* Render.
 */
 
static void _separator_render(struct widget *widget,struct image *dst) {
  if (widget->childc!=2) return;
  struct widget *ch0=widget->childv[0];
  struct widget *ch1=widget->childv[1];
  if (WIDGET->orient=='x') {
    image_fill_rect(dst,ch0->x+ch0->w,0,1,widget->h,WIDGET->linecolor);
    image_fill_rect(dst,ch1->x-1,0,1,widget->h,WIDGET->linecolor);
  } else {
    image_fill_rect(dst,0,ch0->y+ch0->h,widget->w,1,WIDGET->linecolor);
    image_fill_rect(dst,0,ch1->y-1,widget->w,1,WIDGET->linecolor);
  }
}

/* Measure.
 */
 
static void _separator_measure(int *w,int *h,struct widget *widget,int maxw,int maxh) {
  if (widget->childc!=2) return;
  int ch0w=maxw,ch0h=maxh,ch1w=maxw,ch1h=maxh;
  widget_measure(&ch0w,&ch0h,widget->childv[0],maxw,maxh);
  widget_measure(&ch1w,&ch1h,widget->childv[1],maxw,maxh);
  if (WIDGET->orient=='x') {
    *w=ch0w+WIDGET->barw+ch1w;
    *h=(ch0h>ch1h)?ch0h:ch1h;
  } else {
    *w=(ch0w>ch1w)?ch0w:ch1w;
    *h=ch0h+WIDGET->barw+ch1h;
  }
  (*w)+=widget->padx<<1;
  (*h)+=widget->pady<<1;
}

/* Pack.
 */
 
static void _separator_pack(struct widget *widget) {
  if (widget->childc!=2) return;
  struct widget *ch0=widget->childv[0];
  struct widget *ch1=widget->childv[1];
  int innerw=widget->w-(widget->padx<<1);
  int innerh=widget->h-(widget->pady<<1);
  if (innerw<0) innerw=0;
  if (innerh<0) innerh=0;
  if (WIDGET->orient=='x') {
    ch0->h=innerh;
    ch1->h=innerh;
    if (WIDGET->size) ch0->w=WIDGET->size;
    else ch0->w=((innerw-WIDGET->barw)*WIDGET->pct)/100;
    ch1->w=innerw-WIDGET->barw-ch0->w;
  } else {
    ch0->w=innerw;
    ch1->w=innerw;
    if (WIDGET->size) ch0->h=WIDGET->size;
    else ch0->h=((innerh-WIDGET->barw)*WIDGET->pct)/100;
    ch1->h=innerh-WIDGET->barw-ch0->h;
  }
  if (ch0->w<0) ch0->w=0;
  if (ch0->h<0) ch0->h=0;
  if (ch1->w<0) ch1->w=0;
  if (ch1->h<0) ch1->h=0;
  ch0->x=widget->padx;
  ch0->y=widget->pady;
  if (WIDGET->orient=='x') {
    ch1->x=ch0->x+ch0->w+WIDGET->barw;
    ch1->y=widget->pady;
  } else {
    ch1->x=widget->padx;
    ch1->y=ch0->y+ch0->h+WIDGET->barw;
  }
  widget_pack(ch0);
  widget_pack(ch1);
}

/* Type definition.
 */
 
const struct widget_type widget_type_separator={
  .name="separator",
  .objlen=sizeof(struct widget_separator),
  .del=_separator_del,
  .init=_separator_init,
  .render=_separator_render,
  .autorender=1,
  .measure=_separator_measure,
  .pack=_separator_pack,
};

/* Get panel.
 */

struct widget *widget_separator_get_panel(const struct widget *widget,int side) {
  if (!widget||(widget->type!=&widget_type_separator)) return 0;
  side=side?1:0;
  if (side>=widget->childc) return 0; // Shouldn't be possible but easy to check.
  struct widget *panel=widget->childv[side];
  if (panel->type==&widget_type_dummy) return 0;
  return panel;
}

/* Replace panel.
 */

struct widget *widget_separator_spawn_panel(struct widget *widget,int side,const struct widget_type *type,const void *args,int argslen) {
  if (!widget) return 0;
  // Mostly as a convenience to widget_dashboard, we defer to regular spawn if called with anything that isn't a separator.
  if (widget->type!=&widget_type_separator) return widget_spawn(widget,type,args,argslen);
  side=side?1:0;
  if (side>=widget->childc) return 0; // Shouldn't be possible.
  if (!type) {
    type=&widget_type_dummy;
  }
  struct widget *panel=widget_new(widget->ctx,type,args,argslen); // NB STRONG
  if (!panel) return 0;
  widget_childv_remove_at(widget,side);
  int err=widget_childv_insert(widget,side,panel);
  widget_del(panel);
  if (err<0) return 0;
  return panel;
}
