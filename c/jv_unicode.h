#ifndef JV_UNICODE_H
#define JV_UNICODE_H
int jvp_utf8_decode_length(char startchar);

int jvp_utf8_encode_length(int codepoint);
int jvp_utf8_encode(int codepoint, char* out);
#endif
