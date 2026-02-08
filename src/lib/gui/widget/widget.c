#include "lib/gui/gui_internal.h"

/* Delete.
 */
 
void widget_del(struct widget *widget) {
  if (!widget) return;
  if (widget->refc-->1) return;
  if (widget->parent) fprintf(stderr,
    "%s:%d:WARNING: Deleting widget %p (%s), parent==%p. Must be null by this point.\n",
    __FILE__,__LINE__,widget,widget->type->name,widget->parent
  );
  if (widget->childv) {
    while (widget->childc-->0) {
      struct widget *child=widget->childv[widget->childc];
      child->parent=0;
      widget_del(child);
    }
    free(widget->childv);
  }
  if (widget->type->del) widget->type->del(widget);
  free(widget);
}

/* Retain.
 */

int widget_ref(struct widget *widget) {
  if (!widget) return -1;
  if ((widget->refc<1)||(widget->refc>=INT_MAX)) return -1;
  widget->refc++;
  return 0;
}

/* New.
 */

struct widget *widget_new(
  struct gui_context *ctx,
  const struct widget_type *type,
  const void *args,int argslen
) {
  if (!ctx||!type) return 0;
  struct widget *widget=calloc(1,type->objlen);
  if (!widget) return 0;
  widget->refc=1;
  widget->type=type;
  widget->ctx=ctx;
  if (type->init&&(type->init(widget,args,argslen)<0)) {
    widget_del(widget);
    return 0;
  }
  return widget;
}

/* Spawn.
 */

struct widget *widget_spawn(
  struct widget *parent,
  const struct widget_type *type,
  const void *args,int argslen
) {
  if (!parent||!type) return 0;
  struct widget *widget=widget_new(parent->ctx,type,args,argslen);
  if (!widget) return 0;
  if (widget_childv_insert(parent,-1,widget)<0) {
    widget_del(widget);
    return 0;
  }
  widget_del(widget);
  return widget;
}

/* Add child.
 */

int widget_childv_insert(struct widget *parent,int p,struct widget *child) {
  if (!parent||!child) return -1;
  if ((p<0)||(p>parent->childc)) p=parent->childc;
  
  // If this relationship already exists, it's only a request to change the order.
  if (child->parent==parent) {
    int pvp=widget_childv_search(parent,child);
    if (pvp<0) return -1; // Inconsistent references!
    if (p>pvp) p--; // Rephrase (p) to be the index after removal.
    if (p==pvp) return 0; // Two possible redundant positions.
    // It copies more than necessary, but to keep things neat, do a full remove then a full insert:
    memmove(parent->childv+pvp,parent->childv+pvp+1,sizeof(void*)*(parent->childc-1-pvp));
    memmove(parent->childv+p+1,parent->childv+p,sizeof(void*)*(parent->childc-1-p));
    parent->childv[p]=child;
    return 0;
  }
  
  // If (child) already has a parent, it's an error. Do not remove from the existing, caller must do that manually.
  if (child->parent) return -1;
  
  if (parent->childc>=parent->childa) {
    int na=parent->childa+8;
    if (na>INT_MAX/sizeof(void*)) return -1;
    void *nv=realloc(parent->childv,sizeof(void*)*na);
    if (!nv) return -1;
    parent->childv=nv;
    parent->childa=na;
  }
  if (widget_ref(child)<0) return -1;
  memmove(parent->childv+p+1,parent->childv+p,sizeof(void*)*(parent->childc-p));
  parent->childv[p]=child;
  parent->childc++;
  child->parent=parent;
  return 0;
}

/* Remove child.
 */
 
int widget_childv_remove(struct widget *parent,struct widget *child) {
  return widget_childv_remove_at(parent,widget_childv_search(parent,child));
}

int widget_childv_remove_at(struct widget *parent,int p) {
  if (!parent||(p<0)||(p>=parent->childc)) return -1;
  struct widget *child=parent->childv[p];
  parent->childc--;
  memmove(parent->childv+p,parent->childv+p+1,sizeof(void*)*(parent->childc-p));
  child->parent=0;
  widget_del(child);
  return 0;
}

/* Search child list.
 */
 
int widget_childv_search(const struct widget *parent,const struct widget *child) {
  if (!parent||!child) return -1;
  int i=0;
  struct widget **p=parent->childv;
  for (;i<parent->childc;p++) if (*p==child) return i;
  return -1;
}

/* Test ancestry.
 */
 
