/**
 * Polls an energymon at the requested interval and prints the average power
 * over that interval.
 *
 * @author Connor Imes
 * @date 2016-08-30
 */
#define _GNU_SOURCE
#include <errno.h>
#include <float.h>
#include <getopt.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "energymon-default.h"
#include "energymon-time-util.h"

static const int IGNORE_INTERRUPT = 0;

static volatile uint64_t running = 1;
static int count = 0;
static const char* filename = NULL;
static int summarize = 0;
static uint64_t interval = 0;

static const char short_options[] = "hc:f:i:s";
static const struct option long_options[] = {
  {"help",      no_argument,       NULL, 'h'},
  {"count",     required_argument, NULL, 'c'},
  {"file",      required_argument, NULL, 'f'},
  {"interval",  required_argument, NULL, 'i'},
  {"summarize", no_argument,       NULL, 's'},
  {0, 0, 0, 0}
};

static void print_usage(int exit_code) {
  fprintf(exit_code ? stderr : stdout,
          "Usage: energymon-power-poller [OPTION]...\n"
          "Options:\n"
          "  -h, --help               Print this message and exit\n"
          "  -c, --count=N            Stop after N reads\n"
          "  -f, --file=FILE          The output file\n"
          "  -i, --interval=US        The update interval in microseconds\n"
          "  -s, --summarize          Print out a summary at completion\n");
  exit(exit_code);
}

static void parse_args(int argc, char** argv) {
  int c;
  while ((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
    switch (c) {
      case 'h':
        print_usage(0);
        break;
      case 'c':
        count = 1;
        running = strtoull(optarg, NULL, 0);
        break;
      case 'f':
        filename = optarg;
        break;
      case 'i':
        interval = strtoull(optarg, NULL, 0);
        if (interval == 0) {
          fprintf(stderr, "Interval must be > 0\n");
          print_usage(1);
        }
        break;
      case 's':
        summarize = 1;
        break;
      case '?':
      default:
        print_usage(1);
        break;
    }
  }
}

static void shandle(int sig) {
  switch (sig) {
    case SIGTERM:
    case SIGINT:
#ifdef SIGQUIT
    case SIGQUIT:
#endif
#ifdef SIGKILL
    case SIGKILL:
#endif
#ifdef SIGHUP
    case SIGHUP:
#endif
      running = 0;
    default:
      break;
  }
}

int main(int argc, char** argv) {
  energymon em;
  uint64_t min_interval;
  uint64_t energy;
  uint64_t energy_last;
  float power;
  uint64_t n = 0;
  float pmin = FLT_MAX;
  float pmax = FLT_MIN;
  float pavg = 0;
  float pavg_last = 0;
  float pstd = 0;
  FILE* fout = stdout;
  int ret = 0;

  // register the signal handler
  signal(SIGINT, shandle);

  parse_args(argc, argv);

  // initialize the energy monitor
  if (energymon_get_default(&em)) {
    perror("energymon_get_default");
    return 1;
  }
  if (em.finit(&em)) {
    perror("energymon:finit");
    return 1;
  }

  // get the update interval
  min_interval = em.finterval(&em);
  if (interval == 0) {
    interval = min_interval;
  } else if (interval < min_interval) {
    fprintf(stderr,
            "Requested interval is too short, minimum available: %"PRIu64"\n",
            min_interval);
    em.ffinish(&em);
    return 1;
  }

  // open the output file
  if (filename != NULL) {
    fout = fopen(filename, "w");
    if (fout == NULL) {
      perror(filename);
      em.ffinish(&em);
      return 1;
    }
  }

  // output at regular intervals
  energy_last = em.fread(&em);
  energymon_sleep_us(interval, &IGNORE_INTERRUPT);
  while (running) {
    if (count) {
      running--;
    }
    energy = em.fread(&em);
    power = (energy - energy_last) / ((float) interval);
    if (fprintf(fout, "%f\n", power) < 0) {
      ret = 1;
      if (filename == NULL) {
        // should never happen writing to stdout
        perror("Writing");
      } else {
        perror(filename);
      }
      break;
    }
    fflush(fout);
    energy_last = energy;
    if (power > pmax) {
      pmax = power;
    }
    if (power < pmin) {
      pmin = power;
    }
    // get the running average and data to compute stddev
    pavg_last = pavg;
    n++;
    if (n == 1) {
      pavg = power;
    } else {
      pavg = pavg + ((power - pavg) / n);
    }
    pstd = pstd + (power - pavg) * (power - pavg_last);
    if (running) {
      energymon_sleep_us(interval, &IGNORE_INTERRUPT);
    }
  }

  if (summarize) {
    fprintf(fout, "Samples: %"PRIu64"\n", n);
    fprintf(fout, "Pavg: %f\n", pavg);
    fprintf(fout, "Pmax: %f\n", pmax);
    fprintf(fout, "Pmin: %f\n", pmin);
    fprintf(fout, "Pstdev: %f\n", n == 1 ? 0 : (sqrt(pstd / (n - 1))));
    fprintf(fout, "Joules: %f\n", n * pavg * (interval / 1000000.0f));
  }

  // cleanup
  if (filename != NULL && fclose(fout)) {
    perror(filename);
  }
  if (em.ffinish(&em)) {
    perror("em.ffinish");
  }

  return ret;
}
