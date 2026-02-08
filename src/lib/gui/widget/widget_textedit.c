#include "lib/gui/gui_internal.h"

struct widget_textedit {
  struct widget hdr;
};

#define WIDGET ((struct widget_textedit*)widget)

/* Cleanup.
 */
 
static void _textedit_del(struct widget *widget) {
}

/* Init.
 */
 
static int _textedit_init(struct widget *widget,const void *args,int argslen) {
  return 0;
}

/* Render.
 */
 
static void _textedit_render(struct widget *widget,struct image *dst) {
  image_fill_rect(dst,0,0,widget->w,widget->h,0);
  image_fill_rect(dst,10-widget->scrollx,10-widget->scrolly,90,40,0xffffffff);
  //TODO
}

/* Type definition.
 */
 
const struct widget_type widget_type_textedit={
  .name="texedit",
  .objlen=sizeof(struct widget_textedit),
  .del=_textedit_del,
  .init=_textedit_init,
  .render=_textedit_render,
};
