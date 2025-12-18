/* wm_x11_public.c
 * The generic "wm" interface except:
 *  - wm_update() lives in wm_x11_update.c
 *  - framebuffer and pixel functions in wm_x11_video.c
 */

#include "wm_x11_internal.h"

struct wm_x11 wm_x11={0};

/* Cleanup bits.
 */
 
static void wm_x11_cursor_cleanup(struct wm_x11_cursor *cursor) {
  if (cursor->rgba) free(cursor->rgba);
}

/* Quit.
 */
 
void wm_quit() {
  if (wm_x11.init) {
    if (wm_x11.cursorv) {
      while (wm_x11.cursorc-->0) wm_x11_cursor_cleanup(wm_x11.cursorv+wm_x11.cursorc);
      free(wm_x11.cursorv);
    }
    if (wm_x11.fb) XDestroyImage(wm_x11.fb);
    if (wm_x11.dpy) XCloseDisplay(wm_x11.dpy);
  }
  memset(&wm_x11,0,sizeof(wm_x11));
}

/* Init, private.
 */
 
static int wm_x11_init() {
  if (!(wm_x11.dpy=XOpenDisplay(0))) return -1;
  wm_x11.screen=DefaultScreen(wm_x11.dpy);
  
  #define GETATOM(tag) wm_x11.atom_##tag=XInternAtom(wm_x11.dpy,#tag,0);
  GETATOM(WM_PROTOCOLS)
  GETATOM(WM_DELETE_WINDOW)
  GETATOM(_NET_WM_STATE)
  GETATOM(_NET_WM_STATE_FULLSCREEN)
  GETATOM(_NET_WM_STATE_ADD)
  GETATOM(_NET_WM_STATE_REMOVE)
  GETATOM(_NET_WM_ICON)
  GETATOM(_NET_WM_ICON_NAME)
  GETATOM(_NET_WM_NAME)
  GETATOM(STRING)
  GETATOM(UTF8_STRING)
  GETATOM(WM_CLASS)
  #undef GETATOM
  
  wm_x11.w=640;//TODO Default window size.
  wm_x11.h=360;
  
  XSetWindowAttributes wattr={
    .event_mask=
      StructureNotifyMask|
      KeyPressMask|KeyReleaseMask|
      PointerMotionMask|ButtonPressMask|ButtonReleaseMask|
      EnterWindowMask|LeaveWindowMask|
      FocusChangeMask|ExposureMask|
      //TODO Is it possible to request motion events when outside our window? We'd rather get all of them.
    0,
  };
  if (!(wm_x11.win=XCreateWindow(
    wm_x11.dpy,RootWindow(wm_x11.dpy,wm_x11.screen),
    0,0,wm_x11.w,wm_x11.h,0,
    DefaultDepth(wm_x11.dpy,wm_x11.screen),InputOutput,CopyFromParent,
    CWBorderPixel|CWEventMask,&wattr
  ))) return -1;
  if (!(wm_x11.gc=XCreateGC(wm_x11.dpy,wm_x11.win,0,0))) return -1;
  
  XMapWindow(wm_x11.dpy,wm_x11.win);
  XSync(wm_x11.dpy,0);
  XSetWMProtocols(wm_x11.dpy,wm_x11.win,&wm_x11.atom_WM_DELETE_WINDOW,1);
  
  return 0;
}

/* Init, public.
 */
 
int wm_init(const struct wm_delegate *delegate) {
  if (wm_x11.init) return -1;
  wm_x11.init=1;
  wm_x11.delegate=*delegate;
  if (wm_x11_init()<0) {
    wm_quit();
    return -1;
  }
  return 0;
}

/* Set title.
 */

void wm_set_title(const char *src,int srcc) {
  if (!wm_x11.init) return;
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  if (srcc>WM_X11_TITLE_LIMIT) { // If too long, truncate, then back up to a UTF-8 boundary.
    srcc=WM_X11_TITLE_LIMIT;
    while (srcc&&((src[srcc]&0xc0)==0x80)) srcc--;
  }
  
  // I've seen these properties in GNOME 2, unclear whether they might still be in play:
  XTextProperty prop={.value=(void*)src,.encoding=wm_x11.atom_STRING,.format=8,.nitems=srcc};
  XSetWMName(wm_x11.dpy,wm_x11.win,&prop);
  XSetWMIconName(wm_x11.dpy,wm_x11.win,&prop);
  XSetTextProperty(wm_x11.dpy,wm_x11.win,&prop,wm_x11.atom__NET_WM_ICON_NAME);
    
  // This one becomes the window title and bottom-bar label, in GNOME 3:
  prop.encoding=wm_x11.atom_UTF8_STRING;
  XSetTextProperty(wm_x11.dpy,wm_x11.win,&prop,wm_x11.atom__NET_WM_NAME);
    
  // This daffy bullshit becomes the Alt-Tab text in GNOME 3:
  {
    char tmp[256];
    int len=prop.nitems+1+prop.nitems;
    if (len<sizeof(tmp)) {
      memcpy(tmp,prop.value,prop.nitems);
      tmp[prop.nitems]=0;
      memcpy(tmp+prop.nitems+1,prop.value,prop.nitems);
      tmp[prop.nitems+1+prop.nitems]=0;
      prop.value=tmp;
      prop.nitems=prop.nitems+1+prop.nitems;
      prop.encoding=wm_x11.atom_STRING;
      XSetTextProperty(wm_x11.dpy,wm_x11.win,&prop,wm_x11.atom_WM_CLASS);
    }
  }
}

