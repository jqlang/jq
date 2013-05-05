#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jv.h"
#include "jv_dtoa.h"
#include "jv_parse.h"
#include "jv_unicode.h"
#include "jv_alloc.h"

typedef const char* presult;

#define TRY(x) do {presult msg__ = (x); if (msg__) return msg__; } while(0)
#ifdef __GNUC__
#define pfunc __attribute__((warn_unused_result)) presult
#else
#define pfunc presult
#endif

void jv_parser_init(struct jv_parser* p) {
  p->stack = 0;
  p->stacklen = p->stackpos = 0;
  p->next = jv_invalid();
  p->tokenbuf = 0;
  p->tokenlen = p->tokenpos = 0;
  p->st = JV_PARSER_NORMAL;
  p->curr_buf = 0;
  p->curr_buf_length = p->curr_buf_pos = p->curr_buf_is_partial = 0;
  p->bom_strip_position = 0;
  p->line = 1;
  p->column = 0;
  jvp_dtoa_context_init(&p->dtoa);
}

void jv_parser_free(struct jv_parser* p) {
  jv_free(p->next);
  for (int i=0; i<p->stackpos; i++) 
    jv_free(p->stack[i]);
  jv_mem_free(p->stack);
  jv_mem_free(p->tokenbuf);
  jvp_dtoa_context_free(&p->dtoa);
}

static pfunc value(struct jv_parser* p, jv val) {
  if (jv_is_valid(p->next)) return "Expected separator between values";
  jv_free(p->next);
  p->next = val;
  return 0;
}

static void push(struct jv_parser* p, jv v) {
  assert(p->stackpos <= p->stacklen);
  if (p->stackpos == p->stacklen) {
    p->stacklen = p->stacklen * 2 + 10;
    p->stack = jv_mem_realloc(p->stack, p->stacklen * sizeof(jv));
  }
  assert(p->stackpos < p->stacklen);
  p->stack[p->stackpos++] = v;
}

static pfunc token(struct jv_parser* p, char ch) {
  switch (ch) {
  case '[':
    if (jv_is_valid(p->next)) return "Expected separator between values";
    push(p, jv_array());
    break;

  case '{':
    if (jv_is_valid(p->next)) return "Expected separator between values";
    push(p, jv_object());
    break;

  case ':':
    if (!jv_is_valid(p->next)) 
      return "Expected string key before ':'";
    if (p->stackpos == 0 || jv_get_kind(p->stack[p->stackpos-1]) != JV_KIND_OBJECT)
      return "':' not as part of an object";
    if (jv_get_kind(p->next) != JV_KIND_STRING)
      return "Object keys must be strings";
    push(p, p->next);
    p->next = jv_invalid();
    break;

  case ',':
    if (!jv_is_valid(p->next))
      return "Expected value before ','";
    if (p->stackpos == 0)
      return "',' not as part of an object or array";
    if (jv_get_kind(p->stack[p->stackpos-1]) == JV_KIND_ARRAY) {
      p->stack[p->stackpos-1] = jv_array_append(p->stack[p->stackpos-1], p->next);
      p->next = jv_invalid();
    } else if (jv_get_kind(p->stack[p->stackpos-1]) == JV_KIND_STRING) {
      assert(p->stackpos > 1 && jv_get_kind(p->stack[p->stackpos-2]) == JV_KIND_OBJECT);
      p->stack[p->stackpos-2] = jv_object_set(p->stack[p->stackpos-2], 
                                              p->stack[p->stackpos-1], p->next);
      p->stackpos--;
      p->next = jv_invalid();
    } else {
      // this case hits on input like {"a", "b"}
      return "Objects must consist of key:value pairs";
    }
    break;

  case ']':
    if (p->stackpos == 0 || jv_get_kind(p->stack[p->stackpos-1]) != JV_KIND_ARRAY)
      return "Unmatched ']'";
    if (jv_is_valid(p->next)) {
      p->stack[p->stackpos-1] = jv_array_append(p->stack[p->stackpos-1], p->next);
      p->next = jv_invalid();
    } else {
      if (jv_array_length(jv_copy(p->stack[p->stackpos-1])) != 0) {
        // this case hits on input like [1,2,3,]
        return "Expected another array element";
      }
    }
    jv_free(p->next);
    p->next = p->stack[--p->stackpos];
    break;

  case '}':
    if (p->stackpos == 0)
      return "Unmatched '}'";
    if (jv_is_valid(p->next)) {
      if (jv_get_kind(p->stack[p->stackpos-1]) != JV_KIND_STRING)
        return "Objects must consist of key:value pairs";
      assert(p->stackpos > 1 && jv_get_kind(p->stack[p->stackpos-2]) == JV_KIND_OBJECT);
      p->stack[p->stackpos-2] = jv_object_set(p->stack[p->stackpos-2], 
                                              p->stack[p->stackpos-1], p->next);
      p->stackpos--;
      p->next = jv_invalid();
    } else {
      if (jv_get_kind(p->stack[p->stackpos-1]) != JV_KIND_OBJECT)
        return "Unmatched '}'";
      if (jv_object_length(jv_copy(p->stack[p->stackpos-1])) != 0)
        return "Expected another key-value pair";
    }
    jv_free(p->next);
    p->next = p->stack[--p->stackpos];
    break;
  }
  return 0;
}


