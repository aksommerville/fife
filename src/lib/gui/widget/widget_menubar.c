#include "lib/gui/gui_internal.h"

struct widget_menubar {
  struct widget hdr;
  struct font *font;
  uint32_t linecolor;
  void (*cb)(struct widget *widget,void *userdata);
  void *userdata;
};

#define WIDGET ((struct widget_menubar*)widget)

/* Cleanup.
 */
 
static void _menubar_del(struct widget *widget) {
  font_del(WIDGET->font);
}

/* Init.
 */
 
static int _menubar_init(struct widget *widget,const void *args,int argslen) {
  if (args&&(argslen==sizeof(struct widget_args_menubar))) {
    const struct widget_args_menubar *ARGS=args;
    if (ARGS->font) {
      if (font_ref(ARGS->font)<0) return -1;
      WIDGET->font=ARGS->font;
    }
    WIDGET->cb=ARGS->cb;
    WIDGET->userdata=ARGS->userdata;
  }
  if (!WIDGET->font) {
    struct font *font=gui_get_default_font(widget->ctx);
    if (font_ref(font)<0) return -1;
    WIDGET->font=font;
  }
  widget->padx=5; // Between items, and the left edge.
  widget->pady=6; // Total extra space divided between top and bottom.
  WIDGET->linecolor=wm_pixel_from_rgbx(0x000000ff);
  widget->bgcolor=wm_pixel_from_rgbx(0xc0c0c0ff);
  return 0;
}

/* Render.
 */
 
static void _menubar_render(struct widget *widget,struct image *dst) {
  image_fill_rect(dst,0,widget->h-1,widget->w,1,WIDGET->linecolor);
}

/* Measure.
 */
 
static void _menubar_measure(int *w,int *h,struct widget *widget,int maxw,int maxh) {
  *w=maxw;
  *h=font_get_height(WIDGET->font)+widget->pady;
}

/* Pack.
 */
 
static void _menubar_pack(struct widget *widget) {
  int all_child_height=font_get_height(WIDGET->font);
  int x=widget->padx;
  int y=(widget->h>>1)-(all_child_height>>1);
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    child->x=x;
    child->y=y;
    int chw=0,chh=0;
    widget_measure(&chw,&chh,child,widget->w-x,widget->h-y);
    if (child->x>widget->w-chw) chw=widget->w-child->x;
    if (chw<0) chw=0;
    child->w=chw;
    child->h=all_child_height;
    x+=chw+widget->padx;
    widget_pack(child);
  }
}

/* Type definition.
 */
 
const struct widget_type widget_type_menubar={
  .name="menubar",
  .objlen=sizeof(struct widget_menubar),
  .del=_menubar_del,
  .init=_menubar_init,
  .render=_menubar_render,
  .autorender=1,
  .measure=_menubar_measure,
  .pack=_menubar_pack,
};

/* Spawn menu.
 */
 
struct widget *widget_menubar_spawn_menu(struct widget *widget,const char *label,int labelc) {
  if (!widget||(widget->type!=&widget_type_menubar)) return 0;
  struct widget_args_menu args={
    .font=WIDGET->font,
    .text=label,
    .textc=labelc,
    .cb=WIDGET->cb,
    .userdata=WIDGET->userdata,
  };
  struct widget *menu=widget_spawn(widget,&widget_type_menu,&args,sizeof(args));
  if (!menu) return 0;
  return menu;
}
