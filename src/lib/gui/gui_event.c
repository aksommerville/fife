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
    //TODO mark bounds dirty or call pack
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
  //fprintf(stderr,"%s %d,%d,%d,%d\n",__func__,x,y,w,h);
  struct widget *root=gui_global_context->root;
  if (!root) return;
  // Playing it dumb for now, and redraw the whole window on every exposure event.
  int fbw=0,fbh=0,stride=0;
  void *fb=wm_get_framebuffer(&fbw,&fbh,&stride);
  if (!fb) return;
  if ((root->w!=fbw)||(root->h!=fbh)) return; // This shouldn't happen, and I don't know what to make of it.
  struct image image={
    .v=fb,
    .w=fbw,
    .h=fbh,
    .stride=stride,
    .pixelsize=32,
    .writeable=1,
  };
  root->type->render(root,&image);
  wm_framebuffer_dirty(0,0,fbw,fbh);
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
