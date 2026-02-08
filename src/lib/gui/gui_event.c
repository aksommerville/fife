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
  //fprintf(stderr,"%s %d,%d\n",__func__,x,y);
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
  //XXX test scrolling.
  struct widget *root=gui_global_context->root;
  if (!root) return;
  root->scrollx+=dx*5;
  root->scrolly+=dy*5;
  fprintf(stderr,"...scroll %d,%d\n",root->scrollx,root->scrolly);
  gui_global_context->render_soon=1;
}
