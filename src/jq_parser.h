#ifndef JQ_PARSER_H
#define JQ_PARSER_H
#include "locfile.h"
#include "compile.h"

#ifdef __cplusplus
extern "C" {
#endif

int jq_parse(struct locfile* source, block* answer);
int jq_parse_library(struct locfile* locations, block* answer);

#ifdef __cplusplus
}
#endif

#endif
