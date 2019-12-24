//------------------------------------------------------------------------------
// 4190.308                     Computer Architecture                Spring 2019
//
// Cache Simulator Lab
//
// File: cachesim..c
//
// (C) 2015 Computer Systems and Platforms Laboratory, Seoul National University
//
// Changelog
// 20151119   bernhard    created
// 20190522   ca_ta       modified

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>
#include "cache.h"

uint32_t capacity = 0;
uint32_t blocksize = 0;
uint32_t ways = 0;
REP_POLICY rp = 0;  // replacement policy
WRITE_POLICY wp = 0;  // write allocate policy
uint32_t verbosity = 0;

char RP_STR[3][32] = {
  "round robin", "random", "LRU (least-recently used)"
};

char WP_STR[2][20] = {
  "write-allocate", "no write-allocate"
};

void syntax(void)
{
  uint32_t i;

  printf("cachesim: a cache simulator.\n"
         "\n"
         "Usage: $ cachesim <options>\n"
         "where <options> is\n"
         "  -h / --help                  Show this help screen.\n"
         "  -v / --verbose               Be verbose while running.\n"
         "  -c / --capacity <number>     Set the cache capacity.\n"
         "  -b / --blocksize <number>    Set the block size.\n"
         "  -w / --ways <number>         Set the number of ways.\n"
         "  -r / --replacement <number>  Set the replacement strategy.\n");
  for (i=0; i<sizeof(RP_STR)/sizeof(RP_STR[0]); i++) {
    printf("%32s%2d  %s%s\n", "", i, RP_STR[i], (i==0?" (default)":""));
  }
  printf("  -W / --write <number>       Set the write strategy.\n");
  for (i=0; i<sizeof(WP_STR)/sizeof(WP_STR[0]); i++) {
    printf("%32s%2d  %s%s\n", "", i, WP_STR[i], (i==0?" (default)":""));
  }
  printf("\n");

  exit(EXIT_FAILURE);
}

int parse_arguments(int argc, char *argv[])
{
  int opt;
  int opt_index;

  struct option options[] = {
    {"help", 0, 0, 0},
    {"verbose", 0, 0, 0},
    {"capacity", 1, 0, 0},
    {"blocksize", 1, 0, 0},
    {"ways", 1, 0, 0},
    {"replacement", 1, 0, 0},
    {"write", 1, 0, 0},
    {0, 0, 0, 0}
  };

  while ((opt = getopt_long(argc, argv, "hvc:b:w:r:W:", options, &opt_index)) != -1) {
    switch ( opt ) {
      case 0 :  // long argument
        if ( strcmp(options[opt_index].name, "verbose") == 0 ) {
	      verbosity = 1;
	    }
        else if ( strcmp(options[opt_index].name, "capacity") == 0 ) {
	      capacity = atoi(optarg);
	    }
	    else if ( strcmp(options[opt_index].name, "blocksize") == 0 ) {
	      blocksize = atoi(optarg);
	    }
	    else if ( strcmp(options[opt_index].name, "ways") == 0 ) {
	      ways = atoi(optarg);
	    }
	    else if ( strcmp(options[opt_index].name, "replacement") == 0 ) {
	      rp = atoi(optarg);
	    }
	    else if ( strcmp(options[opt_index].name, "write") == 0 ) {
	      wp = atoi(optarg);
	    } else {
	      return EXIT_FAILURE;
	    }
	    break;
      // short argument
      case 'v' :
        verbosity = 1;
	    break;
      case 'c' :
        capacity = atoi(optarg);
	    break;
      case 'b' :
        blocksize = atoi(optarg);
	    break;
      case 'w' :
        ways = atoi(optarg);
	    break;
      case 'r' :
        rp = atoi(optarg);
	    break;
      case 'W' :
        wp = atoi(optarg);
	    break;
      case 'h' :
      default :
	    return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
  Cache *cache;

  // parse arguments
  if (parse_arguments(argc, argv) != EXIT_SUCCESS) {
    syntax();
  }

  // create cache
  cache = create_cache(capacity, blocksize, ways, rp, wp, verbosity);

  // set initial random seed
  srand(1522000800);

  // read input line by line
  char *type = NULL;
  uint32_t address, length;
  char *line = NULL;
  size_t ll = 0;
  ssize_t ilen = getline(&line, &ll, stdin);
  uint32_t count = 0;

  printf("Processing input..."); fflush(stdout);
  if (verbosity) printf("\n");

  while (ilen > 0) {
    if (sscanf(line, "%ms %x,%x", &type, &address, &length) == 3) {
      switch (type[0]) {
        // instruction
        case 'I':
          // ignore
          break;

        // memory load ('L') or load-store ('M')
        case 'M':
        case 'L':
          cache_access(cache, READ, address, length);
          if (type[0] == 'L') break;

        // memory store ('S') or load-store ('M')
        case 'S':
          cache_access(cache, WRITE, address, length);
          break;
      }
    }

    // read next line from input
    if (type != NULL) { free(type); type = NULL; }
    ilen = getline(&line, &ll, stdin);

    count++;
    if (!verbosity && (count % 32768 == 0)) { printf("."); fflush(stdout); }
  }

  // print statistics
  printf("\n\n"
         "Cache simulation statistics:\n"
         "  accesses:       %8d\n"
         "  hit:            %8d\n"
         "  miss:           %8d\n"
         "  evictions:      %8d\n"
         "  miss ratio:     %4.2f%%\n",
         cache->s_access, cache->s_hit, cache->s_miss, cache->s_evict,
         cache->s_access > 0 ? ((float)cache->s_miss)/cache->s_access*100 : 0.0);

  // free cache & exit
  delete_cache(cache);

  return EXIT_SUCCESS;
}
