/* widget_packer.c
 * Generic one-dimensional layout widget.
 */
 
#include "lib/gui/gui_internal.h"

// (parentuse) bits in our children.
#define PARENTUSE_FLEX_YES 0x0001 /* If any child has this bit, only those with the bit are flexible. */
#define PARENTUSE_FLEX_NO  0x0002 /* Exclude just this child from growth. */

struct widget_packer {
  struct widget hdr;
  struct widget_args_packer args;
};

#define WIDGET ((struct widget_packer*)widget)

/* Cleanup.
 */
 
static void _packer_del(struct widget *widget) {
}

/* Init.
 */
 
static int _packer_init(struct widget *widget,const void *args,int argslen) {

  if (argslen==sizeof(struct widget_args_packer)) {
    WIDGET->args=*(const struct widget_args_packer*)args;
  } else {
    WIDGET->args.orientation='y';
    WIDGET->args.reverse=0;
    WIDGET->args.majoralign=-1;
    WIDGET->args.minoralign=-1;
    WIDGET->args.spacing=0;
  }

  return 0;
}

/* Initial measurements, invoked at both measure and pack.
 * (avail:major) is how much space remains after measuring the children. Can be negative.
 * (avail:minor) is just our maximum minus padding.
 * (kids) is the sum or max of children's preferred sizes.
 */
 
static void packer_premeasure(
  int *availw,int *availh,
  int *kidsw,int *kidsh,
  struct widget *widget,
  int maxw,int maxh,
  int assign_child_sizes
) {

  /* Take adjusted values for (maxw,maxh) reflecting our padding, spacing, and orientation.
   */
  int spacetotal=0;
  if (widget->childc>1) spacetotal=WIDGET->args.spacing*(widget->childc-1);
  *availw=maxw-(widget->padx<<1);
  *availh=maxh-(widget->pady<<1);
  if (widget->childc>1) {
    if (WIDGET->args.orientation=='x') {
      (*availw)-=spacetotal;
    } else {
      (*availh)-=spacetotal;
    }
  }

  /* Get children's preferred sizes.
   * Sum along the major axis and max along the minor.
   */
  *kidsw=*kidsh=0;
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    int chw=*availw,chh=*availh;
    widget_measure(&chw,&chh,child,chw,chh);
    if (assign_child_sizes) {
      child->w=chw;
      child->h=chh;
    }
    if (WIDGET->args.orientation=='x') {
      (*kidsw)+=chw;
      if (chh>*kidsh) *kidsh=chh;
      (*availw)-=chw;
    } else {
      if (chw>*kidsw) *kidsw=chw;
      (*kidsh)+=chh;
      (*availh)-=chh;
    }
  }
}

/* Measure.
 */
 
static void _packer_measure(int *w,int *h,struct widget *widget,int maxw,int maxh) {

  /* Get the total size that the kids want in aggregate.
   */
  int availw,availh,kidsw,kidsh;
  packer_premeasure(&availw,&availh,&kidsw,&kidsh,widget,maxw,maxh,0);
  
  /* Add padding and spacing, and that's our answer.
   * We don't care about the space remaining available; that's pack's problem.
   */
  int spacetotal=0;
  if (widget->childc>1) spacetotal=WIDGET->args.spacing*(widget->childc-1);
  *w=kidsw+(widget->padx<<1);
  *h=kidsh+(widget->pady<<1);
  if (WIDGET->args.orientation=='x') {
    (*w)+=spacetotal;
  } else {
    (*h)+=spacetotal;
  }
}

/* Trim a certain amount of excess, spreading the load across all child widgets.
 */
 
static void packer_trim_widths(struct widget *widget,int excess) {
  if (excess<1) return;
  int each=excess/widget->childc;
  int more=excess%widget->childc;
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    child->w-=each;
    if (more-->0) child->w--;
  }
}

static void packer_trim_heights(struct widget *widget,int excess) {
  if (excess<1) return;
  int each=excess/widget->childc;
  int more=excess%widget->childc;
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    child->h-=each;
    if (more-->0) child->h--;
  }
}

/* Spread some amount of excess width or height into all flexible children.
 */
 
static void packer_spread_excess_width(struct widget *widget,int excess) {
  if (excess<1) return;
  int yesc=0,maybec=0;
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    if (child->parentuse&PARENTUSE_FLEX_YES) {
      yesc++;
    } else if (child->parentuse&PARENTUSE_FLEX_NO) {
    } else {
      maybec++;
    }
  }
  uint32_t require;
  if (yesc) require=PARENTUSE_FLEX_YES;
  else if (maybec) { require=0; yesc=maybec; }
  else return; // Nowhere to spread it, give up.
  int each=excess/yesc;
  int more=excess%yesc;
  for (childp=widget->childv,i=widget->childc;i-->0;childp++) {
    struct widget *child=*childp;
    if (child->parentuse==require) {
      child->w+=each;
      if (more-->0) child->w++;
    }
  }
}

