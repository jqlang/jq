#include <stdio.h>
#include <stdlib.h>
#include "jv.h"
#include "jv_dtoa.h"
jv stack[1000];
int stackpos = 0;
jv next;
int hasnext;

typedef const char* presult;

#define TRY(x) do {presult msg__ = (x); if (msg__) return msg__; } while(0)
#ifdef __GNUC__
#define pfunc __attribute__((warn_unused_result)) presult
#else
#define pfunc presult
#endif



pfunc value(jv val) {
  if (hasnext) return "Expected separator between values";
  hasnext = 1;
  next = val;
  return 0;
}

void push(jv v) {
  stack[stackpos++] = v;
}

pfunc token(char ch) {
  switch (ch) {
  case '[':
    if (hasnext) return "Expected separator between values";
    push(jv_array());
    break;

  case '{':
    if (hasnext) return "Expected separator between values";
    push(jv_object());
    break;

  case ':':
    if (!hasnext) 
      return "Expected string key before ':'";
    if (stackpos == 0 || jv_get_kind(stack[stackpos-1]) != JV_KIND_OBJECT)
      return "':' not as part of an object";
    if (jv_get_kind(next) != JV_KIND_STRING)
      return "Object keys must be strings";
    push(next);
    hasnext = 0;
    break;

  case ',':
    if (!hasnext)
      return "Expected value before ','";
    if (stackpos == 0)
      return "',' not as part of an object or array";
    if (jv_get_kind(stack[stackpos-1]) == JV_KIND_ARRAY) {
      stack[stackpos-1] = jv_array_append(stack[stackpos-1], next);
      hasnext = 0;
    } else if (jv_get_kind(stack[stackpos-1]) == JV_KIND_STRING) {
      assert(stackpos > 1 && jv_get_kind(stack[stackpos-2]) == JV_KIND_OBJECT);
      stack[stackpos-2] = jv_object_set(stack[stackpos-2], stack[stackpos-1], next);
      stackpos--;
      hasnext = 0;
    } else {
      // this case hits on input like {"a", "b"}
      return "Objects must consist of key:value pairs";
    }
    break;

  case ']':
    if (stackpos == 0 || jv_get_kind(stack[stackpos-1]) != JV_KIND_ARRAY)
      return "Unmatched ']'";
    if (hasnext) {
      stack[stackpos-1] = jv_array_append(stack[stackpos-1], next);
      hasnext = 0;
    } else {
      if (jv_array_length(jv_copy(stack[stackpos-1])) != 0) {
        // this case hits on input like [1,2,3,]
        return "Expected another array element";
      }
    }
    hasnext = 1;
    next = stack[--stackpos];
    break;

  case '}':
    if (stackpos == 0)
      return "Unmatched '}'";
    if (hasnext) {
      if (jv_get_kind(stack[stackpos-1]) != JV_KIND_STRING)
        return "Objects must consist of key:value pairs";
      assert(stackpos > 1 && jv_get_kind(stack[stackpos-2]) == JV_KIND_OBJECT);
      stack[stackpos-2] = jv_object_set(stack[stackpos-2], stack[stackpos-1], next);
      stackpos--;
      hasnext = 0;
    } else {
      if (jv_get_kind(stack[stackpos-1]) != JV_KIND_OBJECT)
        return "Unmatched '}'";
      if (jv_object_length(jv_copy(stack[stackpos-1])) != 0)
        return "Expected another key-value pair";
    }
    hasnext = 1;
    next = stack[--stackpos];
    break;
  }
  return 0;
}


char tokenbuf[1000];
int tokenpos;
struct dtoa_context dtoa;

void tokenadd(char c) {
  tokenbuf[tokenpos++] = c;
}

int unhex4(char* hex) {
  int r = 0;
  for (int i=0; i<4; i++) {
    char c = *hex++;
    int n;
    if ('0' <= c && c <= '9') n = c - '0';
    else if ('a' <= c && c <= 'f') n = c - 'a' + 10;
    else if ('A' <= c && c <= 'F') n = c - 'A' + 10;
    r <<= 4;
    r |= n;
  }
  return r;
}

