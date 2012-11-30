#ifndef JV_AUX_H
#define JV_AUX_H

#include "jv.h"

jv jv_lookup(jv t, jv k);
jv jv_modify(jv t, jv k, jv v);
jv jv_insert(jv root, jv value, jv* path, int pathlen);

jv jv_keys(jv /*object or array*/);
int jv_cmp(jv, jv);

jv jv_group(jv objects, jv keys);
jv jv_sort(jv objects, jv keys);


#endif
