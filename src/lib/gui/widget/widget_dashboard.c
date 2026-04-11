#include "lib/gui/gui_internal.h"

/* We abuse (parentuse) among our descendant separators, to identify the pieces.
 * Separator doesn't use it, so it's ok.
 */
#define PARENTUSE_MENUBAR 0xdab00001
#define PARENTUSE_LEFT    0xdab00002
#define PARENTUSE_BOTTOM  0xdab00003
#define PARENTUSE_RIGHT   0xdab00004
#define PARENTUSE_TABBER  0xdab00005
#define PARENTUSE_MAIN    0xdab00006 /* Only used if no tabber. */

struct widget_dashboard {
  struct widget hdr;
  struct widget_args_dashboard args;
};

#define WIDGET ((struct widget_dashboard*)widget)

/* Cleanup.
 */
 
static void _dashboard_del(struct widget *widget) {
}

/* Init.
 */
 
static int _dashboard_init(struct widget *widget,const void *args,int argslen) {
  if (args&&(argslen==sizeof(struct widget_args_dashboard))) {
    const struct widget_args_dashboard *ARGS=args;
    WIDGET->args=*ARGS;
  }
  
  if (WIDGET->args.use_menubar) {
    struct widget_args_menubar menubar_args={
      .cb=WIDGET->args.cb_menu,
      .userdata=WIDGET->args.userdata,
    };
    struct widget *menubar=widget_spawn(widget,&widget_type_menubar,&menubar_args,sizeof(menubar_args));
    if (!menubar) return -1;
    menubar->parentuse=PARENTUSE_MENUBAR;
  }
  
  /* The sidebars have a strict and opinionated order: Left, Bottom, Right.
   * ie the Left panel occupies the entire height below the menubar, and the Right panel extends down only to the Bottom panel.
   */
  struct widget *parent=widget;
  int childp=0; // Points to the half of (parent) that we should descend into. Irrelevant if not a separator.
  if (WIDGET->args.use_left_panel) {
    struct widget_args_separator separator_args={
      .orient='x',
      .pct=20,
    };
    struct widget *separator=widget_separator_spawn_panel(parent,childp,&widget_type_separator,&separator_args,sizeof(separator_args));
    if (!separator) return -1;
    separator->parentuse=PARENTUSE_LEFT;
    parent=separator;
    childp=1;
  }
  if (WIDGET->args.use_bottom_panel) {
    struct widget_args_separator separator_args={
      .orient='y',
      .pct=80,
    };
    struct widget *separator=widget_separator_spawn_panel(parent,childp,&widget_type_separator,&separator_args,sizeof(separator_args));
    if (!separator) return -1;
    separator->parentuse=PARENTUSE_BOTTOM;
    parent=separator;
    childp=0;
  }
  if (WIDGET->args.use_right_panel) {
    struct widget_args_separator separator_args={
      .orient='x',
      .pct=80,
    };
    struct widget *separator=widget_separator_spawn_panel(parent,childp,&widget_type_separator,&separator_args,sizeof(separator_args));
    if (!separator) return -1;
    separator->parentuse=PARENTUSE_RIGHT;
    parent=separator;
    childp=0;
  }
  if (WIDGET->args.use_tabs) {
    struct widget_args_tabber tabber_args={0};
    struct widget *tabber=widget_separator_spawn_panel(parent,childp,&widget_type_tabber,&tabber_args,sizeof(tabber_args));
    if (!tabber) return -1;
    tabber->parentuse=PARENTUSE_TABBER;
  }
  
  widget->bgcolor=wm_pixel_from_rgbx(0xc0c0c0ff);
  
  return 0;
}

/* Measure.
 */
 
static void _dashboard_measure(int *w,int *h,struct widget *widget,int maxw,int maxh) {
  *w=maxw;
  *h=maxh;
}

/* Pack.
 */
 
static void _dashboard_pack(struct widget *widget) {
  /* We have either 1 or 2 children. First is the optional menubar.
   * In any case, we can generalize that to packing vertically, and assign all remainder to the last child.
   */
  int y=0,i=widget->childc;
  struct widget **childp=widget->childv;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    int chw=0,chh=0;
    if (i) widget_measure(&chw,&chh,child,widget->w,widget->h-y);
    else chh=widget->h-y;
    if (chh>widget->h-y) chh=widget->h-y;
    child->x=0;
    child->y=y;
    child->w=widget->w;
    child->h=chh;
    y+=chh;
    widget_pack(child);
  }
}

/* Type definition.
 */
 
