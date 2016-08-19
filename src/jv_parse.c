#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "jv.h"
#include "jv_dtoa.h"
#include "jv_unicode.h"
#include "jv_alloc.h"
#include "jv_dtoa.h"

typedef const char* presult;

#ifndef MAX_PARSING_DEPTH
#define MAX_PARSING_DEPTH (256)
#endif

#define TRY(x) do {presult msg__ = (x); if (msg__) return msg__; } while(0)
#ifdef __GNUC__
#define pfunc __attribute__((warn_unused_result)) presult
#else
#define pfunc presult
#endif

enum last_seen {
  JV_LAST_NONE = 0,
  JV_LAST_OPEN_ARRAY = '[',
  JV_LAST_OPEN_OBJECT = '{',
  JV_LAST_COLON = ':',
  JV_LAST_COMMA = ',',
  JV_LAST_VALUE = 'V',
};

struct jv_parser {
  const char* curr_buf;
  int curr_buf_length;
  int curr_buf_pos;
  int curr_buf_is_partial;
  int eof;
  unsigned bom_strip_position;

  int flags;

  jv* stack;                   // parser
  int stackpos;                // parser
  int stacklen;                // both (optimization; it's really pathlen for streaming)
  jv path;                     // streamer
  enum last_seen last_seen;    // streamer
  jv output;                   // streamer
  jv next;                     // both

  char* tokenbuf;
  int tokenpos;
  int tokenlen;

  int line, column;

  struct dtoa_context dtoa;

  enum {
    JV_PARSER_NORMAL,
    JV_PARSER_STRING,
    JV_PARSER_STRING_ESCAPE,
    JV_PARSER_WAITING_FOR_RS // parse error, waiting for RS
  } st;
  unsigned int last_ch_was_ws:1;
};


static void parser_init(struct jv_parser* p, int flags) {
  p->flags = flags;
  if ((p->flags & JV_PARSE_STREAMING)) {
    p->path = jv_array();
  } else {
    p->path = jv_invalid();
    p->flags &= ~(JV_PARSE_STREAM_ERRORS);
  }
  p->stack = 0;
  p->stacklen = p->stackpos = 0;
  p->last_seen = JV_LAST_NONE;
  p->output = jv_invalid();
  p->next = jv_invalid();
  p->tokenbuf = 0;
  p->tokenlen = p->tokenpos = 0;
  if ((p->flags & JV_PARSE_SEQ))
    p->st = JV_PARSER_WAITING_FOR_RS;
  else
    p->st = JV_PARSER_NORMAL;
  p->eof = 0;
  p->curr_buf = 0;
  p->curr_buf_length = p->curr_buf_pos = p->curr_buf_is_partial = 0;
  p->bom_strip_position = 0;
  p->last_ch_was_ws = 0;
  p->line = 1;
  p->column = 0;
  jvp_dtoa_context_init(&p->dtoa);
}

static void parser_reset(struct jv_parser* p) {
  if ((p->flags & JV_PARSE_STREAMING)) {
    jv_free(p->path);
    p->path = jv_array();
    p->stacklen = 0;
  }
  p->last_seen = JV_LAST_NONE;
  jv_free(p->output);
  p->output = jv_invalid();
  jv_free(p->next);
  p->next = jv_invalid();
  for (int i=0; i<p->stackpos; i++)
    jv_free(p->stack[i]);
  p->stackpos = 0;
  p->tokenpos = 0;
  p->st = JV_PARSER_NORMAL;
}

static void parser_free(struct jv_parser* p) {
  parser_reset(p);
  jv_free(p->path);
  jv_free(p->output);
  jv_mem_free(p->stack);
  jv_mem_free(p->tokenbuf);
  jvp_dtoa_context_free(&p->dtoa);
}