static void tokenadd(struct jv_parser* p, char c) {
  assert(p->tokenpos <= p->tokenlen);
  if (p->tokenpos == p->tokenlen) {
    p->tokenlen = p->tokenlen*2 + 256;
    p->tokenbuf = jv_mem_realloc(p->tokenbuf, p->tokenlen);
  }
  assert(p->tokenpos < p->tokenlen);
  p->tokenbuf[p->tokenpos++] = c;
}

static int unhex4(char* hex) {
  int r = 0;
  for (int i=0; i<4; i++) {
    char c = *hex++;
    int n;
    if ('0' <= c && c <= '9') n = c - '0';
    else if ('a' <= c && c <= 'f') n = c - 'a' + 10;
    else if ('A' <= c && c <= 'F') n = c - 'A' + 10;
    else return -1;
    r <<= 4;
    r |= n;
  }
  return r;
}

static pfunc found_string(struct jv_parser* p) {
  char* in = p->tokenbuf;
  char* out = p->tokenbuf;
  char* end = p->tokenbuf + p->tokenpos;
  
  while (in < end) {
    char c = *in++;
    if (c == '\\') {
      if (in >= end)
        return "Expected escape character at end of string";
      c = *in++;
      switch (c) {
      case '\\':
      case '"':
      case '/': *out++ = c;    break;
      case 'b': *out++ = '\b'; break;
      case 'f': *out++ = '\f'; break;
      case 't': *out++ = '\t'; break;
      case 'n': *out++ = '\n'; break;
      case 'r': *out++ = '\r'; break;

      case 'u':
        /* ahh, the complicated case */
        if (in + 4 > end)
          return "Invalid \\uXXXX escape";
        int hexvalue = unhex4(in);
        if (hexvalue < 0)
          return "Invalid characters in \\uXXXX escape";
        unsigned long codepoint = (unsigned long)hexvalue;
        in += 4;
        if (0xD800 <= codepoint && codepoint <= 0xDBFF) {
          /* who thought UTF-16 surrogate pairs were a good idea? */
          if (in + 6 > end || in[0] != '\\' || in[1] != 'u')
            return "Invalid \\uXXXX\\uXXXX surrogate pair escape";
          unsigned long surrogate = unhex4(in+2);
          if (!(0xDC00 <= surrogate && surrogate <= 0xDFFF))
            return "Invalid \\uXXXX\\uXXXX surrogate pair escape";
          in += 6;
          codepoint = 0x10000 + (((codepoint - 0xD800) << 10)
                                 |(surrogate - 0xDC00));
        }
        // FIXME assert valid codepoint
        out += jvp_utf8_encode(codepoint, out);
        break;

      default:
        return "Invalid escape";
      }
    } else {
      *out++ = c;
    }
  }
  TRY(value(p, jv_string_sized(p->tokenbuf, out - p->tokenbuf)));
  p->tokenpos = 0;
  return 0;
}

static pfunc check_literal(struct jv_parser* p) {
  if (p->tokenpos == 0) return 0;

  const char* pattern = 0;
  int plen;
  jv v;
  switch (p->tokenbuf[0]) {
  case 't': pattern = "true"; plen = 4; v = jv_true(); break;
  case 'f': pattern = "false"; plen = 5; v = jv_false(); break;
  case 'n': pattern = "null"; plen = 4; v = jv_null(); break;
  }
  if (pattern) {
    if (p->tokenpos != plen) return "Invalid literal";
    for (int i=0; i<plen; i++) 
      if (p->tokenbuf[i] != pattern[i])
        return "Invalid literal";
    TRY(value(p, v));
  } else {
    // FIXME: better parser
    p->tokenbuf[p->tokenpos] = 0; // FIXME: invalid
    char* end = 0;
    double d = jvp_strtod(&p->dtoa, p->tokenbuf, &end);
    if (end == 0 || *end != 0)
      return "Invalid numeric literal";
    TRY(value(p, jv_number(d)));
  }
  p->tokenpos = 0;
  return 0;
}

typedef enum {
  LITERAL,
  WHITESPACE,
  STRUCTURE,
  QUOTE,
  INVALID
} chclass;

static chclass classify(char c) {
  switch (c) {
  case ' ':
  case '\t':
  case '\r':
  case '\n':
    return WHITESPACE;
  case '"':
    return QUOTE;
  case '[':
  case ',':
  case ']':
  case '{':
  case ':':
  case '}':
    return STRUCTURE;
  default:
    return LITERAL;
  }
}


static const presult OK = "output produced";

static int check_done(struct jv_parser* p, jv* out) {
  if (p->stackpos == 0 && jv_is_valid(p->next)) {
    *out = p->next;
    p->next = jv_invalid();
    return 1;
  } else {
    return 0;
  }
}

