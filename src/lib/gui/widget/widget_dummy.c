#include "lib/gui/gui_internal.h"

const struct widget_type widget_type_dummy={
  .name="dummy",
  .objlen=sizeof(struct widget),
  .autorender=1,
};
