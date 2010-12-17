/** @file */

#ifndef __SPAMM_TIMER_H
#define __SPAMM_TIMER_H

#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

/** The timer type.
 */
enum spamm_timer_type_t
{
  /** The walltime passed. */
  walltime,

  /** Instructions. */
  total_instructions
};

struct spamm_timer_t *
spamm_timer_new (const enum spamm_timer_type_t type);

void
spamm_timer_delete (struct spamm_timer_t **timer);

void
spamm_timer_start (struct spamm_timer_t *timer);

void
spamm_timer_stop (struct spamm_timer_t *timer);

unsigned int
spamm_timer_get (const struct spamm_timer_t *timer);

#endif
