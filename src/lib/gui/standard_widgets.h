/* standard_widgets.h
 * Clients can of course write their own widget types.
 * But there's a lot of generic boilerplate that we can provide.
 */
 
#ifndef STANDARD_WIDGETS_H
#define STANDARD_WIDGETS_H

/* dummy: No extra behavior. Separator uses it as a placeholder.
 *******************************************************************************/
 
extern const struct widget_type widget_type_dummy;

/* dashboard: Menu bar and tabbed panels, recommended for gui's root.
 ********************************************************************************/
 
extern const struct widget_type widget_type_dashboard;

struct widget_args_dashboard {
  int use_menubar;
  int use_left_panel;
  int use_bottom_panel;
  int use_right_panel;
  int use_tabs;
  void (*cb_menu)(struct widget *widget,void *userdata);
  void *userdata;
};

/* Main panel is a tabber initially if you requested that. Otherwise it's null.
 */
struct widget *widget_dashboard_get_menubar(const struct widget *widget);
struct widget *widget_dashboard_get_left_panel(const struct widget *widget);
struct widget *widget_dashboard_get_bottom_panel(const struct widget *widget);
struct widget *widget_dashboard_get_right_panel(const struct widget *widget);
struct widget *widget_dashboard_get_main_panel(const struct widget *widget);

/* The panels you requested at init are null until you spawn into them.
 * This will first delete anything existing in that slot.
 * If you requested tabs, spawning main creates a new tab.
 */
struct widget *widget_dashboard_spawn_left_panel(struct widget *widget,const struct widget_type *type,const void *args,int argslen);
struct widget *widget_dashboard_spawn_bottom_panel(struct widget *widget,const struct widget_type *type,const void *args,int argslen);
struct widget *widget_dashboard_spawn_right_panel(struct widget *widget,const struct widget_type *type,const void *args,int argslen);
struct widget *widget_dashboard_spawn_main_panel(struct widget *widget,const struct widget_type *type,const void *args,int argslen);

/* menubar: Popup menus arranged horizontally.
 *********************************************************************************/
 
extern const struct widget_type widget_type_menubar;

struct widget_args_menubar {
  struct font *font;
  void (*cb)(struct widget *widget,void *userdata); // (widget) is not this menubar; it's whatever was selected.
  void *userdata;
};

struct widget *widget_menubar_spawn_menu(struct widget *widget,const char *label,int labelc);

/* menu: Popup menu. It's a button at rest, and we produce the modal menu when clicked.
 ********************************************************************************/
 
extern const struct widget_type widget_type_menu;

struct widget_args_menu {
  struct font *font;
  const char *text;
  int textc;
  void (*cb)(struct widget *widget,void *userdata);
  void *userdata;
};

struct widget *widget_menu_spawn_option(struct widget *widget,const char *label,int labelc);

/* separator: Two panels with an interactive divider between them. Horizontal or vertical.
 ********************************************************************************/
 
extern const struct widget_type widget_type_separator;

struct widget_args_separator {
  char orient; // 'x' or 'y'
  int size; // Initial width or height of the left or top half, in pixels. Ignored if zero.
  int pct; // (size) but a percentage. Ignored if zero.
};

/* Null if it's a placeholder.
 */
struct widget *widget_separator_get_panel(const struct widget *widget,int side);

/* (side) is 0 or 1.
 * If we already had a panel in that slot, it is gracefully destroyed.
 * As a convenience, we defer to regular widget_spawn() if (widget) is not a separator.
 * (type) may be null to remove the current panel and spawn a new placeholder, which will be returned (unlike widget_separator_get_panel).
 * If you spawn a dummy panel, that's fine, but we won't be able to distinguish it from our placeholder, so you won't get it back from get_panel().
 */
struct widget *widget_separator_spawn_panel(struct widget *widget,int side,const struct widget_type *type,const void *args,int argslen);

/* tabber: Row of tabs, with a big content panel below.
 ***********************************************************************************/
 
extern const struct widget_type widget_type_tabber;

struct widget_args_tabber {
  struct font *font;
};

// Returns the panel. If you want its tab, see below.
struct widget *widget_tabber_spawn(
  struct widget *widget,
  const char *label,int labelc,
  const struct widget_type *type,
  const void *args,int argslen
);

int widget_tabber_count_tabs(const struct widget *widget);
int widget_tabber_find_tab(const struct widget *widget,const struct widget *panel);
struct widget *widget_tabber_get_tab(const struct widget *widget,int p);
struct widget *widget_tabber_get_panel(const struct widget *widget,int p);
struct widget *widget_tabber_get_tab_for_panel(const struct widget *widget,const struct widget *panel);
struct widget *widget_tabber_get_panel_for_tab(const struct widget *widget,const struct widget *tab);

/* Do not add or remove children directly on a tabber.
 */
int widget_tabber_remove_tab(struct widget *widget,int p);

