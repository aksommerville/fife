/* gui.h
 * Fife's generic GUI framework.
 * wm below, and the client app above.
 * You should not interact with wm directly; let us do that.
 * Let's also do generic inversion of control. Your main() should defer to gui.
 */
 
#ifndef GUI_H
#define GUI_H

struct gui_context;
struct gui_delegate;
struct widget;
struct widget_type;
struct image;
struct font;

#include "standard_widgets.h"

/* Context.
 * Only one context may be live at a time.
 ********************************************************************/
 
struct gui_delegate {
  void *userdata;
  //TODO
};
 
void gui_context_del(struct gui_context *ctx);

struct gui_context *gui_context_new(const struct gui_delegate *delegate);

/* Fails if we already have a root widget.
 * Returns WEAK on success.
 */
struct widget *gui_context_create_root(
  struct gui_context *ctx,
  const struct widget_type *type,
  const void *args,int argslen
);

/* With your context initialized, call this to run the whole app.
 * Returns proposal for program's exit status.
 */
int gui_main(struct gui_context *ctx);

struct gui_context *gui_get_context();

void gui_terminate_soon(struct gui_context *ctx,int status);

/* The GUI context serves as a font respository, since they have to go somewhere.
 * These are WEAK, and they will live as long as the context.
 * If you load a named font first, it becomes the default.
 */
struct font *gui_get_default_font(struct gui_context *ctx);
struct font *gui_get_named_font(struct gui_context *ctx,const char *name,int namec);

/* Generic widget.
 *******************************************************************/
 
struct widget_type {
  const char *name;
  int objlen; // >=sizeof(struct widget)
  void (*del)(struct widget *widget);
  int (*init)(struct widget *widget,const void *args,int argslen);
  
  /* (dst) is typically a slice of the framebuffer, how we effect clipping.
   * (dst) is already set to (widget)'s bounds, so render yourself at (0,0) in it.
   * REQUIRED.
   */
  void (*render)(struct widget *widget,struct image *dst);
  
  /* Fill (w,h) with your preferred size.
   * Caller must prepopulate (w,h), and receiver may leave them untouched to accept that.
   * Caller estimates the available space as (maxw,maxh).
   * In any case, parents establish their children's bounds, and the children must accept it.
   */
  void (*measure)(int *w,int *h,struct widget *widget,int maxw,int maxh);
  
  /* Notification that bounds have changed.
   * Pack your children and do whatever other layout work needs done.
   */
  void (*pack)(struct widget *widget);
  
  //TODO events
};
 
struct widget {
  const struct widget_type *type;
  struct gui_context *ctx; // REQUIRED.
  int refc;
  struct widget *parent; // WEAK. Null for the root.
  struct widget **childv; // STRONG.
  int childc,childa;
  int x,y,w,h; // (x,y) relative to parent.
  int scrollx,scrolly; // My content is offset by so much. These are typically positive if not zero.
  int padx,pady; // Interior padding. Generic pack and measure will use it. If you do those yourself, it's up to you.
};

void widget_del(struct widget *widget);
int widget_ref(struct widget *widget);

/* Create a widget and return a STRONG reference.
 * Caller must pack the parent widget at some point after.
 */
struct widget *widget_new(
  struct gui_context *ctx,
  const struct widget_type *type,
  const void *args,int argslen
);

/* Create a widget, append to some parent, and return a WEAK reference.
 */
struct widget *widget_spawn(
  struct widget *parent,
  const struct widget_type *type,
  const void *args,int argslen
);

/* Child list access.
 * A widget with a parent will fail to add to any other parent.
 * To change children's order, you can re-add a widget to the same parent.
 * In that case (p) refers to the current list, so the child's current index and also that plus one, refer to the same position.
 * (p<0) when inserting to append. Or >childc.
 * When removing, beware that (parent) very likely holds the last reference to (child). Retain child first if you need to.
 * widget_is_ancestor, the edge case where they're the same widget, the answer is YES.
 */
int widget_childv_insert(struct widget *parent,int p,struct widget *child);
int widget_childv_remove(struct widget *parent,struct widget *child);
int widget_childv_remove_at(struct widget *parent,int p);
int widget_childv_search(const struct widget *parent,const struct widget *child);
int widget_is_ancestor(const struct widget *ancestor,const struct widget *descendant);

/* The ancestor widget with no parent, possibly (widget) itself.
 * This is not necessarily what the context thinks is the root.
 */
struct widget *widget_get_root(struct widget *widget);

/* Rewrite (x,y) in place to convert between (widget)'s origin and the global origin.
 */
void widget_coords_global_from_local(int *x,int *y,const struct widget *widget);
void widget_coords_local_from_global(int *x,int *y,const struct widget *widget);

/* Examine all ancestor boundaries and return the bounds in global space that this widget clips to.
 */
void widget_get_clip(int *x,int *y,int *w,int *h,const struct widget *widget);

/* Render all my child widgets into (dst), which must have (widget)'s bounds.
 * ie this takes exactly the same arguments as the render hook.
 */
void widget_render_children(struct widget *widget,struct image *dst);

/* Hook wrappers, possibly with fallback logic if the hook is not implemented.
 */
void widget_measure(int *w,int *h,struct widget *widget,int maxw,int maxh);
void widget_pack(struct widget *widget);

#endif
