#include "lib/gui/gui_internal.h"

struct widget_tabber {
  struct widget hdr;
  struct font *font;
  struct page {
    struct widget *tab; // STRONG, also always a child.
    struct widget *panel; // STRONG, sometimes a child.
  } *pagev;
  int pagec,pagea;
  int pagep;
  int liney;
};

#define WIDGET ((struct widget_tabber*)widget)

/* Cleanup.
 */
 
static void page_cleanup(struct page *page) {
  widget_del(page->tab);
  widget_del(page->panel);
}
 
static void _tabber_del(struct widget *widget) {
  if (WIDGET->pagev) {
    while (WIDGET->pagec-->0) page_cleanup(WIDGET->pagev+WIDGET->pagec);
    free(WIDGET->pagev);
  }
  font_del(WIDGET->font);
}

/* Init.
 */
 
static int _tabber_init(struct widget *widget,const void *args,int argslen) {
  if (args&&(argslen==sizeof(struct widget_args_tabber))) {
    const struct widget_args_tabber *ARGS=args;
    if (ARGS->font) {
      if (font_ref(ARGS->font)<0) return -1;
      WIDGET->font=ARGS->font;
    }
  }
  if (!WIDGET->font) {
    struct font *font=gui_get_default_font(widget->ctx);
    if (font_ref(font)<0) return -1;
    WIDGET->font=font;
  }
  WIDGET->pagep=-1;
  return 0;
}

/* Render.
 */
 
static void _tabber_render(struct widget *widget,struct image *dst) {
  image_fill_rect(dst,0,WIDGET->liney,widget->w,1,0);
}

/* Measure.
 */
 
static void _tabber_measure(int *w,int *h,struct widget *widget,int maxw,int maxh) {
  if (WIDGET->pagec<1) {
    *w=widget->padx<<1;
    *h=widget->pady<<1;
    return;
  }
  int tabwsum=0;
  int tabhmax=0;
  int panelwmax=0;
  int panelhmax=0;
  int i=WIDGET->pagec;
  struct page *page=WIDGET->pagev;
  for (;i-->0;page++) {
    int chw=0,chh=0;
    widget_measure(&chw,&chh,page->tab,maxw,maxh);
    tabwsum+=chw;
    if (chh>tabhmax) tabhmax=chh;
    widget_measure(&chw,&chh,page->panel,maxw,maxh);
    if (chw>panelwmax) panelwmax=chw;
    if (chh>panelhmax) panelhmax=chh;
  }
  *w=(tabwsum>panelwmax)?tabwsum:panelwmax;
  *h=tabhmax+panelhmax;
  (*w)+=widget->padx<<1;
  (*h)+=widget->pady<<1;
}

/* Pack.
 */
 
static void _tabber_pack(struct widget *widget) {

  /* First establish the tab bar height.
   * Be thorough about it, in case we ever do variable-height tabs.
   * Also assign widths in this pass, so we don't have to measure the tabs twice.
   */
  int tabh=0;
  int i=WIDGET->pagec;
  struct page *page=WIDGET->pagev;
  for (;i-->0;page++) {
    int chw=0,chh=0;
    widget_measure(&chw,&chh,page->tab,widget->w,widget->h);
    page->tab->w=chw;
    if (chh>tabh) tabh=chh;
  }
  WIDGET->liney=widget->pady+tabh;

  /* Pack tabs horizontally, with bottom edges aligned.
   * All panels get the same box, everything below the tab bar.
   */
  int x=widget->padx;
  int y=widget->pady;
  int panely=WIDGET->liney+1;
  int panelw=widget->w-(widget->padx<<1);
  if (panelw<0) panelw=0;
  int panelh=widget->h-widget->pady-panely;
  if (panelh<0) panelh=0;
  for (i=WIDGET->pagec,page=WIDGET->pagev;i-->0;page++) {
    page->tab->x=x;
    page->tab->y=y;
    page->tab->h=tabh;
    page->panel->x=widget->padx;
    page->panel->y=panely;
    page->panel->w=panelw;
    page->panel->h=panelh;
    widget_pack(page->tab);
    widget_pack(page->panel);
    x+=page->tab->w;
  }
}

/* Type definition.
 */
 
const struct widget_type widget_type_tabber={
  .name="tabber",
  .objlen=sizeof(struct widget_tabber),
  .del=_tabber_del,
  .init=_tabber_init,
  .render=_tabber_render,
  .autorender=1,
  .measure=_tabber_measure,
  .pack=_tabber_pack,
};

/* Callback when one of our tabs gets clicked.
 */
 
static void tabber_cb_tab(struct widget *caller,void *userdata) {
  struct widget *widget=userdata;
  struct page *page=WIDGET->pagev;
  int i=0;
  for (;i<WIDGET->pagec;i++,page++) {
    if (page->tab!=caller) continue;
    widget_tabber_focus_tab(widget,i);
    return;
  }
}

/* Generate (tab) and (panel) in a new blank page.
 * This does not add them as children.
 */
 