int widget_tabber_focus_tab(struct widget *widget,int p);

/* packer: Generic container that arranges its children 1-dimensionally.
 *******************************************************************************/
 
extern const struct widget_type widget_type_packer;

struct widget_args_packer {
  char orientation; // 'x' to place children in a row horizontally, or 'y' to place them in a column vertically.
  int reverse; // Left-to-right or top-to-bottom by default; nonzero here to reverse it.
  int majoralign; // (-2,-1,0,1) = fill, left/top, center, right/bottom
  int minoralign; // Alignments are not affected by (reverse).
  int spacing; // Pixels between children along the major axis.
};

/* Mark a specific child as accepting (>0) or rejecting (<0) excess space, when using (majoralign==-2).
 * If we have any child with positive flex, only those positives will grow.
 * If they're all zero or negative, the zeroes will grow.
 * If they're all negative, you shouldn't be using majoralign -2.
 * When we need to shrink, all children get shrunk equally.
 */
int widget_packer_flex_child(struct widget *widget,struct widget *child,int flex);

/* button: Label in a box you can focus or click.
 *****************************************************************************/
 
extern const struct widget_type widget_type_button;

struct widget_args_button {
  struct font *font;
  const char *text;
  int textc;
  void (*cb)(struct widget *widget,void *userdata);
  void *userdata;
};

int widget_button_set_font(struct widget *widget,struct font *font);
int widget_button_set_text(struct widget *widget,const char *src,int srcc);
int widget_button_set_callback(struct widget *widget,void (*cb)(struct widget *widget,void *userdata),void *userdata);

/* label: Static text.
 ****************************************************************************/
 
extern const struct widget_type widget_type_label;
 
struct widget_args_label {
  struct font *font;
  const char *text;
  int textc;
  uint32_t fgcolor;
};

int widget_label_set_text(struct widget *widget,const char *src,int srcc);
int widget_label_set_font(struct widget *widget,struct font *font);
int widget_label_set_fgcolor(struct widget *widget,uint32_t fgcolor);

/* field: Mutable text, single line.
 **************************************************************************/
 
extern const struct widget_type widget_type_field;

struct widget_args_field {
  struct font *font;
  const char *text;
  int textc;
  void *userdata;
  
  // Called before making a change. Return nonzero to reject the change; we'll try to return the same.
  // Mind that any changes you make to the widget's text invalidate (before).
  int (*cb_preedit)(struct widget *widget,const char *before,int beforec,int p,int c,const char *incoming,int incomingc);
  
  // Called after every change. (text) is the entire text and (editp) is the insertion point after the change.
  void (*cb_postedit)(struct widget *widget,const char *text,int textc,int editp);
};

int widget_field_get_text(void *dstpp,const struct widget *widget); // Text is weak and volatile. Copy immediately if you need to keep it. Never returns <0.
int widget_field_set_text(struct widget *widget,const char *src,int srcc);
int widget_field_handoff_text(struct widget *widget,char *src,int srcc); // On success, we take ownership of (src) and will eventually free it.
int widget_field_set_font(struct widget *widget,struct font *font);
void *widget_field_get_userdata(const struct widget *widget);
int widget_field_set_userdata(struct widget *widget,void *userdata);
int widget_field_get_selection(int *p,const struct widget *widget); // => c
int widget_field_set_selection(struct widget *widget,int p,int c); // (c<0) to extend to the end, ie (0,-1) is "select all"
int widget_field_select_word(struct widget *widget,int p); // Sets selection to the word surrounding (p), by our own definition of "word".

/* Text entry operations.
 * Our key event hook calls these, and you can too.
 */
int widget_field_move_cursor(struct widget *widget,int dx,int dy);
int widget_field_delete(struct widget *widget);
int widget_field_backspace(struct widget *widget);
int widget_field_insert_codepoint(struct widget *widget,int codepoint);

/* textedit: Edit text with scrolling.
 ********************************************************************************/
 
extern const struct widget_type widget_type_textedit;

struct widget_args_textedit {
  int TODO;
};

/* checkbox: Button with togglable state.
 * Should this include a text label? For now I'm sayng no.
 ************************************************************************/
 
extern const struct widget_type widget_type_checkbox;

struct widget_args_checkbox {
  void *userdata;
  void (*cb)(struct widget *widget,int value,void *userdata);
  int enable; // <--- Must be nonzero if you're providing args, if you want it enabled. No args, enabled by default.
  int value;
};

void *widget_checkbox_get_userdata(const struct widget *widget);
int widget_checkbox_set_userdata(struct widget *widget,void *userdata);
int widget_checkbox_get_enable(const struct widget *widget);
int widget_checkbox_set_enable(struct widget *widget,int enable);
int widget_checkbox_get_value(const struct widget *widget); // 0,1. If you set to some other integer, it becomes 1.
int widget_checkbox_set_value(struct widget *widget,int value);

#endif