static pfunc value(struct jv_parser* p, jv val) {
  if ((p->flags & JV_PARSE_STREAMING)) {
    if (jv_is_valid(p->next) || p->last_seen == JV_LAST_VALUE)
      return "Expected separator between values";
    if (p->stacklen > 0)
      p->last_seen = JV_LAST_VALUE;
    else
      p->last_seen = JV_LAST_NONE;
  } else {
    if (jv_is_valid(p->next)) return "Expected separator between values";
  }
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

static pfunc parse_token(struct jv_parser* p, char ch) {
  switch (ch) {
  case '[':
    if (p->stackpos >= MAX_PARSING_DEPTH) return "Exceeds depth limit for parsing";
    if (jv_is_valid(p->next)) return "Expected separator between values";
    push(p, jv_array());
    break;

  case '{':
    if (p->stackpos >= MAX_PARSING_DEPTH) return "Exceeds depth limit for parsing";
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

static pfunc stream_token(struct jv_parser* p, char ch) {
  jv_kind k;
  jv last;

  switch (ch) {
  case '[':
    if (jv_is_valid(p->next))
      return "Expected a separator between values";
    p->path = jv_array_append(p->path, jv_number(0)); // push
    p->last_seen = JV_LAST_OPEN_ARRAY;
    p->stacklen++;
    break;

  case '{':
    if (p->last_seen == JV_LAST_VALUE)
      return "Expected a separator between values";
    // Push object key: null, since we don't know it yet
    p->path = jv_array_append(p->path, jv_null()); // push
    p->last_seen = JV_LAST_OPEN_OBJECT;
    p->stacklen++;
    break;

  case ':':
    if (p->stacklen == 0 || jv_get_kind(jv_array_get(jv_copy(p->path), p->stacklen - 1)) == JV_KIND_NUMBER)
      return "':' not as part of an object";
    if (!jv_is_valid(p->next) || p->last_seen == JV_LAST_NONE)
      return "Expected string key before ':'";
    if (jv_get_kind(p->next) != JV_KIND_STRING)
      return "Object keys must be strings";
    if (p->last_seen != JV_LAST_VALUE)
      return "':' should follow a key";
    p->last_seen = JV_LAST_COLON;
    p->path = jv_array_set(p->path, p->stacklen - 1, p->next);
    p->next = jv_invalid();
    break;

  case ',':
    if (p->last_seen != JV_LAST_VALUE)
      return "Expected value before ','";
    if (p->stacklen == 0)
      return "',' not as part of an object or array";
    last = jv_array_get(jv_copy(p->path), p->stacklen - 1);
    k = jv_get_kind(last);
    if (k == JV_KIND_NUMBER) {
      int idx = jv_number_value(last);

      if (jv_is_valid(p->next)) {
        p->output = JV_ARRAY(jv_copy(p->path), p->next);
        p->next = jv_invalid();
      }
      p->path = jv_array_set(p->path, p->stacklen - 1, jv_number(idx + 1));
      p->last_seen = JV_LAST_COMMA;
    } else if (k == JV_KIND_STRING) {
      if (jv_is_valid(p->next)) {
        p->output = JV_ARRAY(jv_copy(p->path), p->next);
        p->next = jv_invalid();
      }
      p->path = jv_array_set(p->path, p->stacklen - 1, jv_true()); // ready for another name:value pair
      p->last_seen = JV_LAST_COMMA;
    } else {
      assert(k == JV_KIND_NULL);
      // this case hits on input like {,}
      // make sure to handle input like {"a", "b"} and {"a":, ...}
      jv_free(last);
      return "Objects must consist of key:value pairs";
    }
    jv_free(last);
    break;

  case ']':
    if (p->stacklen == 0)
      return "Unmatched ']' at the top-level";
    if (p->last_seen == JV_LAST_COMMA)
      return "Expected another array element";
    if (p->last_seen == JV_LAST_OPEN_ARRAY)
      assert(!jv_is_valid(p->next));

    last = jv_array_get(jv_copy(p->path), p->stacklen - 1);
    k = jv_get_kind(last);
    jv_free(last);

    if (k != JV_KIND_NUMBER)
      return "Unmatched ']' in the middle of an object";
    if (jv_is_valid(p->next)) {
      p->output = JV_ARRAY(jv_copy(p->path), p->next, jv_true());
      p->next = jv_invalid();
    } else if (p->last_seen != JV_LAST_OPEN_ARRAY) {
      p->output = JV_ARRAY(jv_copy(p->path));
    }

    p->path = jv_array_slice(p->path, 0, --(p->stacklen)); // pop
    //assert(!jv_is_valid(p->next));
    jv_free(p->next);
    p->next = jv_invalid();

    if (p->last_seen == JV_LAST_OPEN_ARRAY)
      p->output = JV_ARRAY(jv_copy(p->path), jv_array()); // Empty arrays are leaves

    if (p->stacklen == 0)
      p->last_seen = JV_LAST_NONE;
    else
      p->last_seen = JV_LAST_VALUE;
    break;

  case '}':
    if (p->stacklen == 0)
      return "Unmatched '}' at the top-level";
    if (p->last_seen == JV_LAST_COMMA)
      return "Expected another key:value pair";
    if (p->last_seen == JV_LAST_OPEN_OBJECT)
      assert(!jv_is_valid(p->next));

    last = jv_array_get(jv_copy(p->path), p->stacklen - 1);
    k = jv_get_kind(last);
    jv_free(last);
    if (k == JV_KIND_NUMBER)
      return "Unmatched '}' in the middle of an array";

    if (jv_is_valid(p->next)) {
      if (k != JV_KIND_STRING)
        return "Objects must consist of key:value pairs";
      p->output = JV_ARRAY(jv_copy(p->path), p->next, jv_true());
      p->next = jv_invalid();
    } else {
      // Perhaps {"a":[]}
      if (p->last_seen == JV_LAST_COLON)
        // Looks like {"a":}
        return "Missing value in key:value pair";
      if (p->last_seen == JV_LAST_COMMA)
        // Looks like {"a":0,}
        return "Expected another key-value pair";
      if (p->last_seen == JV_LAST_OPEN_ARRAY)
        return "Unmatched '}' in the middle of an array";
      if (p->last_seen != JV_LAST_VALUE && p->last_seen != JV_LAST_OPEN_OBJECT)
        return "Unmatched '}'";
      if (p->last_seen != JV_LAST_OPEN_OBJECT)
        p->output = JV_ARRAY(jv_copy(p->path));
    }
    p->path = jv_array_slice(p->path, 0, --(p->stacklen)); // pop
    jv_free(p->next);
    p->next = jv_invalid();

    if (p->last_seen == JV_LAST_OPEN_OBJECT)
      p->output = JV_ARRAY(jv_copy(p->path), jv_object()); // Empty arrays are leaves

    if (p->stacklen == 0)
      p->last_seen = JV_LAST_NONE;
    else
      p->last_seen = JV_LAST_VALUE;
    break;
  }
  return 0;
}

static void tokenadd(struct jv_parser* p, char c) {
  assert(p->tokenpos <= p->tokenlen);
  if (p->tokenpos >= (p->tokenlen - 1)) {
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
        if (codepoint > 0x10FFFF)
          codepoint = 0xFFFD; // U+FFFD REPLACEMENT CHARACTER
        out += jvp_utf8_encode(codepoint, out);
        break;

      default:
        return "Invalid escape";
      }
    } else {
      if (c > 0 && c < 0x001f)
        return "Invalid string: control characters from U+0000 through U+001F must be escaped";
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
    p->tokenbuf[p->tokenpos] = 0;
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

static int parse_check_done(struct jv_parser* p, jv* out) {
  if (p->stackpos == 0 && jv_is_valid(p->next)) {
    *out = p->next;
    p->next = jv_invalid();
    return 1;
  } else {
    return 0;
  }
}

static int stream_check_done(struct jv_parser* p, jv* out) {
  if (p->stacklen == 0 && jv_is_valid(p->next)) {
    *out = JV_ARRAY(jv_copy(p->path),p->next);
    p->next = jv_invalid();
    return 1;
  } else if (jv_is_valid(p->output)) {
    if (jv_array_length(jv_copy(p->output)) > 2) {
      // At end of an array or object, necessitating one more output by
      // which to indicate this
      *out = jv_array_slice(jv_copy(p->output), 0, 2);
      p->output = jv_array_slice(p->output, 0, 1);      // arrange one more output
    } else {
      // No further processing needed
      *out = p->output;
      p->output = jv_invalid();
    }
    return 1;
  } else {
    return 0;
  }
}

static int parse_check_truncation(struct jv_parser* p) {
  return ((p->flags & JV_PARSE_SEQ) && !p->last_ch_was_ws && (p->stackpos > 0 || p->tokenpos > 0 || jv_get_kind(p->next) == JV_KIND_NUMBER));
}

static int stream_check_truncation(struct jv_parser* p) {
  jv_kind k = jv_get_kind(p->next);
  return (p->stacklen > 0 || k == JV_KIND_NUMBER || k == JV_KIND_TRUE || k == JV_KIND_FALSE || k == JV_KIND_NULL);
}

static int parse_is_top_num(struct jv_parser* p) {
  return (p->stackpos == 0 && jv_get_kind(p->next) == JV_KIND_NUMBER);
}

static int stream_is_top_num(struct jv_parser* p) {
  return (p->stacklen == 0 && jv_get_kind(p->next) == JV_KIND_NUMBER);
}

#define check_done(p, o) \
   (((p)->flags & JV_PARSE_STREAMING) ? stream_check_done((p), (o)) : parse_check_done((p), (o)))

#define token(p, ch) \
   (((p)->flags & JV_PARSE_STREAMING) ? stream_token((p), (ch)) : parse_token((p), (ch)))

#define check_truncation(p) \
   (((p)->flags & JV_PARSE_STREAMING) ? stream_check_truncation((p)) : parse_check_truncation((p)))

#define is_top_num(p) \
   (((p)->flags & JV_PARSE_STREAMING) ? stream_is_top_num((p)) : parse_is_top_num((p)))

static pfunc scan(struct jv_parser* p, char ch, jv* out) {
  p->column++;
  if (ch == '\n') {
    p->line++;
    p->column = 0;
  }
  if (ch == '\036' /* ASCII RS; see draft-ietf-json-sequence-07 */) {
    if (check_truncation(p)) {
      if (check_literal(p) == 0 && is_top_num(p))
        return "Potentially truncated top-level numeric value";
      return "Truncated value";
    }
    TRY(check_literal(p));
    if (p->st == JV_PARSER_NORMAL && check_done(p, out))
      return OK;
    // shouldn't happen?
    assert(!jv_is_valid(*out));
    parser_reset(p);
    jv_free(*out);
    *out = jv_invalid();
    return OK;
  }
  presult answer = 0;
  p->last_ch_was_ws = 0;
  if (p->st == JV_PARSER_NORMAL) {
    chclass cls = classify(ch);
    if (cls == WHITESPACE)
      p->last_ch_was_ws = 1;
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

struct jv_parser* jv_parser_new(int flags) {
  struct jv_parser* p = jv_mem_alloc(sizeof(struct jv_parser));
  parser_init(p, flags);
  p->flags = flags;
  return p;
}

void jv_parser_free(struct jv_parser* p) {
  parser_free(p);
  jv_mem_free(p);
}

static const unsigned char UTF8_BOM[] = {0xEF,0xBB,0xBF};

int jv_parser_remaining(struct jv_parser* p) {
  if (p->curr_buf == 0)
    return 0;
  return (p->curr_buf_length - p->curr_buf_pos);
}

void jv_parser_set_buf(struct jv_parser* p, const char* buf, int length, int is_partial) {
  assert((p->curr_buf == 0 || p->curr_buf_pos == p->curr_buf_length)
         && "previous buffer not exhausted");
  while (length > 0 && p->bom_strip_position < sizeof(UTF8_BOM)) {
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

static jv make_error(struct jv_parser*, const char *, ...) JV_PRINTF_LIKE(2, 3);

static jv make_error(struct jv_parser* p, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  jv e = jv_string_vfmt(fmt, ap);
  va_end(ap);
  if ((p->flags & JV_PARSE_STREAM_ERRORS))
    return JV_ARRAY(e, jv_copy(p->path));
  return jv_invalid_with_msg(e);
}

jv jv_parser_next(struct jv_parser* p) {
  if (p->eof)
    return jv_invalid();
  if (!p->curr_buf)
    return jv_invalid(); // Need a buffer
  if (p->bom_strip_position == 0xff) {
    if (!(p->flags & JV_PARSE_SEQ))
      return jv_invalid_with_msg(jv_string("Malformed BOM"));
    p->st =JV_PARSER_WAITING_FOR_RS;
    parser_reset(p);
  }
  jv value = jv_invalid();
  if ((p->flags & JV_PARSE_STREAMING) && stream_check_done(p, &value))
    return value;
  char ch;
  presult msg = 0;
  while (!msg && p->curr_buf_pos < p->curr_buf_length) {
    ch = p->curr_buf[p->curr_buf_pos++];
    if (p->st == JV_PARSER_WAITING_FOR_RS) {
      if (ch == '\n') {
        p->line++;
        p->column = 0;
      } else {
        p->column++;
      }
      if (ch == '\036')
        p->st = JV_PARSER_NORMAL;
      continue; // need to resync, wait for RS
    }
    msg = scan(p, ch, &value);
  }
  if (msg == OK) {
    return value;
  } else if (msg) {
    jv_free(value);
    if (ch != '\036' && (p->flags & JV_PARSE_SEQ)) {
      // Skip to the next RS
      p->st = JV_PARSER_WAITING_FOR_RS;
      value = make_error(p, "%s at line %d, column %d (need RS to resync)", msg, p->line, p->column);
      parser_reset(p);
      return value;
    }
    value = make_error(p, "%s at line %d, column %d", msg, p->line, p->column);
    parser_reset(p);
    if (!(p->flags & JV_PARSE_SEQ)) {
      // We're not parsing a JSON text sequence; throw this buffer away.
      // XXX We should fail permanently here.
      p->curr_buf = 0;
      p->curr_buf_pos = 0;
    } // Else ch must be RS; don't clear buf so we can start parsing again after this ch
    return value;
  } else if (p->curr_buf_is_partial) {
    assert(p->curr_buf_pos == p->curr_buf_length);
    // need another buffer
    return jv_invalid();
  } else {
    // at EOF
    p->eof = 1;
    assert(p->curr_buf_pos == p->curr_buf_length);
    jv_free(value);
    if (p->st == JV_PARSER_WAITING_FOR_RS)
      return make_error(p, "Unfinished abandoned text at EOF at line %d, column %d", p->line, p->column);
    if (p->st != JV_PARSER_NORMAL) {
      value = make_error(p, "Unfinished string at EOF at line %d, column %d", p->line, p->column);
      parser_reset(p);
      p->st = JV_PARSER_WAITING_FOR_RS;
      return value;
    }
    if ((msg = check_literal(p))) {
      value = make_error(p, "%s at EOF at line %d, column %d", msg, p->line, p->column);
      parser_reset(p);
      p->st = JV_PARSER_WAITING_FOR_RS;
      return value;
    }
    if (((p->flags & JV_PARSE_STREAMING) && p->stacklen != 0) ||
        (!(p->flags & JV_PARSE_STREAMING) && p->stackpos != 0)) {
      value = make_error(p, "Unfinished JSON term at EOF at line %d, column %d", p->line, p->column);
      parser_reset(p);
      p->st = JV_PARSER_WAITING_FOR_RS;
      return value;
    }
    // p->next is either invalid (nothing here, but no syntax error)
    // or valid (this is the value). either way it's the thing to return
    if ((p->flags & JV_PARSE_STREAMING) && jv_is_valid(p->next)) {
      value = JV_ARRAY(jv_copy(p->path), p->next); // except in streaming mode we've got to make it [path,value]
    } else {
      value = p->next;
    }
    p->next = jv_invalid();
    if ((p->flags & JV_PARSE_SEQ) && !p->last_ch_was_ws && jv_get_kind(value) == JV_KIND_NUMBER) {
      jv_free(value);
      return make_error(p, "Potentially truncated top-level numeric value at EOF at line %d, column %d", p->line, p->column);
    }
    return value;
  }
}

jv jv_parse_sized(const char* string, int length) {
  struct jv_parser parser;
  parser_init(&parser, 0);
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
  parser_free(&parser);

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