const struct widget_type widget_type_dashboard={
  .name="dashboard",
  .objlen=sizeof(struct widget_dashboard),
  .del=_dashboard_del,
  .init=_dashboard_init,
  .autorender=1,
  .measure=_dashboard_measure,
  .pack=_dashboard_pack,
};

/* Get a child by parentuse.
 */
 
static struct widget *widget_dashboard_get_child_inner(const struct widget *widget,uint32_t parentuse) {
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    if (child->parentuse==parentuse) return child;
    if (child->type==&widget_type_separator) { // Recur only into separators.
      if (child=widget_dashboard_get_child_inner(child,parentuse)) return child;
    }
  }
  return 0;
}
 
static struct widget *widget_dashboard_get_child(const struct widget *widget,uint32_t parentuse) {
  if (!widget||(widget->type!=&widget_type_dashboard)) return 0;
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    if (child->parentuse==parentuse) return child;
    if (child->type==&widget_type_separator) { // Recur only into separators.
      if (child=widget_dashboard_get_child_inner(child,parentuse)) return child;
    }
  }
  return 0;
}

/* Spawn a child of one of the separators, or of the tab bar.
 */
 
static struct widget *widget_dashboard_spawn_child(struct widget *widget,uint32_t parentuse,int side,const struct widget_type *type,const void *args,int argslen) {
  struct widget *separator=widget_dashboard_get_child(widget,parentuse);
  if (!separator) return 0;
  return widget_separator_spawn_panel(separator,side,type,args,argslen);
}

/* Child accessors, public.
 */
 
struct widget *widget_dashboard_get_menubar(const struct widget *widget) {
  return widget_dashboard_get_child(widget,PARENTUSE_MENUBAR);
}

struct widget *widget_dashboard_get_left_panel(const struct widget *widget) {
  return widget_dashboard_get_child(widget,PARENTUSE_LEFT);
}

struct widget *widget_dashboard_get_bottom_panel(const struct widget *widget) {
  return widget_dashboard_get_child(widget,PARENTUSE_BOTTOM);
}

struct widget *widget_dashboard_get_right_panel(const struct widget *widget) {
  return widget_dashboard_get_child(widget,PARENTUSE_RIGHT);
}

struct widget *widget_dashboard_get_main_panel(const struct widget *widget) {
  if (!widget||(widget->type!=&widget_type_dashboard)) return 0;
  if (WIDGET->args.use_tabs) return widget_dashboard_get_child(widget,PARENTUSE_TABBER);
  return widget_dashboard_get_child(widget,PARENTUSE_MAIN);
}

struct widget *widget_dashboard_spawn_left_panel(struct widget *widget,const struct widget_type *type,const void *args,int argslen) {
  return widget_dashboard_spawn_child(widget,PARENTUSE_LEFT,0,type,args,argslen);
}

struct widget *widget_dashboard_spawn_bottom_panel(struct widget *widget,const struct widget_type *type,const void *args,int argslen) {
  return widget_dashboard_spawn_child(widget,PARENTUSE_BOTTOM,1,type,args,argslen);
}

struct widget *widget_dashboard_spawn_right_panel(struct widget *widget,const struct widget_type *type,const void *args,int argslen) {
  return widget_dashboard_spawn_child(widget,PARENTUSE_RIGHT,1,type,args,argslen);
}

struct widget *widget_dashboard_spawn_main_panel(struct widget *widget,const struct widget_type *type,const void *args,int argslen) {
  if (!widget||(widget->type!=&widget_type_dashboard)) return 0;
  if (WIDGET->args.use_tabs) {
    struct widget *tabber=widget_dashboard_get_child(widget,PARENTUSE_TABBER);
    return widget_tabber_spawn(tabber,0,0,type,args,argslen);
  }
  if (WIDGET->args.use_right_panel) return widget_dashboard_spawn_child(widget,PARENTUSE_RIGHT,0,type,args,argslen);
  if (WIDGET->args.use_bottom_panel) return widget_dashboard_spawn_child(widget,PARENTUSE_BOTTOM,0,type,args,argslen);
  if (WIDGET->args.use_left_panel) return widget_dashboard_spawn_child(widget,PARENTUSE_LEFT,1,type,args,argslen);
  // We don't have a tabber or any sidebars. So the main panel is a direct child of (widget).
  if ((widget->childc>=1)&&(widget->childv[widget->childc-1]->parentuse==PARENTUSE_MAIN)) {
    widget_childv_remove_at(widget,widget->childc-1);
  }
  struct widget *main=widget_spawn(widget,type,args,argslen);
  if (!main) return 0;
  main->parentuse=PARENTUSE_MAIN;
  return main;
}
