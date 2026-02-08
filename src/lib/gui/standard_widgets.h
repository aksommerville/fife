/* standard_widgets.h
 * Clients can of course write their own widget types.
 * But there's a lot of generic boilerplate that we can provide.
 */
 
#ifndef STANDARD_WIDGETS_H
#define STANDARD_WIDGETS_H

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
