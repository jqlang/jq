#ifndef JV_AUX_H
#define JV_AUX_H

#include "jv.h"

jv jv_get(jv t, jv k);
jv jv_set(jv t, jv k, jv v);
jv jv_has(jv t, jv k);
jv jv_setpath(jv root, jv path, jv value);
jv jv_getpath(jv root, jv path);
jv jv_delpaths(jv root, jv paths);

jv jv_keys(jv /*object or array*/);
int jv_cmp(jv, jv);

jv jv_group(jv objects, jv keys);
jv jv_sort(jv objects, jv keys);


#endif
