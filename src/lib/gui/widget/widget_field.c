#include "lib/gui/gui_internal.h"

struct widget_field {
  struct widget hdr;
  struct font *font;
  char *text;
  int textc,texta;
  int focus;
  uint32_t fgcolor;
  uint32_t highlight_color;
  uint32_t cursor_color;
  int selp,selc; // Selection start and length (0..textc). (selc) may be negative, if the business end is on the right.
  int selx,selw; // Selection range in pixels. Set (selw<0) to invalidate, and render will recalculate both.
  void *userdata;
  int (*cb_preedit)(struct widget *widget,const char *before,int beforec,int p,int c,const char *incoming,int incomingc);
  void (*cb_postedit)(struct widget *widget,const char *text,int textc,int editp);
};

#define WIDGET ((struct widget_field*)widget)

/* Cleanup.
 */
 
static void _field_del(struct widget *widget) {
  if (WIDGET->text) free(WIDGET->text);
  font_del(WIDGET->font);
}

/* Init.
 */
 
static int _field_init(struct widget *widget,const void *args,int argslen) {
  if (argslen==sizeof(struct widget_args_field)) {
    const struct widget_args_field *ARGS=args;
    if (ARGS->text&&(widget_field_set_text(widget,ARGS->text,ARGS->textc)<0)) return -1;
    if (ARGS->font&&(widget_field_set_font(widget,ARGS->font)<0)) return -1;
    WIDGET->userdata=ARGS->userdata;
    WIDGET->cb_preedit=ARGS->cb_preedit;
    WIDGET->cb_postedit=ARGS->cb_postedit;
  }
  if (!WIDGET->font&&(widget_field_set_font(widget,gui_get_default_font(widget->ctx))<0)) return -1;
  widget->bgcolor=        wm_pixel_from_rgbx(0xffffffff);
  WIDGET->fgcolor=        wm_pixel_from_rgbx(0x000000ff);
  WIDGET->highlight_color=wm_pixel_from_rgbx(0x40c0ffff);
  WIDGET->cursor_color=   wm_pixel_from_rgbx(0x000000ff);
  widget->padx=5;
  widget->pady=3;
  widget->focusable=1;
  WIDGET->selw=-1;
  return 0;
}

/* Measure.
 */
 
static void _field_measure(int *w,int *h,struct widget *widget,int maxw,int maxh) {
  *w=font_get_width(WIDGET->font)*40+(widget->padx<<1);
  *h=font_get_height(WIDGET->font)+(widget->pady<<1);
}

/* Render.
 */
 
static void _field_render(struct widget *widget,struct image *image) {
  //TODO We need a whole different color palette for when blurred.

  // Refresh selection geometry if dirty.
  if (WIDGET->selw<0) {
    int p=WIDGET->selp;
    int c=WIDGET->selc;
    if (c<0) {
      p+=c;
      c=-c;
    }
    WIDGET->selx=font_measure_string(WIDGET->font,WIDGET->text,p)+widget->padx;
    WIDGET->selw=c?font_measure_string(WIDGET->font,WIDGET->text+p,c):0;
  }
  
  // If there's a selected range, fill it.
  if (WIDGET->selw) {
    image_fill_rect(image,WIDGET->selx,widget->pady,WIDGET->selw,widget->h-(widget->pady<<1),WIDGET->highlight_color);
  
  // No selection, draw a skinny cursor.
  } else {
    image_fill_rect(image,WIDGET->selx,widget->pady,1,widget->h-(widget->pady<<1),WIDGET->cursor_color);
  }
  
  // The text.
  font_set_color_normal(WIDGET->font,WIDGET->fgcolor);
  font_render_string(image,widget->padx,widget->pady,WIDGET->font,WIDGET->text,WIDGET->textc);
  
  // Outer frame.
  image_frame_rect(image,0,0,widget->w,widget->h,0x00000000);
}

/* Gain or lose focus.
 */
 
static void _field_focus(struct widget *widget,int focus) {
  WIDGET->focus=focus;
}

/* Keystroke.
 */
 
static int _field_key(struct widget *widget,int keycode,int value,int codepoint) {
  //fprintf(stderr,"%s keycode=0x%08x value=%d codepoint=U+%x\n",__func__,keycode,value,codepoint);
  
  /* Check for non-text keys.
   * Tempting to say (&&!codepoint) here, but some like Enter and Escape usually do have codepoints.
   */
  if (value) switch (keycode) {
    case 0x00070028: break; // Enter
    case 0x00070029: break; // Escape
    case 0x0007002a: widget_field_backspace(widget); return 1; // Backspace
    case 0x00070049: break; // Insert
    case 0x0007004a: widget_field_set_selection(widget,0,0); return 1; // Home
    case 0x0007004b: break; // Page Up
    case 0x0007004c: widget_field_delete(widget); return 1; // Delete
    case 0x0007004d: widget_field_set_selection(widget,WIDGET->textc,0); return 1; // End
    case 0x0007004e: break; // Page Down
    case 0x0007004f: widget_field_move_cursor(widget,1,0); return 1; // Right
    case 0x00070050: widget_field_move_cursor(widget,-1,0); return 1; // Left
    case 0x00070051: widget_field_move_cursor(widget,0,1); return 1; // Down
    case 0x00070052: widget_field_move_cursor(widget,0,-1); return 1; // Up
  }
  
  /* Is it text?
   */
  if (value&&codepoint) {
    if (codepoint<0x20) {
      // Reject C0.
    } else if ((codepoint>=0x7f)&&(codepoint<0xa0)) {
      // Reject C1.
    } else {
      // Everything else is ok.
      widget_field_insert_codepoint(widget,codepoint);
      return 1;
    }
  }
  
  return 0;
}

