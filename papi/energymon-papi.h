/**
 * Read energy from PAPI.
 *
 * @author Connor Imes
 */
#ifndef _ENERGYMON_PAPI_H_
#define _ENERGYMON_PAPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stddef.h>
#include "energymon.h"

int energymon_init_papi(energymon* em);

uint64_t energymon_read_total_papi(const energymon* em);

int energymon_finish_papi(energymon* em);

char* energymon_get_source_papi(char* buffer, size_t n);

uint64_t energymon_get_interval_papi(const energymon* em);

int energymon_get_papi(energymon* em);

#ifdef __cplusplus
}
#endif

#endif
