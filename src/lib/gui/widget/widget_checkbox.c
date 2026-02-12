#include "lib/gui/gui_internal.h"

#define CHECKBOXW 16
#define CHECKBOXH 16

struct widget_checkbox {
  struct widget hdr;
  struct widget_args_checkbox args;
  int focus;
  int track;
};

#define WIDGET ((struct widget_checkbox*)widget)

/* Init.
 */
 
static int _checkbox_init(struct widget *widget,const void *args,int argslen) {
  WIDGET->args.enable=1;
  if (argslen==sizeof(struct widget_args_checkbox)) {
    WIDGET->args=*(const struct widget_args_checkbox*)args;
  }
  if (WIDGET->args.enable) {
    widget->focusable=1;
    widget->clickable=1;
  }
  return 0;
}

/* Measure.
 */
 
static void _checkbox_measure(int *w,int *h,struct widget *widget,int maxw,int maxh) {
  *w=(widget->padx<<1)+CHECKBOXW;
  *h=(widget->pady<<1)+CHECKBOXH;
}

/* Activate.
 */
 
static void _checkbox_activate(struct widget *widget) {
  if (!WIDGET->args.enable) return;
  if (WIDGET->args.value) {
    WIDGET->args.value=0;
  } else {
    WIDGET->args.value=1;
  }
  if (WIDGET->args.cb) WIDGET->args.cb(widget,WIDGET->args.value,WIDGET->args.userdata);
  widget->ctx->render_soon=1;
}

/* Focus.
 */
 
static void _checkbox_focus(struct widget *widget,int focus) {
  WIDGET->focus=focus;
  widget->ctx->render_soon=1;
}

/* Track.
 */
 
static int _checkbox_track(struct widget *widget,int state) {
  switch (state) {
    case GUI_TRACK_BEGIN:
    case GUI_TRACK_REENTER: {
        if (!WIDGET->args.enable) return 0;
        WIDGET->track=1;
        widget->ctx->render_soon=1;
      } return 1;
    case GUI_TRACK_EXIT:
    case GUI_TRACK_END_OUT:
    case GUI_TRACK_END_IN: {
        WIDGET->track=0;
        widget->ctx->render_soon=1;
      } return 0;
  }
  return 0;
}

/* Render.
 */
 
static void _checkbox_render(struct widget *widget,struct image *dst) {
  // We'd like to always be a square, but widgets must accept the bounds their parents dictate.
  // Could handle mouse interactions on our own if we really wanted to force it, but meh, let's roll with it.
  int x=widget->padx;
  int y=widget->pady;
  int w=widget->w-(widget->padx<<1);
  int h=widget->h-(widget->pady<<1);
  uint32_t bgcolor=0xffffffff,framecolor=0x00000000,dotscolor=0x00000000;
  if (!WIDGET->args.enable) {
    bgcolor=0xc0c0c0c0;
    framecolor=0x40404040;
    dotscolor=0x40404040;
  }
  image_fill_rect(dst,x,y,w,h,bgcolor);
  image_frame_rect(dst,x,y,w,h,framecolor);
  if (WIDGET->track) {
    image_frame_rect(dst,x+1,y+1,w-2,h-2,framecolor);
  }
  if (WIDGET->focus) {
    image_frame_rect_dotted(dst,x+2,y+2,w-4,h-4,dotscolor);
  }
  if (WIDGET->args.value) {
    image_fill_rect(dst,x+3,y+3,w-6,h-6,wm_pixel_from_rgbx(0x0000ffff));
  }
}

/* Type definition.
 */
 
const struct widget_type widget_type_checkbox={
  .name="checkbox",
  .objlen=sizeof(struct widget_checkbox),
  .autorender=1,
  .init=_checkbox_init,
  .measure=_checkbox_measure,
  .focus=_checkbox_focus,
  .activate=_checkbox_activate,
  .track=_checkbox_track,
  .render=_checkbox_render,
};
