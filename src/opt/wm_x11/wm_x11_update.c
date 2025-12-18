#include "wm_x11_internal.h"

/* Key press, release, or repeat.
 */
 
static int wm_x11_evt_key(XKeyEvent *evt,int value) {

  /* Pass the raw keystroke. */
  if (wm_x11.delegate.cb_key) {
    KeySym keysym=XkbKeycodeToKeysym(wm_x11.dpy,evt->keycode,0,0);
    if (keysym) {
      int keycode=wm_x11_usb_usage_from_keysym((int)keysym);
      if (keycode) {
        int err=wm_x11.delegate.cb_key(keycode,value);
        if (err) return err; // Stop here if acknowledged.
      }
    }
  }
  
  /* Pass text if press or repeat, and text can be acquired. */
  if (wm_x11.delegate.cb_text) {
    int shift=(evt->state&ShiftMask)?1:0;
    KeySym tkeysym=XkbKeycodeToKeysym(wm_x11.dpy,evt->keycode,0,shift);
    if (shift&&!tkeysym) { // If pressing shift makes this key "not a key anymore", fuck that and pretend shift is off
      tkeysym=XkbKeycodeToKeysym(wm_x11.dpy,evt->keycode,0,0);
    }
    if (tkeysym) {
      int codepoint=wm_x11_codepoint_from_keysym(tkeysym);
      if (codepoint && (evt->type == KeyPress || evt->type == KeyRepeat)) {
        wm_x11.delegate.cb_text(codepoint);
      }
    }
  }
  
  return 0;
}

/* Mouse events.
 */
 
static int wm_x11_evt_mbtn(XButtonEvent *evt,int value) {
  
  // I swear X11 used to automatically report the wheel as (6,7) while shift held, and (4,5) otherwise.
  // After switching to GNOME 3, seems it is only ever (4,5).
  #define SHIFTABLE(v) (evt->state&ShiftMask)?v:0,(evt->state&ShiftMask)?0:v
  
  switch (evt->button) {
    case 1: if (wm_x11.delegate.cb_mbutton) wm_x11.delegate.cb_mbutton(1,value); break;
    case 2: if (wm_x11.delegate.cb_mbutton) wm_x11.delegate.cb_mbutton(3,value); break;
    case 3: if (wm_x11.delegate.cb_mbutton) wm_x11.delegate.cb_mbutton(2,value); break;
    case 4: if (value&&wm_x11.delegate.cb_mwheel) wm_x11.delegate.cb_mwheel(SHIFTABLE(-1)); break;
    case 5: if (value&&wm_x11.delegate.cb_mwheel) wm_x11.delegate.cb_mwheel(SHIFTABLE(1)); break;
    case 6: if (value&&wm_x11.delegate.cb_mwheel) wm_x11.delegate.cb_mwheel(-1,0); break;
    case 7: if (value&&wm_x11.delegate.cb_mwheel) wm_x11.delegate.cb_mwheel(1,0); break;
  }
  #undef SHIFTABLE
  return 0;
}

static int wm_x11_evt_mmotion(XMotionEvent *evt) {
  if (wm_x11.delegate.cb_mmotion) {
    wm_x11.delegate.cb_mmotion(evt->x,evt->y);
  }
  return 0;
}

static int wm_x11_evt_mcrossing(XCrossingEvent *evt) {
  if (wm_x11.delegate.cb_mmotion) {
    wm_x11.delegate.cb_mmotion(evt->x,evt->y);
  }
  return 0;
}

/* Client message.
 */
 
static int wm_x11_evt_client(XClientMessageEvent *evt) {
  if (evt->message_type==wm_x11.atom_WM_PROTOCOLS) {
    if (evt->format==32) {
      if (evt->data.l[0]==wm_x11.atom_WM_DELETE_WINDOW) {
        if (wm_x11.delegate.cb_close) {
          wm_x11.delegate.cb_close();
        }
      }
    }
  }
  return 0;
}

/* Configuration event (eg resize).
 */
 
static int wm_x11_evt_configure(XConfigureEvent *evt) {
  int nw=evt->width,nh=evt->height;
  if ((nw!=wm_x11.w)||(nh!=wm_x11.h)) {
    wm_x11.w=nw;
    wm_x11.h=nh;
    if (wm_x11.delegate.cb_resize) {
      wm_x11.delegate.cb_resize(nw,nh);
    }
  }
  return 0;
}

/* Focus.
 */
 
static int wm_x11_evt_focus(XFocusInEvent *evt,int value) {
  if (value==wm_x11.focus) return 0;
  wm_x11.focus=value;
  if (wm_x11.delegate.cb_focus) {
    wm_x11.delegate.cb_focus(value);
  }
  return 0;
}

/* Exposure.
 */
 
static int wm_x11_evt_expose(XExposeEvent *evt) {
  //fprintf(stderr,"%s %d,%d,%d,%d +%d\n",__func__,evt->x,evt->y,evt->width,evt->height,evt->count);
  if (wm_x11.delegate.cb_expose) {
    wm_x11.delegate.cb_expose(evt->x,evt->y,evt->width,evt->height);
  }
  return 0;
}

/* Process one event.
 */
 
static int wm_x11_receive_event(XEvent *evt) {
  if (!evt) return -1;
  switch (evt->type) {
  
    case KeyPress: return wm_x11_evt_key(&evt->xkey,1);
    case KeyRelease: return wm_x11_evt_key(&evt->xkey,0);
    case KeyRepeat: return wm_x11_evt_key(&evt->xkey,2);
    
    case ButtonPress: return wm_x11_evt_mbtn(&evt->xbutton,1);
    case ButtonRelease: return wm_x11_evt_mbtn(&evt->xbutton,0);
    case MotionNotify: return wm_x11_evt_mmotion(&evt->xmotion);
    case EnterNotify: return wm_x11_evt_mcrossing(&evt->xcrossing);
    case LeaveNotify: return wm_x11_evt_mcrossing(&evt->xcrossing);
    
    case ClientMessage: return wm_x11_evt_client(&evt->xclient);
    
    case ConfigureNotify: return wm_x11_evt_configure(&evt->xconfigure);
    
    case FocusIn: return wm_x11_evt_focus(&evt->xfocus,1);
    case FocusOut: return wm_x11_evt_focus(&evt->xfocus,0);
    
    case Expose: return wm_x11_evt_expose(&evt->xexpose);
    
    //default: fprintf(stderr,"X11 event type %d\n",evt->type);
  }
  return 0;
}

/* Update.
 */
 
int wm_update() {
  if (!wm_x11.init) return -1;
  int evtc=XEventsQueued(wm_x11.dpy,QueuedAfterFlush);
  while (evtc-->0) {
    XEvent evt={0};
    XNextEvent(wm_x11.dpy,&evt);
    if ((evtc>0)&&(evt.type==KeyRelease)) {
      XEvent next={0};
      XNextEvent(wm_x11.dpy,&next);
      evtc--;
      if ((next.type==KeyPress)&&(evt.xkey.keycode==next.xkey.keycode)&&(evt.xkey.time>=next.xkey.time-WM_X11_KEY_REPEAT_INTERVAL)) {
        evt.type=KeyRepeat;
        if (wm_x11_receive_event(&evt)<0) return -1;
      } else {
        if (wm_x11_receive_event(&evt)<0) return -1;
        if (wm_x11_receive_event(&next)<0) return -1;
      }
    } else {
      if (wm_x11_receive_event(&evt)<0) return -1;
    }
  }
  return 0;
}
