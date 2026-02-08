#include "gui_internal.h"

/* Add one widget to the focus ring.
 */
 
static int gui_add_focus(struct gui_context *ctx,struct widget *widget) {
  if (ctx->focusc>=ctx->focusa) {
    int na=ctx->focusa+16;
    if (na>INT_MAX/sizeof(void*)) return -1;
    void *nv=realloc(ctx->focusv,sizeof(void*)*na);
    if (!nv) return -1;
    ctx->focusv=nv;
    ctx->focusa=na;
  }
  if (widget_ref(widget)<0) return -1;
  ctx->focusv[ctx->focusc++]=widget;
  return 0;
}

/* Build focus ring.
 * Must be empty at the start.
 * This is recursive. We first add (widget) itself if applicable, then recur into its children in order.
 */
 
static int gui_build_focus_ring(struct gui_context *ctx,struct widget *widget) {
  if (!widget) return 0;
  if (widget->focusable) {
    if (gui_add_focus(ctx,widget)<0) return -1;
  }
  int i=0;
  for (;i<widget->childc;i++) {
    if (gui_build_focus_ring(ctx,widget->childv[i])<0) return -1;
  }
  return 0;
}

/* Rebuild focus ring.
 */
 
void gui_rebuild_focus_ring(struct gui_context *ctx) {
  
  // Yoink the previous focus if present.
  struct widget *pvfocus=0;
  if ((ctx->focusp>=0)&&(ctx->focusp<ctx->focusc)) {
    pvfocus=ctx->focusv[ctx->focusc]; // HANDOFF, temporarily
    ctx->focusc--;
    memmove(ctx->focusv+ctx->focusp,ctx->focusv+ctx->focusp+1,sizeof(void*)*(ctx->focusc-ctx->focusp));
  }
  ctx->focusp=-1;
  
  // Drop the previous ring.
  while (ctx->focusc>0) {
    ctx->focusc--;
    widget_del(ctx->focusv[ctx->focusc]);
  }
  
  // Call out to build the new ring.
  gui_build_focus_ring(ctx,ctx->root);
  
  // If there was a focus before, either blur it or update our (focusp).
  if (pvfocus) {
    int np=-1,i=ctx->focusc;
    while (i-->0) {
      if (ctx->focusv[i]==pvfocus) {
        np=i;
        break;
      }
    }
    if (np>=0) { // Still here, cool.
      ctx->focusp=np;
    } else { // Not focusable anymore, probly was dropped. Tell it.
      if (pvfocus->type->focus) pvfocus->type->focus(pvfocus,0);
    }
    widget_del(pvfocus);
  }
  
  // If we don't now have a focus, use the first thing in the ring.
  if ((ctx->focusp<0)&&ctx->focusc) {
    ctx->focusp=0;
    struct widget *focus=ctx->focusv[0];
    if (focus->type->focus) focus->type->focus(focus,1);
  }
}

/* Public: Query focus or move relatively.
 */
 
struct widget *gui_focus_next(struct gui_context *ctx,int d) {
  if (!ctx) return 0;
  
  // What should be focussed next? Normalize OOB to -1.
  int np=-1;
  if (d<-1) { // Front of ring.
    np=0;
  } else if (d==-1) { // Previous, wrapping.
    if ((np=ctx->focusp-1)<0) np=ctx->focusc-1;
  } else if (!d) { // No change, just querying.
    np=ctx->focusp;
  } else if (d==1) { // Next, wrapping.
    if ((np=ctx->focusp+1)>=ctx->focusc) np=0;
  } else if (d>1) { // End of ring.
    np=ctx->focusc-1;
  }
  if ((np<0)||(np>=ctx->focusc)) np=-1;
  
  // If next is not current, blur the old and focus the new.
  if (np!=ctx->focusp) {
    if ((ctx->focusp>=0)&&(ctx->focusp<ctx->focusc)) {
      struct widget *widget=ctx->focusv[ctx->focusp];
      ctx->focusp=-1;
      if (widget->type->focus) widget->type->focus(widget,0);
    }
    if ((np>=0)&&(np<ctx->focusc)) {
      struct widget *widget=ctx->focusv[np];
      ctx->focusp=np;
      if (widget->type->focus) widget->type->focus(widget,1);
    }
  }
  
  // Return the new focus weakly.
  if ((ctx->focusp<0)||(ctx->focusp>=ctx->focusc)) {
    return ctx->focusv[ctx->focusp];
  }
  return 0;
}

/* Public: Set focus to a specific widget, or null for none.
 */

struct widget *gui_focus_widget(struct gui_context *ctx,struct widget *widget) {
  if (!ctx) return 0;
  
  // Null (widget) is a request to drop all focus.
  if (!widget) {
    if ((ctx->focusp>=0)&&(ctx->focusp<ctx->focusc)) {
      struct widget *prev=ctx->focusv[ctx->focusp];
      ctx->focusp=-1;
      if (prev->type->focus) prev->type->focus(prev,0);
    }
    return 0;
  }
  
  // Any other (widget) must be in the focus ring.
  int np=-1,i=ctx->focusc;
  while (i-->0) {
    if (ctx->focusv[i]==widget) {
      np=i;
      break;
    }
  }
  if (np<0) return 0; // Not focussing that trash, sorry bud.
  if (np==ctx->focusp) return widget; // Already focussed, cool.
  
  // Effect the change.
  if ((ctx->focusp>=0)&&(ctx->focusp<ctx->focusc)) {
    struct widget *prev=ctx->focusv[ctx->focusp];
    ctx->focusp=-1;
    if (prev->type->focus) prev->type->focus(prev,0);
  }
  if ((np>=0)&&(np<ctx->focusc)) {
    ctx->focusp=np;
    if (widget->type->focus) widget->type->focus(widget,1);
  }
  return widget;
}
