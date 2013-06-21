#ifndef JV_PARSE_H
#define JV_PARSE_H
#include "jv_dtoa.h"
struct jv_parser {
  const char* curr_buf;
  int curr_buf_length;
  int curr_buf_pos;
  int curr_buf_is_partial;
  unsigned bom_strip_position;

  jv* stack;
  int stackpos;
  int stacklen;
  jv next;
  
  char* tokenbuf;
  int tokenpos;
  int tokenlen;

  int line, column;
  
  struct dtoa_context dtoa;

  enum {
    JV_PARSER_NORMAL,
    JV_PARSER_STRING,
    JV_PARSER_STRING_ESCAPE
  } st;
};

#endif