static void packer_spread_excess_height(struct widget *widget,int excess) {
  if (excess<1) return;
  int yesc=0,maybec=0;
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    if (child->parentuse&PARENTUSE_FLEX_YES) {
      yesc++;
    } else if (child->parentuse&PARENTUSE_FLEX_NO) {
    } else {
      maybec++;
    }
  }
  uint32_t require;
  if (yesc) require=PARENTUSE_FLEX_YES;
  else if (maybec) { require=0; yesc=maybec; }
  else return; // Nowhere to spread it, give up.
  int each=excess/yesc;
  int more=excess%yesc;
  for (childp=widget->childv,i=widget->childc;i-->0;childp++) {
    struct widget *child=*childp;
    if (child->parentuse==require) {
      child->h+=each;
      if (more-->0) child->h++;
    }
  }
}

/* Update minor-axis size of all children.
 * Either clamping (-1,0,1) or forcing (-2).
 */
 
static void packer_clamp_widths(struct widget *widget,int w) {
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    if (child->w>w) child->w=w;
  }
}

static void packer_force_widths(struct widget *widget,int w) {
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    child->w=w;
  }
}

static void packer_clamp_heights(struct widget *widget,int h) {
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    if (child->h>h) child->h=h;
  }
}

static void packer_force_heights(struct widget *widget,int h) {
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    child->h=h;
  }
}

/* Set one axis of all children's positions, in the given order.
 * "rl" and "bt" align to the positive side. "lr" and "tb" start at zero.
 */
 
static void packer_layout_lr(struct widget *widget) {
  int x=widget->padx,i=widget->childc;
  struct widget **childp=widget->childv;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    child->x=x;
    x+=child->w+WIDGET->args.spacing;
  }
}
 
static void packer_layout_rl(struct widget *widget) {
  int x=widget->w-widget->padx,i=widget->childc;
  struct widget **childp=widget->childv;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    x-=child->w;
    child->x=x;
    x-=WIDGET->args.spacing;
  }
}
 
static void packer_layout_tb(struct widget *widget) {
  int y=widget->pady,i=widget->childc;
  struct widget **childp=widget->childv;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    child->y=y;
    y+=child->h+WIDGET->args.spacing;
  }
}
 
static void packer_layout_bt(struct widget *widget) {
  int y=widget->h-widget->pady,i=widget->childc;
  struct widget **childp=widget->childv;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    y-=child->h;
    child->y=y;
    y-=WIDGET->args.spacing;
  }
}

/* Adjust the major-axis positions of all children to reflect aggregate alignment.
 * Do this after "layout", if we want to align in something other than the layout order.
 */
 
static void packer_realign_horz(struct widget *widget) {
  int totalw=WIDGET->args.spacing*(widget->childc-1);
  int kidsleft=widget->w;
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    totalw+=child->w;
    if (child->x<kidsleft) kidsleft=child->x;
  }
  int nleft;
  switch (WIDGET->args.majoralign) {
    case 0: nleft=(widget->w>>1)-(totalw>>1); break;
    case 1: nleft=widget->w-widget->padx-totalw; break;
    default: nleft=widget->padx; break;
  }
  int d=nleft-kidsleft;
  if (!d) return;
  for (i=widget->childc,childp=widget->childv;i-->0;childp++) {
    struct widget *child=*childp;
    child->x+=d;
  }
}

static void packer_realign_vert(struct widget *widget) {
  int totalh=WIDGET->args.spacing*(widget->childc-1);
  int kidstop=widget->h;
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    totalh+=child->h;
    if (child->y<kidstop) kidstop=child->y;
  }
  int ntop;
  switch (WIDGET->args.majoralign) {
    case 0: ntop=(widget->h>>1)-(totalh>>1); break;
    case 1: ntop=widget->h-widget->pady-totalh; break;
    default: ntop=widget->pady; break;
  }
  int d=ntop-kidstop;
  if (!d) return;
  for (i=widget->childc,childp=widget->childv;i-->0;childp++) {
    struct widget *child=*childp;
    child->y+=d;
  }
}

/* Force the minor-axis position of all children.
 */
 