/* Set icon.
 */
 
static void wm_x11_copy_icon_pixels(long *dst,const uint8_t *src,int c) {
  for (;c-->0;dst++,src+=4) {
    *dst=(src[3]<<24)|(src[0]<<16)|(src[1]<<8)|src[2];
  }
}
 
void wm_set_icon(const void *rgba,int w,int h) {
  if (!wm_x11.init) return;
  if (!rgba||(w<1)||(h<1)||(w>WM_X11_ICON_LIMIT)||(h>WM_X11_ICON_LIMIT)) return;
  int length=2+w*h;
  long *pixels=malloc(sizeof(long)*length);
  if (!pixels) return;
  pixels[0]=w;
  pixels[1]=h;
  wm_x11_copy_icon_pixels(pixels+2,rgba,w*h);
  XChangeProperty(wm_x11.dpy,wm_x11.win,wm_x11.atom__NET_WM_ICON,XA_CARDINAL,32,PropModeReplace,(unsigned char*)pixels,length);
  free(pixels);
}

/* Define cursor.
 */
 
static int wm_x11_unused_cursorid() {
  if (!wm_x11.cursorc) return 1;
  return wm_x11.cursorv[wm_x11.cursorc-1].cursorid+1;
}
 
int wm_define_cursor(const void *rgba,int w,int h) {
  if (!wm_x11.init) return -1;
  if (!rgba||(w<1)||(h>1)||(w>WM_X11_CURSOR_LIMIT)||(h>WM_X11_CURSOR_LIMIT)) return -1;
  if (wm_x11.cursorc>=wm_x11.cursora) {
    int na=wm_x11.cursora+4;
    if (na>INT_MAX/sizeof(struct wm_x11_cursor)) return -1;
    void *nv=realloc(wm_x11.cursorv,sizeof(struct wm_x11_cursor)*na);
    if (!nv) return -1;
    wm_x11.cursorv=nv;
    wm_x11.cursora=na;
  }
  int cursorid=wm_x11_unused_cursorid();
  if (cursorid<1) return -1; // eg overflow of cursorid, extremely unlikely and failing is probably wise.
  struct wm_x11_cursor *cursor=wm_x11.cursorv+wm_x11.cursorc++;
  memset(cursor,0,sizeof(struct wm_x11_cursor));
  cursor->cursorid=cursorid;
  cursor->w=w;
  cursor->h=h;
  int stride=w<<2;
  int len=stride*h;
  if (!(cursor->rgba=malloc(len))) {
    wm_x11.cursorc--;
    return -1;
  }
  memcpy(cursor->rgba,rgba,len);
  return cursorid;
  /*TODO Something along these lines...
    XColor color;
    Pixmap pixmap=XCreateBitmapFromData(x11fb->dpy,x11fb->win,"\0\0\0\0\0\0\0\0",1,1);
    Cursor cursor=XCreatePixmapCursor(x11fb->dpy,pixmap,pixmap,&color,&color,0,0);
    XDefineCursor(x11fb->dpy,x11fb->win,cursor);
    XFreeCursor(x11fb->dpy,cursor);
    XFreePixmap(x11fb->dpy,pixmap);
  /**/
}

/* Set cursor.
 */
 
void wm_set_cursor(int cursorid) {
  if (!wm_x11.init) return;
  const struct wm_x11_cursor *cursor=0;
  int lo=0,hi=wm_x11.cursorc;
  while (lo<hi) {
    int ck=(lo+hi)>>1;
    const struct wm_x11_cursor *q=wm_x11.cursorv+ck;
         if (cursorid<q->cursorid) hi=ck;
    else if (cursorid>q->cursorid) lo=ck+1;
    else {
      cursor=q;
      break;
    }
  }
  if (!cursor) return;
  //TODO
}

/* Get size.
 */

void wm_get_size(int *w,int *h) {
  if (!wm_x11.init) return;
  *w=wm_x11.w;
  *h=wm_x11.h;
}
