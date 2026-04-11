/**
 * @file error_strings.c
 * @brief Implementation of the error code to string translation.
 * @author Abhin Parekadan Jose
 * @date 2024-06-01
 */

#include "error/error_strings.h"

const char *error_to_string(long code) {
  switch (code) {
/**
 * @inner
 * Internal X-Macro expansion to generate switch cases.
 */
#define X(val, str)                                                            \
  case val:                                                                    \
    return str;

    /* Expand the map defined in the header */
    STRING_MAP

#undef X
  default:
    return "UNKNOWN";
  }
}
