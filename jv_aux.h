#ifndef JV_AUX_H
#define JV_AUX_H

#include "jv.h"

jv jv_lookup(jv t, jv k);
jv jv_modify(jv t, jv k, jv v);
jv jv_insert(jv root, jv value, jv* path, int pathlen);


#endif
