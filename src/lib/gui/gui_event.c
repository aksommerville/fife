#include "gui_internal.h"

/* Main window closed.
 */
 
void gui_cb_close() {
  fprintf(stderr,"%s\n",__func__);
  gui_global_context->termstatus=0;
  gui_global_context->terminate=1;
}

/* Window resized.
 */
 
void gui_cb_resize(int w,int h) {
  //fprintf(stderr,"%s %d,%d\n",__func__,w,h);
  if ((w<1)||(h<1)) return;
  if ((w==gui_global_context->w)&&(h==gui_global_context->h)) return;
  gui_global_context->w=w;
  gui_global_context->h=h;
  struct widget *root=gui_global_context->root;
  if (root) {
    root->w=w;
    root->h=h;
    widget_pack(root);
  }
  //TODO Should we repack modals somehow? Might not have all the knowledge they had at spawn.
}

/* Window focus or blur.
 */
 
void gui_cb_focus(int focus) {
  fprintf(stderr,"%s %d\n",__func__,focus);
}

/* Window exposure.
 */
 
void gui_cb_expose(int x,int y,int w,int h) {
  if (!gui_global_context) return;
  gui_global_context->render_soon=1;
  // For now at least, we're ignoring the exposed bounds and redrawing everything whenever anything needs it.
  // That sounds heavy-handed but in truth it's probably the right way to go.
}

/* Keyboard event.
 */
 
void gui_cb_key(int keycode,int value,int codepoint) {
  //fprintf(stderr,"%s 0x%08x=%d U+%x\n",__func__,keycode,value,codepoint);
  struct gui_context *ctx=gui_global_context;
  if (!ctx) return;
  
  // We track modifier keys even if focus will consume it.
  switch (keycode) {
    #define _(kc,tag) case kc: if (value) ctx->modifiers|=GUI_MOD_##tag; else ctx->modifiers&=~GUI_MOD_##tag; break;
    _(0x000700e0,LCTL)
    _(0x000700e1,LSHIFT)
    _(0x000700e2,LALT)
    _(0x000700e3,LSUPER)
    _(0x000700e4,RCTL)
    _(0x000700e5,RSHIFT)
    _(0x000700e6,RALT)
    _(0x000700e7,RSUPER)
    #undef _
  }
  
  // First give the focus widget a crack at it, if there is one.
  if ((ctx->focusp>=0)&&(ctx->focusp<ctx->focusc)) {
    struct widget *focus=ctx->focusv[ctx->focusp];
    if (focus->type->key) {
      if (focus->type->key(focus,keycode,value,codepoint)) return;
    }
    if (focus->type->activate&&value&&(keycode==0x0007002c)) { // Space => "activate"
      focus->type->activate(focus);
      return;
    }
  }
  
  /* Not handled by the focussed widget, consider global actions.
   */
  if (value) switch (keycode) {
    case 0x00070029: { // Esc
        if (ctx->modalc>0) gui_remove_modal(ctx,ctx->modalv[ctx->modalc-1]);
      } break;
    case 0x0007002b: { // Tab
        if (ctx->modifiers&GUI_MOD_SHIFT) gui_focus_next(ctx,-1);
        else gui_focus_next(ctx,1);
      } break;
  }
}

/* Find a widget that supports tracking, at the given point.
 */
 
static struct widget *gui_find_track_widget(struct widget *widget,int x,int y) {
  if (!widget_point_in_bounds(widget,x,y)) return 0;
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    struct widget *found=gui_find_track_widget(child,x,y);
    if (found) return found;
  }
  if (widget->proxyto) {
    if (widget->proxyto->parent&&widget->proxyto->clickable) return widget;
  } else {
    if (widget->clickable) return widget;
  }
  return 0;
}

/* Find the innermost widget that supports raw mouse events, at the given point.
 */
 
static struct widget *gui_find_mouse_widget(struct widget *widget,int x,int y) {
  if (!widget_point_in_bounds(widget,x,y)) return 0;
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    struct widget *found=gui_find_mouse_widget(child,x,y);
    if (found) return found;
  }
  if (widget->proxyto) {
    if (widget->proxyto->parent&&widget->proxyto->rawmouse) return widget;
  } else {
    if (widget->rawmouse) return widget;
  }
  return 0;
}

/* Mouse motion, client coords.
 */
 
