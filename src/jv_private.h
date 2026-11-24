#ifndef JV_PRIVATE
#define JV_PRIVATE

int jvp_number_cmp(jv, jv);
int jvp_number_is_nan(jv);
int jvp_parser_is_seq(const jv_parser *);
void jvp_parser_reset_input(jv_parser *);

#endif //JV_PRIVATE
