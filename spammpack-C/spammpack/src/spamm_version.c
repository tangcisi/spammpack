#include "spamm_config.h"
#include "spamm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Return the version string of this library.
 *
 * @return The version string. This string is allocated with malloc() and the
 * caller is responsible for free()ing the string.
 */
char *
spamm_version ()
{
  char *string = (char *) malloc(sizeof(char)*strlen(VERSION)+1+strlen(COMMIT_TAG)+1);

  sprintf(string, "%s:%s", VERSION, COMMIT_TAG);
  return string;
}