int utf8_encode(int codepoint, char* out) {
  assert(codepoint >= 0 && codepoint <= 0x10FFFF);
  char* start = out;
  if (codepoint <= 0x7F) {
    *out++ = codepoint;
  } else if (codepoint <= 0x7FF) {
    *out++ = 0xC0 + ((codepoint & 0x7C0) >> 6);
    *out++ = 0x80 + ((codepoint & 0x03F));
  } else if(codepoint <= 0xFFFF) {
    *out++ = 0xE0 + ((codepoint & 0xF000) >> 12);
    *out++ = 0x80 + ((codepoint & 0x0FC0) >> 6);
    *out++ = 0x80 + ((codepoint & 0x003F));
  } else {
    *out++ = 0xF0 + ((codepoint & 0x1C0000) >> 18);
    *out++ = 0x80 + ((codepoint & 0x03F000) >> 12);
    *out++ = 0x80 + ((codepoint & 0x000FC0) >> 6);
    *out++ = 0x80 + ((codepoint & 0x00003F));
  }
  return out - start;
}

pfunc found_string() {
  char* in = tokenbuf;
  char* out = tokenbuf;
  char* end = tokenbuf + tokenpos;
  
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
        unsigned long codepoint = unhex4(in);
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
        out += utf8_encode(codepoint, out);
        break;

      default:
        return "Invalid escape";
      }
    } else {
      *out++ = c;
    }
  }
  TRY(value(jv_string_sized(tokenbuf, out - tokenbuf)));
  tokenpos=0;
  return 0;
}

pfunc check_literal() {
  if (tokenpos == 0) return 0;

  const char* pattern = 0;
  int plen;
  jv v;
  switch (tokenbuf[0]) {
  case 't': pattern = "true"; plen = 4; v = jv_true(); break;
  case 'f': pattern = "false"; plen = 5; v = jv_false(); break;
  case 'n': pattern = "null"; plen = 4; v = jv_null(); break;
  }
  if (pattern) {
    if (tokenpos != plen) return "Invalid literal";
    for (int i=0; i<plen; i++) 
      if (tokenbuf[i] != pattern[i])
        return "Invalid literal";
    TRY(value(v));
  } else {
    // FIXME: better parser
    tokenbuf[tokenpos] = 0; // FIXME: invalid
    char* end = 0;
    double d = jvp_strtod(&dtoa, tokenbuf, &end);
    if (end == 0 || *end != 0)
      return "Invalid numeric literal";
    TRY(value(jv_number(d)));
  }
  tokenpos=0;
  return 0;
}

typedef enum {
  LITERAL,
  WHITESPACE,
  STRUCTURE,
  QUOTE,
  INVALID
} chclass;

chclass classify(char c) {
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


enum state {
  NORMAL,
  STRING,
  STRING_ESCAPE
};

enum state st = NORMAL;

pfunc scan(char ch) {
  if (st == NORMAL) {
    chclass cls = classify(ch);
    if (cls != LITERAL) {
      TRY(check_literal());
    }
    switch (cls) {
    case LITERAL:
      tokenadd(ch);
      break;
    case WHITESPACE:
      break;
    case QUOTE:
      st = STRING;
      break;
    case STRUCTURE:
      TRY(token(ch));
      break;
    case INVALID:
      return "Invalid character";
    }
  } else {
    if (ch == '"' && st == STRING) {
      TRY(found_string());
      st = NORMAL;
    } else {
      tokenadd(ch);
      if (ch == '\\' && st == STRING) {
        st = STRING_ESCAPE;
      } else {
        st = STRING;
      }
    }
  }
  return 0;
}

pfunc finish() {
  assert(st == NORMAL);
  TRY(check_literal());

  if (stackpos != 0)
    return "Unfinished JSON term";
  
  // this will happen on the empty string
  if (!hasnext)
    return "Expected JSON value";
  
  return 0;
}


int main(int argc, char* argv[]) {
  assert(argc == 2);
  jvp_dtoa_context_init(&dtoa);
  char* p = argv[1];
  char ch;
  while ((ch = *p++)) {
    presult msg = scan(ch);
    if (msg){
      printf("ERROR: %s\n", msg);
      return 1;
    }
  }
  presult msg = finish();
  if (msg) {
    printf("ERROR: %s\n", msg);
    return 1;
  }
  jvp_dtoa_context_free(&dtoa);
  jv_dump(next);
  printf("\n");
  return 0;
}
