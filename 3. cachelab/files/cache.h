//------------------------------------------------------------------------------
// 4190.308                     Computer Architecture                Spring 2019
//
// Cache Simulator
//
// File: cache.h
//
// (C) 2015 Computer Systems and Platforms Laboratory, Seoul National University
//
// Changelog
// 20151119   bernhard    created
// 20190522   ca_ta       modified

#ifndef __CACHE_H__
#define __CACHE_H__

#include <stdint.h>

// ISPOW2: returns 1 if x is a power of 2, 0 otherwise
#define ISPOW2(x) ((x != 0) && !(x & (x-1)))

// access types
typedef enum {
	READ = 0,	// read access
	WRITE		// write access
} ACCESS_TYPE;

// replacement policies
typedef enum {
	RP_RR = 0,	// round robin
	RP_RANDOM,	// random
	RP_LRU	// least-recently used
} REP_POLICY;

// write allocate policies
typedef enum {
	WP_WRITEALLOC = 0,	// write-allocate
	WP_NOWRITEALLOC	// no write-allocate
} WRITE_POLICY;

// Line: one cache line
// (hint: for the simulation you do not need to store the actual data in the
//        cache, the tag and management information is sufficient)
typedef struct __line {
	// TODO
	// if necessary add more fields
	uint32_t valid;
	uint32_t tag;		//WARNING : not initialized!!
	uint32_t time;		//WARNING : not initialized!!
} Line;

// Set: one set of the cache
typedef struct __set {
	Line  *way;                         // cache lines

	// TODO
	// if necessary add more fields

	uint32_t validLines;
	uint32_t roundRobin;
} Set;

// Cache: the cache
typedef struct __cache {
	Set   *set;                         // cache sets

	uint32_t s_access;                    // statistics: number of accesses
	uint32_t s_hit;                       // statistics: number of hits
	uint32_t s_miss;                      // statistics: number of misses
	uint32_t s_evict;                     // statistics: number of evictions

	// TODO
	// if necessary add more fields

	REP_POLICY rep_policy;
	WRITE_POLICY write_policy;

	uint32_t setNum;
	uint32_t indexBits;
	uint32_t wayNum;
	uint32_t offsetBits;
	uint32_t time;
} Cache;

//
// create/delete a cache
//
// create initializes the data structure of a cache given its capacity,
// blocksize, number of ways, replacement policy and write-allocate policy
//
// delete frees the memory of a cache (lines, sets, and the cache itself)
//
Cache* create_cache(uint32_t capacity, uint32_t blocksize, uint32_t ways,
                    REP_POLICY rp, WRITE_POLICY wp, uint32_t verbosity);
void delete_cache(Cache *c);


//
// simulate access to a cache line
//
void line_access(Cache *c, Line *l);

//
// allocate a tag into a given cache line
//
void line_alloc(Cache *c, Line *l, uint32_t tag);

//
// find a victim line for a given cache set
//
uint32_t set_find_victim(Cache *c, Set *s);

//
// simulate a cache access
//
// parameters
//   type         READ/WRITE
//   address      requested address
//   length       length of the access
//
void cache_access(Cache *c, uint32_t type, uint32_t address, uint32_t length);

#endif // __CACHE_H__
