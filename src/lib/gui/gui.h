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

#include <stdint.h>
#include "standard_widgets.h"

/* Context.
 * Only one context may be live at a time.
 ********************************************************************/
 
struct gui_delegate {
  void *userdata;
  double update_rate; // hz, only relevant if you use gui_main.
  int log_clock_at_quit; // 1 to show counters and CPU consumption on normal exits. >1 to log on abnormal exits too.
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

/* If you're not using gui_main(), call this often.
 * Does not block for timing, that's your concern.
 */
int gui_update(struct gui_context *ctx,double elapsed);

/* Change or query keyboard focus.
 * These return a WEAK reference to the newly-focussed widget on success.
 * Exception: gui_focus_widget() with something non-null but invalid, we return null, not the current focus.
 * next(0) to get the focussed widget without changing anything.
 * next(-1) or (1) to walk the focus ring.
 * next(-2) or (2) to focus the first or last widget in the ring.
 * gui_focus_widget(null) to make nothing focussed, which is a perfectly legal state.
 */
struct widget *gui_focus_next(struct gui_context *ctx,int d);
struct widget *gui_focus_widget(struct gui_context *ctx,struct widget *widget);

/* Generic widget.
 *******************************************************************/
 
struct widget_type {
  const char *name;
  int objlen; // >=sizeof(struct widget)
  void (*del)(struct widget *widget);
  int (*init)(struct widget *widget,const void *args,int argslen);
  
  /* (dst) is typically a slice of the framebuffer, how we effect clipping.
   * (dst) is already set to (widget)'s bounds, so render yourself at (0,0) in it.
   * Your children will automatically have your scroll applied, but you yourself will not.
   * Subtract (scrollx,scrolly) if you want to render something scrolled.
   */
  void (*render)(struct widget *widget,struct image *dst);
  int autorender; // If nonzero, wrapper will automatically render your background color and children.
  
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
  
  /* Notifiction that you've gained (1) or lost (0) keyboard focus.
   */
  void (*focus)(struct widget *widget,int focus);
  
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
  uint32_t bgcolor; // If nonzero, fill my background with this. (individual render hooks are expected to, and the no-hook default will).
  int focusable; // Nonzero to accept keyboard focus. Set (ctx->tree_changed) if you change.
  uint32_t parentuse; // Private field for widget's parent's use only. eg for layout bookkeeping. Resets to zero when reparenting.
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
 * It's better to set (type->autorender) and let the wrapper take care of it.
 * But you might want to render something on top of the kids, in which case you'll need this.
 */
void widget_render_children(struct widget *widget,struct image *dst);

/* Hook wrappers, possibly with fallback logic if the hook is not implemented.
 */
void widget_render(struct widget *widget,struct image *dst);
void widget_measure(int *w,int *h,struct widget *widget,int maxw,int maxh);
void widget_pack(struct widget *widget);

/* Timing regulator, used by gui_main().
 *************************************************************************/
 
struct gui_clock {
  double period; // Desired update period in seconds. Typically 1/60.
  double starttime_real;
  double starttime_cpu;
  double prevtime;
  double nexttime;
  int framec;
  int panicc;
};

// Read the realtime and cpu clocks.
double gui_now_real();
double gui_now_cpu();
void gui_sleep(double s);

/* Prepare a clock that will tick at the given rate.
 * If we don't like (rate_hz), we'll make something up.
 */
void gui_clock_init(struct gui_clock *clock,double rate_hz);

/* Examine current and recent time.
 * If it's too soon, we'll sleep into the next period.
 * Returns time elapsed since the last tick, or a sensible lie on the first tick.
 */
double gui_clock_tick(struct gui_clock *clock);

/* Print frame count and CPU consumption to stderr, if we have sufficient data.
 * This is something I always do in games, where performance matters. For these general GUI apps, not so important.
 */
void gui_clock_report(struct gui_clock *clock);

#endif
