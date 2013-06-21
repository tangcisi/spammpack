#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <sstream>

#include "utilities.h"

void logging::log (const int level,
    const char *const filename,
    const int line,
    const char *const format, ...)
{
  va_list va;
  std::string levelname;

  switch(level)
  {
    case logging::INFO:
      levelname = "INFO";
      break;

    case logging::ERROR:
      levelname = "ERROR";
      break;

    default:
      levelname = "UNKNOWN";
      break;
  };

  std::ostringstream format_stream;
  format_stream << "[" << filename << ":" << line << " - " << levelname << "] " << format;
  const char *new_format = format_stream.str().c_str();

  va_start(va, format);
  vprintf(new_format, va);
  va_end(va);
}