int widget_is_ancestor(const struct widget *ancestor,const struct widget *descendant) {
  if (!ancestor||!descendant) return 0;
  while (descendant) {
    if (ancestor==descendant) return 1;
    descendant=descendant->parent;
  }
}

/* Get root.
 */
 
struct widget *widget_get_root(struct widget *widget) {
  if (!widget) return 0;
  while (widget->parent) widget=widget->parent;
  return widget;
}

/* Transform coords.
 */

void widget_coords_global_from_local(int *x,int *y,const struct widget *widget) {
  if (!widget) return;
  do {
    (*x)-=widget->x-widget->scrollx;
    (*y)-=widget->y-widget->scrolly;
  } while (widget=widget->parent);
}

void widget_coords_local_from_global(int *x,int *y,const struct widget *widget) {
  if (!widget) return;
  do {
    (*x)+=widget->x-widget->scrollx;
    (*y)+=widget->y-widget->scrolly;
  } while (widget=widget->parent);
}

void widget_get_clip(int *x,int *y,int *w,int *h,const struct widget *widget) {
  if (!widget) return;
  *x=widget->x;
  *y=widget->y;
  *w=widget->w;
  *h=widget->h;
  if (*x<0) { (*w)+=(*x); *x=0; }
  if (*y<0) { (*h)+=(*y); *y=0; }
  while (widget=widget->parent) {
    (*x)+=widget->x-widget->scrollx;
    (*y)+=widget->y-widget->scrolly;
    if (*x<0) { (*w)+=(*x); *x=0; }
    if (*y<0) { (*h)+=(*y); *y=0; }
    int pr=widget->x+widget->w;
    int pb=widget->y+widget->h;
    if (*x>pr-*w) (*w)=pr-(*x);
    if (*y>pb-*h) (*h)=pb-(*y);
  }
}

/* Render children.
 */
 
void widget_render_children(struct widget *widget,struct image *dst) {
  if (!widget||!dst) return;
  if ((dst->pixelsize!=32)||(dst->stride&3)||!dst->writeable) return;
  int stridewords=dst->stride>>2;
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    int x=child->x-widget->scrollx;
    int y=child->y-widget->scrolly;
    int w=child->w;
    int h=child->h;
    int x0=0,y0=0;
    if (x<0) { x0=x; w+=x; x=0; }
    if (y<0) { y0=y; h+=y; y=0; }
    if (x>dst->w-w) w=dst->w-x;
    if (y>dst->h-h) h=dst->h-y;
    if ((w<1)||(h<1)) continue;
    struct image sub={
      .v=((uint32_t*)dst->v)+stridewords*y+x,
      .w=w,
      .h=h,
      .stride=dst->stride,
      .pixelsize=32,
      .writeable=1,
      .x0=x0,
      .y0=y0,
    };
    child->type->render(child,&sub);
  }
}

/* Measure.
 */
 
void widget_measure(int *w,int *h,struct widget *widget,int maxw,int maxh) {
  if (!widget) return;
  if (widget->type->measure) {
    widget->type->measure(w,h,widget,maxw,maxh);
  } else if (widget->childc) {
    // Take the single largest dimension desired by any of my children, and add my padding to it.
    int defw=(*w)-(widget->padx<<1);
    int defh=(*h)-(widget->pady<<1);
    if (defw<0) defw=0;
    if (defh<0) defh=0;
    int whi=0,hhi=0;
    struct widget **childp=widget->childv;
    int i=widget->childc;
    for (;i-->0;childp++) {
      struct widget *child=*childp;
      int chw=defw,chh=defh;
      widget_measure(&chw,&chh,child,maxw,maxh);
      if (chw>whi) whi=chw;
      if (chh>hhi) hhi=chh;
    }
    *w=whi+(widget->padx<<1);
    *h=hhi+(widget->pady<<1);
  } else {
    // No hook or children, retain the caller's default.
  }
}

void widget_pack(struct widget *widget) {
  if (!widget) return;
  if (widget->type->pack) {
    widget->type->pack(widget);
  } else {
    // All children get my full size, minus padding.
    // If you don't have a pack hook, hopefully you don't take more than one child.
    int chw=widget->w-(widget->padx<<1);
    int chh=widget->h-(widget->pady<<1);
    if (chw<0) chw=0;
    if (chh<0) chh=0;
    struct widget **childp=widget->childv;
    int i=widget->childc;
    for (;i-->0;childp++) {
      struct widget *child=*childp;
      child->x=widget->padx;
      child->y=widget->pady;
      child->w=chw;
      child->h=chh;
      widget_pack(child);
    }
  }
}