static void packer_left_horz(struct widget *widget) {
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    child->x=widget->padx;
  }
}

static void packer_center_horz(struct widget *widget) {
  int x=widget->w>>1;
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    child->x=x-(child->w>>1);
  }
}

static void packer_right_horz(struct widget *widget) {
  int x=widget->w-widget->padx;
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    child->x=x-child->w;
  }
}

static void packer_top_vert(struct widget *widget) {
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    child->y=widget->pady;
  }
}

static void packer_center_vert(struct widget *widget) {
  int y=widget->h>>1;
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    child->y=y-(child->h>>1);
  }
}

static void packer_bottom_vert(struct widget *widget) {
  int y=widget->h-widget->pady;
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) {
    struct widget *child=*childp;
    child->y=y-child->h;
  }
}

/* Pack.
 */
 
static void _packer_pack(struct widget *widget) {
  if (widget->childc<1) return;

  /* Premeasure with (assign_child_sizes) to get the overall numbers and also per-child preferences.
   */
  int availw,availh,kidsw,kidsh;
  packer_premeasure(&availw,&availh,&kidsw,&kidsh,widget,widget->w,widget->h,1);
  
  /* (avail) is most relevant for the major axis.
   * If negative, we need to reduce child sizes.
   * If positive and (majoralign==-2), we need to spread that excess to the flexible children.
   */
  if (WIDGET->args.orientation=='x') {
    if (availw<0) {
      packer_trim_widths(widget,-availw);
    } else if (availw>0) {
      if (WIDGET->args.majoralign==-2) {
        packer_spread_excess_width(widget,availw);
      }
    }
  } else {
    if (availh<0) {
      packer_trim_heights(widget,-availh);
    } else if (availh>0) {
      if (WIDGET->args.majoralign==-2) {
        packer_spread_excess_height(widget,availh);
      }
    }
  }
  
  /* The minor axis is simpler: Either clamp or force each child's size to the available.
   */
  if (WIDGET->args.orientation=='x') {
    if (WIDGET->args.minoralign==-2) {
      packer_force_heights(widget,availh);
    } else {
      packer_clamp_heights(widget,availh);
    }
  } else {
    if (WIDGET->args.minoralign==-2) {
      packer_force_widths(widget,availw);
    } else {
      packer_clamp_widths(widget,availw);
    }
  }
  
  /* Now the children all have their correct size.
   * Give them positions, following one of four orders.
   */
  if (WIDGET->args.orientation=='x') {
    if (WIDGET->args.reverse) { // Right to left.
      packer_layout_rl(widget);
      if ((WIDGET->args.majoralign!=-2)&&(WIDGET->args.majoralign<=0)) packer_realign_horz(widget);
    } else { // Left to right.
      packer_layout_lr(widget);
      if (WIDGET->args.majoralign>=0) packer_realign_horz(widget);
    }
    switch (WIDGET->args.minoralign) {
      case 0: packer_center_vert(widget); break;
      case 1: packer_bottom_vert(widget); break;
      default: packer_top_vert(widget); break;
    }
  } else {
    if (WIDGET->args.reverse) { // Bottom to top.
      packer_layout_bt(widget);
      if ((WIDGET->args.majoralign!=-2)&&(WIDGET->args.majoralign<=0)) packer_realign_vert(widget);
    } else { // Top to bottom.
      packer_layout_tb(widget);
      if (WIDGET->args.majoralign>=0) packer_realign_vert(widget);
    }
    switch (WIDGET->args.minoralign) {
      case 0: packer_center_horz(widget); break;
      case 1: packer_right_horz(widget); break;
      default: packer_left_horz(widget); break;
    }
  }
  
  /* And finally, pack all the children.
   */
  struct widget **childp=widget->childv;
  int i=widget->childc;
  for (;i-->0;childp++) widget_pack(*childp);
}

/* Type definition.
 */
 
const struct widget_type widget_type_packer={
  .name="packer",
  .objlen=sizeof(struct widget_packer),
  .autorender=1,
  .del=_packer_del,
  .init=_packer_init,
  .measure=_packer_measure,
  .pack=_packer_pack,
};

/* Public: Mark a child in a flex list.
 */
 
int widget_packer_flex_child(struct widget *widget,struct widget *child,int flex) {
  if (!widget||(widget->type!=&widget_type_packer)) return -1;
  if (!child||(child->parent!=widget)) return -1;
  child->parentuse=0;
  if (flex>0) child->parentuse=PARENTUSE_FLEX_YES;
  else if (flex<0) child->parentuse=PARENTUSE_FLEX_NO;
  return 0;
}
