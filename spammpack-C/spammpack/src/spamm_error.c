#include "spamm_error.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Print out a warning message.
 *
 * @param filename The filename this error message was generated in.
 * @param line The line number this error message was generated on.
 * @param format The format of the error message.
 */
void
spamm_error_warning (const char *const filename, const int line, ...)
{
  va_list va;
  int format_length;

  char *old_format;
  char *new_format;

  /* Initialize variadic argument list. */
  va_start(va, line);

  /* Get format string. */
  old_format = va_arg(va, char*);

  format_length = 6+strlen(filename)+14+strlen(old_format);
  new_format = calloc(format_length, sizeof(char));
  snprintf(new_format, format_length, "[%s:%i WARNING] ", filename, line);
  strncat(new_format, old_format, format_length);

  /* Print error. */
  vprintf(new_format, va);

  /* Cleanup. */
  va_end(va);
  free(new_format);
}

/** Print out a fatal error message.
 *
 * @param filename The filename this error message was generated in.
 * @param line The line number this error message was generated on.
 * @param format The format of the error message.
 */
void
spamm_error_fatal (const char *const filename, const int line, ...)
{
  va_list va;
  int format_length;

  char *old_format;
  char *new_format;

  /* Initialize variadic argument list. */
  va_start(va, line);

  /* Get format string. */
  old_format = va_arg(va, char*);

  format_length = 6+strlen(filename)+14+strlen(old_format);
  new_format = calloc(format_length, sizeof(char));
  snprintf(new_format, format_length, "[%s:%i FATAL] ", filename, line);
  strncat(new_format, old_format, format_length);

  /* Print error. */
  vprintf(new_format, va);
  va_end(va);

  /* Cleanup. */
  free(new_format);

  /* Exit. */
  exit(1);
}