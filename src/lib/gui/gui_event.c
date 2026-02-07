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
  fprintf(stderr,"%s %d,%d\n",__func__,w,h);
}

/* Window focus or blur.
 */
 
void gui_cb_focus(int focus) {
  fprintf(stderr,"%s %d\n",__func__,focus);
}

/* Window exposure.
 */
 
void gui_cb_expose(int x,int y,int w,int h) {
  fprintf(stderr,"%s %d,%d,%d,%d\n",__func__,x,y,w,h);
}

/* Raw keystroke. USB-HID page 7.
 */
 
int gui_cb_key(int keycode,int value) {
  fprintf(stderr,"%s 0x%08x=%d\n",__func__,keycode,value);
  return 0; // Nonzero to prevent text.
}

/* Digested keystroke. Unicode.
 */
 
void gui_cb_text(int codepoint) {
  fprintf(stderr,"%s U+%x\n",__func__,codepoint);
}

/* Mouse motion, client coords.
 */
 
void gui_cb_mmotion(int x,int y) {
  fprintf(stderr,"%s %d,%d\n",__func__,x,y);
}

/* Mouse button. 1,2,3 = left,right,center.
 */
 
void gui_cb_mbutton(int btnid,int value) {
  fprintf(stderr,"%s %d=%d\n",__func__,btnid,value);
}

/* Mouse wheel.
 */
 
void gui_cb_mwheel(int dx,int dy) {
  fprintf(stderr,"%s %+d,%+d\n",__func__,dx,dy);
}