static pfunc scan(struct jv_parser* p, char ch, jv* out) {
  p->column++;
  if (ch == '\n') {
    p->line++;
    p->column = 0;
  }
  presult answer = 0;
  if (p->st == JV_PARSER_NORMAL) {
    chclass cls = classify(ch);
    if (cls != LITERAL) {
      TRY(check_literal(p));
      if (check_done(p, out)) answer = OK;
    }
    switch (cls) {
    case LITERAL:
      tokenadd(p, ch);
      break;
    case WHITESPACE:
      break;
    case QUOTE:
      p->st = JV_PARSER_STRING;
      break;
    case STRUCTURE:
      TRY(token(p, ch));
      break;
    case INVALID:
      return "Invalid character";
    }
    if (check_done(p, out)) answer = OK;
  } else {
    if (ch == '"' && p->st == JV_PARSER_STRING) {
      TRY(found_string(p));
      p->st = JV_PARSER_NORMAL;
      if (check_done(p, out)) answer = OK;
    } else {
      tokenadd(p, ch);
      if (ch == '\\' && p->st == JV_PARSER_STRING) {
        p->st = JV_PARSER_STRING_ESCAPE;
      } else {
        p->st = JV_PARSER_STRING;
      }
    }
  }
  return answer;
}

static const unsigned char UTF8_BOM[] = {0xEF,0xBB,0xBF};

void jv_parser_set_buf(struct jv_parser* p, const char* buf, int length, int is_partial) {
  assert((p->curr_buf == 0 || p->curr_buf_pos == p->curr_buf_length)
         && "previous buffer not exhausted");
  while (p->bom_strip_position < sizeof(UTF8_BOM)) {
    if ((unsigned char)*buf == UTF8_BOM[p->bom_strip_position]) {
      // matched a BOM character
      buf++;
      length--;
      p->bom_strip_position++;
    } else {
      if (p->bom_strip_position == 0) {
        // no BOM in this document
        p->bom_strip_position = sizeof(UTF8_BOM);
      } else {
        // malformed BOM (prefix present, rest missing)
        p->bom_strip_position = 0xff;
      }
    }
  }
  p->curr_buf = buf;
  p->curr_buf_length = length;
  p->curr_buf_pos = 0;
  p->curr_buf_is_partial = is_partial;
}

jv jv_parser_next(struct jv_parser* p) {
  assert(p->curr_buf && "a buffer must be provided");
  if (p->bom_strip_position == 0xff) return jv_invalid_with_msg(jv_string("Malformed BOM"));
  jv value;
  presult msg = 0;
  while (!msg && p->curr_buf_pos < p->curr_buf_length) {
    char ch = p->curr_buf[p->curr_buf_pos++];
    msg = scan(p, ch, &value);
  }
  if (msg == OK) {
    return value;
  } else if (msg) {
    return jv_invalid_with_msg(jv_string_fmt("%s at line %d, column %d", msg, p->line, p->column));
  } else if (p->curr_buf_is_partial) {
    assert(p->curr_buf_pos == p->curr_buf_length);
    // need another buffer
    return jv_invalid();
  } else {
    assert(p->curr_buf_pos == p->curr_buf_length);
    // at EOF
    if (p->st != JV_PARSER_NORMAL) 
      return jv_invalid_with_msg(jv_string("Unfinished string"));
    if ((msg = check_literal(p)))
      return jv_invalid_with_msg(jv_string(msg));
    if (p->stackpos != 0)
      return jv_invalid_with_msg(jv_string("Unfinished JSON term"));
    // p->next is either invalid (nothing here but no syntax error)
    // or valid (this is the value). either way it's the thing to return
    value = p->next;
    p->next = jv_invalid();
    return value;
  }
}

jv jv_parse_sized(const char* string, int length) {
  struct jv_parser parser;
  jv_parser_init(&parser);
  jv_parser_set_buf(&parser, string, length, 0);
  jv value = jv_parser_next(&parser);
  if (jv_is_valid(value)) {
    jv next = jv_parser_next(&parser);
    if (jv_is_valid(next)) {
      // multiple JSON values, we only wanted one
      jv_free(value);
      jv_free(next);
      value = jv_invalid_with_msg(jv_string("Unexpected extra JSON values"));
    } else if (jv_invalid_has_msg(jv_copy(next))) {
      // parser error after the first JSON value
      jv_free(value);
      value = next;
    } else {
      // a single valid JSON value
      jv_free(next);
    }
  } else if (jv_invalid_has_msg(jv_copy(value))) {
    // parse error, we'll return it
  } else {
    // no value at all
    jv_free(value);
    value = jv_invalid_with_msg(jv_string("Expected JSON value"));
  }
  jv_parser_free(&parser);

  if (!jv_is_valid(value) && jv_invalid_has_msg(jv_copy(value))) {
    jv msg = jv_invalid_get_msg(value);
    value = jv_invalid_with_msg(jv_string_fmt("%s (while parsing '%s')",
                                              jv_string_value(msg),
                                              string));
    jv_free(msg);
  }
  return value;
}

jv jv_parse(const char* string) {
  return jv_parse_sized(string, strlen(string));
}