/* Type definition.
 */
 
const struct widget_type widget_type_field={
  .name="field",
  .objlen=sizeof(struct widget_field),
  .autorender=1,
  .del=_field_del,
  .init=_field_init,
  .measure=_field_measure,
  .render=_field_render,
  .focus=_field_focus,
  .key=_field_key,
};

/* Public accessors.
 */
 
int widget_field_get_text(void *dstpp,const struct widget *widget) {
  if (!widget||(widget->type!=&widget_type_field)) return 0;
  if (dstpp) *(void**)dstpp=WIDGET->text;
  return WIDGET->textc;
}

int widget_field_set_text(struct widget *widget,const char *src,int srcc) {
  if (!widget||(widget->type!=&widget_type_field)) return -1;
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
  WIDGET->texta=srcc;
  WIDGET->selp=WIDGET->textc;
  WIDGET->selc=0;
  WIDGET->selw=-1;
  widget->ctx->render_soon=1;
  return 0;
}

int widget_field_handoff_text(struct widget *widget,char *src,int srcc) {
  if (!widget||(widget->type!=&widget_type_field)) return -1;
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  if (WIDGET->text) free(WIDGET->text);
  WIDGET->text=src;
  WIDGET->textc=srcc;
  WIDGET->texta=srcc;
  WIDGET->selp=WIDGET->textc;
  WIDGET->selc=0;
  WIDGET->selw=-1;
  widget->ctx->render_soon=1;
  return 0;
}

int widget_field_set_font(struct widget *widget,struct font *font) {
  if (!widget||(widget->type!=&widget_type_field)) return -1;
  if (font_ref(font)<0) return -1;
  font_del(WIDGET->font);
  WIDGET->font=font;
  widget->ctx->render_soon=1;
  return 0;
}

void *widget_field_get_userdata(const struct widget *widget) {
  if (!widget||(widget->type!=&widget_type_field)) return 0;
  return WIDGET->userdata;
}

int widget_field_set_userdata(struct widget *widget,void *userdata) {
  if (!widget||(widget->type!=&widget_type_field)) return -1;
  WIDGET->userdata=userdata;
  return 0;
}

int widget_field_get_selection(int *p,const struct widget *widget) {
  if (!widget||(widget->type!=&widget_type_field)) return -1;
  int sp=WIDGET->selp;
  int sc=WIDGET->selc;
  if (sc<0) {
    sp+=sc;
    sc=-sc;
  }
  if (p) *p=sp;
  return sc;
}

int widget_field_set_selection(struct widget *widget,int p,int c) {
  if (!widget||(widget->type!=&widget_type_field)) return -1;
  if (p<0) p=0; else if (p>WIDGET->textc) p=WIDGET->textc;
  if (c<0) c=WIDGET->textc-p; else if (p>WIDGET->textc-c) c=WIDGET->textc-p;
  WIDGET->selp=p;
  WIDGET->selc=c;
  WIDGET->selw=-1;
  widget->ctx->render_soon=1;
  return 0;
}

/* Push (selp) in the direction of (dx) until it lands on a UTF-8 sequence boundary.
 * TODO I guess this need to coordinate with font, so we assume boundaries in the same places?
 */
 
static void widget_field_force_legal_selp(struct widget *widget,int dx) {
  if (WIDGET->selp<0) WIDGET->selp=0;
  else if (WIDGET->selp>WIDGET->textc) WIDGET->selp=WIDGET->textc;
  if (dx<0) {
    if (WIDGET->selp>=WIDGET->textc) return;
    while (WIDGET->selp>0) {
      if ((WIDGET->text[WIDGET->selp]&0xc0)!=0x80) return;
      WIDGET->selp--;
    }
  } else {
    while (WIDGET->selp<WIDGET->textc) {
      if ((WIDGET->text[WIDGET->selp]&0xc0)!=0x80) return;
      WIDGET->selp++;
    }
  }
}

/* Move cursor.
 */
 
