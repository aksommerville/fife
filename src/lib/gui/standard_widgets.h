/* standard_widgets.h
 * Clients can of course write their own widget types.
 * But there's a lot of generic boilerplate that we can provide.
 */
 
#ifndef STANDARD_WIDGETS_H
#define STANDARD_WIDGETS_H

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

/* button: Icon and label in a box you can focus or click.
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

#endif