void gui_cb_mmotion(int x,int y) {
  //fprintf(stderr,"%s %d,%d\n",__func__,x,y);
  struct gui_context *ctx=gui_global_context;
  if (!ctx) return;
  if ((x==ctx->mx)&&(y==ctx->my)) return;
  
  // Tracking?
  if (ctx->track) {
    struct widget *target=ctx->track->proxyto?ctx->track->proxyto:ctx->track;
    if (widget_point_in_bounds(ctx->track,x,y)) {
      if (!ctx->track_in) {
        target->type->track(target,GUI_TRACK_REENTER);
        ctx->track_in=1;
      }
    } else {
      if (ctx->track_in) {
        target->type->track(target,GUI_TRACK_EXIT);
        ctx->track_in=0;
      }
    }
  
  // Raw.
  } else {
    struct widget *root=ctx->root;
    if (ctx->modalc>0) root=ctx->modalv[ctx->modalc-1];
    struct widget *hover=gui_find_mouse_widget(root,ctx->mx,ctx->my);
    while (hover) {
      struct widget *target=hover->proxyto?hover->proxyto:hover;
      if (target->rawmouse&&target->type->mmotion&&target->type->mmotion(target,ctx->mx,ctx->my)) break;
      hover=hover->parent;
    }
  }
  
  ctx->mx=x;
  ctx->my=y;
}

/* Mouse button. 1,2,3 = left,right,center.
 */
 
void gui_cb_mbutton(int btnid,int value) {
  //fprintf(stderr,"%s %d=%d\n",__func__,btnid,value);
  struct gui_context *ctx=gui_global_context;
  if (!ctx) return;
  
  /* If we're tracking something, look for release of btnid 1.
   */
  if (ctx->track&&(btnid==1)) {
    if (value) return; // Huh? I thought it was already pressed.
    struct widget *target=ctx->track->proxyto?ctx->track->proxyto:ctx->track;
    if (ctx->track_in) {
      int ack=target->type->track(target,GUI_TRACK_END_IN);
      if (!ack) {
        if (target->type->activate) {
          target->type->activate(target);
        }
      }
    } else {
      target->type->track(target,GUI_TRACK_END_OUT);
    }
    widget_del(ctx->track);
    ctx->track=0;
    return;
  }
  
  /* Press of btnid 1 can begin a track.
   */
  if ((btnid==1)&&value) {
    struct widget *root=ctx->root;
    if (ctx->modalc>0) root=ctx->modalv[ctx->modalc-1];
    struct widget *track=gui_find_track_widget(root,ctx->mx,ctx->my);
    if (track) {
      struct widget *target=track->proxyto?track->proxyto:track;
      int ack=target->type->track(target,GUI_TRACK_BEGIN);
      if (ack) {
        if (widget_ref(track)<0) return;
        ctx->track=track;
        ctx->track_in=1;
        if (target->focusable) gui_focus_widget(ctx,target);
        return;
      }
    // If a modal is present and the click is outside that modal, dismiss it.
    } else {
      if ((root!=ctx->root)&&(ctx->mx<root->x)||(ctx->my<root->y)||(ctx->mx>=root->x+root->w)||(ctx->my>=root->y+root->h)) {
        gui_remove_modal(ctx,root);
      }
    }
  }
  
  /* Not tracking, send raw mouse events.
   */
  struct widget *root=ctx->root;
  if (ctx->modalc>0) root=ctx->modalv[ctx->modalc-1];
  struct widget *hover=gui_find_mouse_widget(root,ctx->mx,ctx->my);
  while (hover) {
    struct widget *target=hover->proxyto?hover->proxyto:hover;
    if (target->rawmouse&&target->type->mbutton&&target->type->mbutton(target,btnid,value,ctx->mx,ctx->my)) break;
    hover=hover->parent;
  }
}

/* Mouse wheel.
 */
 
void gui_cb_mwheel(int dx,int dy) {
  struct gui_context *ctx=gui_global_context;
  if (!ctx) return;
  struct widget *root=ctx->root;
  if (ctx->modalc>0) root=ctx->modalv[ctx->modalc-1];
  struct widget *hover=gui_find_mouse_widget(root,ctx->mx,ctx->my);
  while (hover) {
    struct widget *target=hover->proxyto?hover->proxyto:hover;
    if (target->rawmouse&&target->type->mwheel&&target->type->mwheel(target,dx,dy,ctx->mx,ctx->my)) break;
    hover=hover->parent;
  }
}