int widget_field_move_cursor(struct widget *widget,int dx,int dy) {
  if (!widget||(widget->type!=&widget_type_field)) return -1;
  //TODO Shift to extend selection.
  //TODO Control to jump by words.
  // Since we're single-line only, at least for the time being, (dy) is equivalent to Home or End.
  if (dy<0) {
    widget_field_set_selection(widget,0,0);
  } else if (dy>0) {
    widget_field_set_selection(widget,WIDGET->textc,0);
  } else if (!dx) {
    return 0; // Why did they call us?
  } else if (WIDGET->selc) {
    // There was a selection. First move releases it and leaves cursor on the indicated edge.
    if (dx<0) {
      if (WIDGET->selc<0) WIDGET->selp+=WIDGET->selc;
      WIDGET->selc=0;
    } else {
      if (WIDGET->selc>0) WIDGET->selp+=WIDGET->selc;
      WIDGET->selc=0;
    }
    WIDGET->selw=-1;
  } else {
    // No selection. Just move the cursor.
    WIDGET->selp+=dx;
    widget_field_force_legal_selp(widget,dx);
    widget->ctx->render_soon=1;
    WIDGET->selw=-1;
  }
  widget->ctx->render_soon=1;
  return 0;
}

/* Delete at cursor.
 */
 
static int widget_field_delete_selection(struct widget *widget) {
  int p=WIDGET->selp;
  int c=WIDGET->selc;
  if (c<0) {
    p+=c;
    c=-c;
  }
  //TODO cb_preedit
  WIDGET->textc-=c;
  memmove(WIDGET->text+p,WIDGET->text+p+c,WIDGET->textc-p);
  //TODO cb_postedit
  WIDGET->selp=p;
  WIDGET->selc=0;
  WIDGET->selw=-1;
  widget->ctx->render_soon=1;
  return 0;
}

int widget_field_delete(struct widget *widget) {
  if (!widget||(widget->type!=&widget_type_field)) return -1;
  if (WIDGET->selc) return widget_field_delete_selection(widget);
  if (WIDGET->selp>=WIDGET->textc) return 0;
  //TODO Assuming UTF-8 but maybe should be configurable.
  int rmc=1;
  while ((WIDGET->selp+rmc<WIDGET->textc)&&((WIDGET->text[WIDGET->selp+rmc]&0xc0)==0x80)) rmc++;
  //TODO cb_preedit
  WIDGET->textc-=rmc;
  memmove(WIDGET->text+WIDGET->selp,WIDGET->text+WIDGET->selp+rmc,WIDGET->textc-WIDGET->selp);
  //TODO cb_postedit
  widget->ctx->render_soon=1;
  return 0;
}

int widget_field_backspace(struct widget *widget) {
  if (!widget||(widget->type!=&widget_type_field)) return -1;
  if (WIDGET->selc) return widget_field_delete_selection(widget);
  if (WIDGET->selp<=0) return 0;
  //TODO Assuming UTF-8 but maybe should be configurable.
  int p=WIDGET->selp-1,c=1;
  while ((p>0)&&((WIDGET->text[p]&0xc0)==0x80)) { p--; c++; }
  //TODO cb_preedit
  WIDGET->textc-=c;
  memmove(WIDGET->text+p,WIDGET->text+p+c,WIDGET->textc-p);
  WIDGET->selp=p;
  WIDGET->selw=-1;
  //TODO cb_postedit
  widget->ctx->render_soon=1;
  return 0;
}

/* Insert one codepoint.
 */
 
int widget_field_insert_codepoint(struct widget *widget,int codepoint) {
  if (!widget||(widget->type!=&widget_type_field)) return -1;
  if (codepoint<0x20) return 0;
  if ((codepoint>=0x7f)&&(codepoint<0xa0)) return 0;
  if (codepoint>0x10ffff) return 0;
  
  //TODO Obviously this needs to live somewhere else.
  char encoded[4];
  int encodedc;
  if (codepoint<0x80) {
    encoded[0]=codepoint;
    encodedc=1;
  } else if (codepoint<0x800) {
    encoded[0]=0xc0|(codepoint>>6);
    encoded[1]=0x80|(codepoint&0x3f);
    encodedc=2;
  } else if (codepoint<0x10000) {
    encoded[0]=0xe0|(codepoint>>12);
    encoded[1]=0x80|((codepoint>>6)&0x3f);
    encoded[2]=0x80|(codepoint&0x3f);
    encodedc=3;
  } else if (codepoint<0x10ffff) {
    encoded[0]=0xf0|(codepoint>>18);
    encoded[1]=0x80|((codepoint>>12)&0x3f);
    encoded[2]=0x80|((codepoint>>6)&0x3f);
    encoded[3]=0x80|(codepoint&0x3f);
    encodedc=4;
  } else {
    return 0;
  }
  
  if (WIDGET->textc>WIDGET->texta-4) {
    int na=(WIDGET->texta+1028)&~1023;
    void *nv=realloc(WIDGET->text,na);
    if (!nv) return -1;
    WIDGET->text=nv;
    WIDGET->texta=na;
  }
  
  if (WIDGET->selc) {
    //TODO cb_preedit with both the removal and insertion
    widget_field_delete_selection(widget);
  } else {
    //TODO cb_preedit, insertion only
  }
  memmove(WIDGET->text+WIDGET->selp+encodedc,WIDGET->text+WIDGET->selp,WIDGET->textc-WIDGET->selp);
  WIDGET->textc+=encodedc;
  memcpy(WIDGET->text+WIDGET->selp,encoded,encodedc);
  WIDGET->selp+=encodedc;
  WIDGET->selw=-1;
  //TODO cb_postedit
  
  widget->ctx->render_soon=1;
  return 0;
}
