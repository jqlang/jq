#ifndef JV_PRIVATE
#define JV_PRIVATE

int jvp_number_cmp(jv, jv);
int jvp_number_is_nan(jv);

/* Internal variant used by fromstream.  String and numeric path components
 * may replace an incompatible ancestor with the container they require. */
jv jv_setpath_coerce(jv root, jv path, jv value);

#endif //JV_PRIVATE
