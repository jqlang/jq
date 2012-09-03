
struct jv_parser {
  jv* stack;
  int stackpos;
  int stacklen;
  jv next;
  int hasnext;
  
  char* tokenbuf;
  int tokenpos;
  int tokenlen;
  
  struct dtoa_context dtoa;

  enum {
    JV_PARSER_NORMAL,
    JV_PARSER_STRING,
    JV_PARSER_STRING_ESCAPE
  } st;
};