static int tabber_generate_page(
  struct widget *widget,
  struct page *page,
  const char *label,int labelc,
  const struct widget_type *type,
  const void *args,int argslen
) {
  struct widget_args_button button_args={
    .font=WIDGET->font,
    .text=label,
    .textc=labelc,
    .cb=tabber_cb_tab,
    .userdata=widget,
  };
  if (!(page->tab=widget_new(widget->ctx,&widget_type_button,&button_args,sizeof(button_args)))) return -1;
  if (!(page->panel=widget_new(widget->ctx,type,args,argslen))) return -1;
  return 0;
}

/* Spawn a new tab.
 */
 
struct widget *widget_tabber_spawn(
  struct widget *widget,
  const char *label,int labelc,
  const struct widget_type *type,
  const void *args,int argslen
) {
  if (!widget||(widget->type!=&widget_type_tabber)) return 0;
  
  if (WIDGET->pagec>=WIDGET->pagea) {
    int na=WIDGET->pagea+8;
    if (na>INT_MAX/sizeof(struct page)) return 0;
    void *nv=realloc(WIDGET->pagev,sizeof(struct page)*na);
    if (!nv) return 0;
    WIDGET->pagev=nv;
    WIDGET->pagea=na;
  }
  struct page *page=WIDGET->pagev+WIDGET->pagec++;
  memset(page,0,sizeof(struct page));
  
  if (tabber_generate_page(widget,page,label,labelc,type,args,argslen)<0) {
    page_cleanup(page);
    WIDGET->pagec--;
    return 0;
  }
  
  if (widget_childv_insert(widget,-1,page->tab)<0) {
    page_cleanup(page);
    WIDGET->pagec--;
    return 0;
  }
  
  widget_tabber_focus_tab(widget,WIDGET->pagec-1);
  
  return page->panel;
}

/* Public access to tabs and panels.
 */

int widget_tabber_count_tabs(const struct widget *widget) {
  if (!widget||(widget->type!=&widget_type_tabber)) return 0;
  return WIDGET->pagec;
}

int widget_tabber_find_tab(const struct widget *widget,const struct widget *panel) {
  if (!widget||(widget->type!=&widget_type_tabber)) return -1;
  struct page *page=WIDGET->pagev;
  int i=0;
  for (;i<WIDGET->pagec;i++,page++) {
    if (page->panel==panel) return i;
  }
  return -1;
}

struct widget *widget_tabber_get_tab(const struct widget *widget,int p) {
  if (!widget||(widget->type!=&widget_type_tabber)) return 0;
  if (p<0) return 0;
  if (p>=WIDGET->pagec) return 0;
  return WIDGET->pagev[p].tab;
}

struct widget *widget_tabber_get_panel(const struct widget *widget,int p) {
  if (!widget||(widget->type!=&widget_type_tabber)) return 0;
  if (p<0) return 0;
  if (p>=WIDGET->pagec) return 0;
  return WIDGET->pagev[p].panel;
}

struct widget *widget_tabber_get_tab_for_panel(const struct widget *widget,const struct widget *panel) {
  if (!widget||(widget->type!=&widget_type_tabber)) return 0;
  if (!panel) return 0;
  struct page *page=WIDGET->pagev;
  int i=WIDGET->pagec;
  for (;i-->0;page++) {
    if (page->panel==panel) return page->tab;
  }
  return 0;
}

struct widget *widget_tabber_get_panel_for_tab(const struct widget *widget,const struct widget *tab) {
  if (!widget||(widget->type!=&widget_type_tabber)) return 0;
  if (!tab) return 0;
  struct page *page=WIDGET->pagev;
  int i=WIDGET->pagec;
  for (;i-->0;page++) {
    if (page->tab==tab) return page->panel;
  }
  return 0;
}

int widget_tabber_remove_tab(struct widget *widget,int p) {
  if (!widget||(widget->type!=&widget_type_tabber)) return -1;
  if (p<0) return -1;
  if (p>=WIDGET->pagec) return -1;
  struct page *page=WIDGET->pagev+p;
  widget_childv_remove(widget,page->tab);
  widget_childv_remove(widget,page->panel);
  page_cleanup(page);
  WIDGET->pagec--;
  memmove(page,page+1,sizeof(struct page)*(WIDGET->pagec-p));
  if (WIDGET->pagep==p) WIDGET->pagep=-1;
  else if (WIDGET->pagep>p) WIDGET->pagep--;
  return 0;
}

/* Focus a tab.
 */
 
int widget_tabber_focus_tab(struct widget *widget,int p) {
  if (!widget||(widget->type!=&widget_type_tabber)) return -1;
  if ((p<0)||(p>=WIDGET->pagec)) p=-1;
  if (p==WIDGET->pagep) return 0;
  if ((WIDGET->pagep>=0)&&(WIDGET->pagep<WIDGET->pagec)) {
    struct page *page=WIDGET->pagev+WIDGET->pagep;
    widget_childv_remove(widget,page->panel);
  }
  WIDGET->pagep=p;
  if ((WIDGET->pagep>=0)&&(WIDGET->pagep<WIDGET->pagec)) {
    struct page *page=WIDGET->pagev+WIDGET->pagep;
    widget_childv_insert(widget,-1,page->panel);
  }
  //TODO Should be some highlight of the tab too
  return 0;
}
