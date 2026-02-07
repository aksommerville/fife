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
struct gui_widget;
struct gui_widget_type;

/* Context.
 * Only one context may be live at a time.
 ********************************************************************/
 
struct gui_delegate {
  void *userdata;
  //TODO
};
 
void gui_context_del(struct gui_context *ctx);

struct gui_context *gui_context_new(const struct gui_delegate *delegate);

/* With your context initialized, call this to run the whole app.
 * Returns proposal for program's exit status.
 */
int gui_main(struct gui_context *ctx);

struct gui_context *gui_get_context();

void gui_terminate_soon(struct gui_context *ctx,int status);

#endif
