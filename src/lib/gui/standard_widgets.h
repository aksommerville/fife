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

/* textedit: Edit text with scrolling.
 ********************************************************************************/
 
extern const struct widget_type widget_type_textedit;

struct widget_args_textedit {
  int TODO;
};

//TODO label
//TODO button
//TODO field

#endif
