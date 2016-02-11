/**
 * Read energy from PAPI.
 *
 * @author Connor Imes
 */

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <papi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "energymon.h"
#include "energymon-papi.h"
#include "energymon-util.h"

#ifdef ENERGYMON_DEFAULT
#include "energymon-default.h"
int energymon_get_default(energymon* em) {
  return energymon_get_papi(em);
}
#endif

typedef struct energymon_papi {
  int event_set;
  long long* values;
} energymon_papi;



int energymon_init_papi(energymon* em) {
  if (em == NULL || em->state != NULL) {
    errno = EINVAL;
    return -1;
  }

  if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) {
    fprintf(stderr, "PAPI library init failed\n");
    return -1;
  }

  int numcmp = PAPI_num_components();
  int numhwctrs;
  const PAPI_component_info_t *cmpinfo;
  int i, j;
  for (i = 0; i < numcmp; i++) {
    if ((cmpinfo = PAPI_get_component_info(i)) == NULL) {
      fprintf(stderr, "NULL for component info %d\n", i);
      continue;
    }
    numhwctrs = PAPI_num_cmp_hwctrs(i);
    printf("Component name: %s, num hw cts = %d\n", cmpinfo->name, numhwctrs);
    for (j = 0; j < numhwctrs; j++) {
      
    }
  }

  energymon_papi* state = malloc(sizeof(energymon_papi));
  if (state == NULL) {
    return -1;
  }

  em->state = state;
  errno = 0;

  return 0;
}

uint64_t energymon_read_total_papi(const energymon* em) {
  if (em == NULL || em->state == NULL) {
    errno = EINVAL;
    return 0;
  }

  return 0;
}

int energymon_finish_papi(energymon* em) {
  if (em == NULL || em->state == NULL) {
    errno = EINVAL;
    return -1;
  }

  free(em->state);
  em->state = NULL;

  return 0;
}

char* energymon_get_source_papi(char* buffer, size_t n) {
  return energymon_strencpy(buffer, "PAPI", n);
}

uint64_t energymon_get_interval_papi(const energymon* em) {
  if (em == NULL) {
    // we don't need to access em, but it's still a programming error
    errno = EINVAL;
    return 0;
  }
  return 1;
}

int energymon_get_papi(energymon* em) {
  if (em == NULL) {
    errno = EINVAL;
    return -1;
  }
  em->finit = &energymon_init_papi;
  em->fread = &energymon_read_total_papi;
  em->ffinish = &energymon_finish_papi;
  em->fsource = &energymon_get_source_papi;
  em->finterval = &energymon_get_interval_papi;
  em->state = NULL;
  return 0;
}
