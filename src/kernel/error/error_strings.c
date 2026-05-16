/**
 * @file error_strings.c
 * @brief Implementation of error code to string translation.
 *
 * Provides the run-time mapping from numeric kernel error codes to human-
 * readable messages used by diagnostic subsystems.
 *
 * @author Abhin Parekadan Jose
 * @date 2026-04-11
 */

#include "error/error_strings.h"

const char *error_to_string(long code)
{
	switch (code) {
/**
 * @note Internal X-Macro expansion to generate switch cases.
 */
#define X(val, str) \
	case val:   \
		return str;

		/* Expand the map defined in the header */
		STRING_MAP

#undef X
	default:
		return "UNKNOWN";
	}
}
