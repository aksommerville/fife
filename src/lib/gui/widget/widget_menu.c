#include "lib/gui/gui_internal.h"

struct widget_menu {
  struct widget hdr;
  struct font *font;
  struct widget *content; // STRONG
  void (*cb)(struct widget *widget,void *userdata);
  void *userdata;
};

#define WIDGET ((struct widget_menu*)widget)

/* Cleanup.
 */
 
static void _menu_del(struct widget *widget) {
  gui_remove_modal(widget->ctx,WIDGET->content);
  font_del(WIDGET->font);
  widget_del(WIDGET->content);
}

/* Button clicked: Show our content.
 * It's not a toggle. If the content is up, we're not interactive.
 */
 
static void menu_cb_toggle(struct widget *button,void *userdata) {
  struct widget *widget=userdata;
  if (!WIDGET->content) return;
  int w=0,h=0;
  widget_measure(&w,&h,WIDGET->content,button->ctx->w,button->ctx->h);
  WIDGET->content->w=w;
  WIDGET->content->h=h;
  WIDGET->content->x=0;
  WIDGET->content->y=button->h;
  widget_coords_global_from_local(&WIDGET->content->x,&WIDGET->content->y,button);
  widget_pack(WIDGET->content);
  gui_add_modal(widget->ctx,WIDGET->content);
}

/* Option clicked: Fire our callback and dismiss the modal.
 */
 
static void menu_cb_option(struct widget *button,void *userdata) {
  struct widget *widget=userdata;
  gui_remove_modal(widget->ctx,WIDGET->content);
  if (WIDGET->cb) WIDGET->cb(button,WIDGET->userdata);
}

/* Init.
 */
 
static int _menu_init(struct widget *widget,const void *args,int argslen) {

  if (!args||(argslen!=sizeof(struct widget_args_menu))) return -1; // args required
  const struct widget_args_menu *ARGS=args;
  
  WIDGET->cb=ARGS->cb;
  WIDGET->userdata=ARGS->userdata;
  
  struct font *font=ARGS->font;
  if (!font) font=gui_get_default_font(widget->ctx);
  if (font_ref(font)<0) return -1;
  WIDGET->font=font;
  
  struct widget_args_button button_args={
    .font=font,
    .text=ARGS->text,
    .textc=ARGS->textc,
    .cb=menu_cb_toggle,
    .userdata=widget,
  };
  struct widget *button=widget_spawn(widget,&widget_type_button,&button_args,sizeof(button_args));
  if (!button) return -1;
  
  struct widget_args_packer packer_args={
    .orientation='y',
    .majoralign=-1,
    .minoralign=-2,
  };
  if (!(WIDGET->content=widget_new(widget->ctx,&widget_type_packer,&packer_args,sizeof(packer_args)))) return -1;
  
  return 0;
}

/* Render.
 */
 
static void _menu_render(struct widget *widget,struct image *dst) {
}

/* Measure.
 */
 
static void _menu_measure(int *w,int *h,struct widget *widget,int maxw,int maxh) {
  if (widget->childc<1) return;
  struct widget *button=widget->childv[0];
  return widget_measure(w,h,button,maxw,maxh);
}

/* Pack.
 */
 
static void _menu_pack(struct widget *widget) {
  if (widget->childc<1) return;
  struct widget *button=widget->childv[0];
  button->x=0;
  button->y=0;
  button->w=widget->w;
  button->h=widget->h;
  widget_pack(button);
}

/* Type definition.
 */
 
const struct widget_type widget_type_menu={
  .name="menu",
  .objlen=sizeof(struct widget_menu),
  .del=_menu_del,
  .init=_menu_init,
  .render=_menu_render,
  .autorender=1,
  .measure=_menu_measure,
  .pack=_menu_pack,
};

/* Spawn option.
 */
 
struct widget *widget_menu_spawn_option(struct widget *widget,const char *label,int labelc) {
  if (!widget||(widget->type!=&widget_type_menu)) return 0;
  struct widget_args_button args={
    .font=WIDGET->font,
    .text=label,
    .textc=labelc,
    .cb=menu_cb_option,
    .userdata=widget,
  };
  struct widget *button=widget_spawn(WIDGET->content,&widget_type_button,&args,sizeof(args));
  if (!button) return 0;
  return button;
}
