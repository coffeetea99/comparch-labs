//------------------------------------------------------------------------------
// 4190.308                     Computer Architecture                Spring 2019
//
// Cache Simulator Lab
//
// File: cache.c
//
// (C) 2015 Computer Systems and Platforms Laboratory, Seoul National University
//
// Changelog
// 20151119   bernhard    created
// 20190522   ca_ta       modified

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "cache.h"

extern char RP_STR[3][32];
extern char WP_STR[2][20];

Cache* create_cache(uint32_t capacity, uint32_t blocksize, uint32_t ways,
                    REP_POLICY rp, WRITE_POLICY wp, uint32_t verbosity)
{

	uint32_t valid = 1;
	// TODO
	//
	// 1. check cache parameters
	//    - capacity, blocksize, and ways must be powers of 2
	//    - capacity must be larger than set size

	//check capacity power of 2
	uint32_t tempCapacity = capacity;
	while(tempCapacity > 1) {
		if ( tempCapacity % 2 == 1 ) valid = 0;
		tempCapacity /= 2;
	}

	//check blocksize power of 2
	uint32_t tempBlocksize = blocksize;
	uint32_t offsetBits = 0;
	while(tempBlocksize > 1) {
		if ( tempBlocksize % 2 == 1 ) valid = 0;
		tempBlocksize /= 2;
		++offsetBits;
	}

	//check capacity is bigger than size
	if ( capacity <= blocksize ) valid = 0;
	//if invalid return null
	if ( valid == 0 ) return NULL;

  // 2. allocate cache and initialize them
  //    - use the above data structures Cache, Set, and Line

	uint32_t totalBlocks = capacity / blocksize;
	uint32_t setNum;
	if ( totalBlocks % ways != 0 ) return NULL;
	else setNum = totalBlocks / ways;

	//allocate memory
	Cache* retCache = malloc (sizeof(Cache));
	retCache->set = malloc(sizeof(Set) * setNum);
	int i;
	for ( i = 0 ; i < setNum ; ++i ) {
		retCache->set[i].way = malloc(sizeof(Line) * ways);
		retCache->set[i].validLines = 0;
		retCache->set[i].roundRobin = 0;
		int j;
		for ( j = 0 ; j < ways ; ++j ) {
			retCache->set[i].way[j].valid = 0;
		}
	}

	//calculate set bits and block bits
	int tempSetNum = setNum;
	uint32_t indexBits = 0;
	while(tempSetNum > 1) {
		tempSetNum /= 2;
		++indexBits;
	}

	//set values on cache
	retCache->s_access = 0;
	retCache->s_hit = 0;
	retCache->s_miss = 0;
	retCache->s_evict = 0;

	retCache->rep_policy = rp;
	retCache->write_policy = wp;

	retCache->setNum = setNum;
	retCache->indexBits = indexBits;
	retCache->wayNum = ways;
	retCache->offsetBits = offsetBits;
	retCache->time = 0;
	
	// 3. print cache configuration
	printf("Cache configuration:\n"
         "  capacity:        %6u\n"
         "  blocksize:       %6u\n"
         "  ways:            %6u\n"
         "  sets:            %6u\n"
         "  tag shift:       %6u\n"
         "  replacement:     %s\n"
         "  on write miss:   %s\n"
         "\n",
         capacity, blocksize, ways, setNum, indexBits + offsetBits, RP_STR[rp], WP_STR[wp]);

	// 4. return cache
	return retCache;
}

void delete_cache(Cache *c)
{
	// TODO
	//
	// clean-up the allocated memory

	uint32_t setNum = c->setNum;
	int i;
	for ( i = 0 ; i < setNum ; ++i ) {
		free(c->set[i].way);
	}
	free(c->set);
	free(c);
}

void line_access(Cache *c, Line *l)
{
	// TODO
	//
	// update data structures to reflect access to a cache line
}


void line_alloc(Cache *c, Line *l, uint32_t tag)
{
	// TODO
	//
	// update data structures to reflect allocation of a new block into a line
}

uint32_t set_find_victim(Cache *c, Set *s)
{
	// TODO
	//
	// for a given set, return the victim line where to place the new block

	uint32_t ret = 0;
	uint32_t wayNum = c->wayNum;
	if ( s->validLines == wayNum ) {				//if the set is full
		switch (c->rep_policy)
		{
		case RP_RR:										//round-robin
			ret = s->roundRobin;
			break;
		case RP_RANDOM:										//random
			ret = rand() % wayNum;
			break;
		case RP_LRU:										//least-recently used
			{	
				uint32_t smallestTime = 4294967295;
				int i;
				for ( i = 0 ; i < wayNum ; ++i ) {
					if ( s->way[i].time < smallestTime ) {
						smallestTime = s->way[i].time;
						ret = i;
					}
				}
				break;
			}
		}
	} else {										//the set is not full
		switch (c->rep_policy)
		{
		case RP_RR:										//round-robin
			ret = s->roundRobin;
			break;
		case RP_RANDOM:										//random
			ret = rand() % wayNum;
			while(s->way[ret].valid == 1) {
				ret = ( ret + 1 ) % wayNum;
			}
			break;
		case RP_LRU:										//least-recently used
			{
				int i;
				for ( i = 0 ; i < wayNum ; ++i ) {
					if ( s->way[i].valid == 0 ) {
						ret = i;
						break;
					}
				}
				break;
			}
		}
	}

	return ret;
}

void cache_access(Cache *c, uint32_t type, uint32_t address, uint32_t length)
{
	// TODO
	//
	// simulate a cache access

	// 1. compute set & tag

	uint32_t indexBits = c->indexBits;
	uint32_t offsetBits = c->offsetBits;
	uint32_t wayNum = c->wayNum;
	uint32_t tagBits = 32 - indexBits - offsetBits;

	uint32_t index = address >> offsetBits;
	uint32_t tag = index >> indexBits;

	index &= (c->setNum - 1);

	uint32_t orTag = 0;
	int i;
	for ( i = 0 ; i < tagBits ; ++i ) {
		orTag <<= 1;
		orTag++;
	}
	tag &= orTag;

	// 2. check if we have a cache hit

	int32_t hit = 0;
	Set* targetSet = &( c->set[index] );
	for ( i = 0 ; i < wayNum ; ++i ) {
		if ( targetSet->way[i].valid && targetSet->way[i].tag == tag ) {			//hit!!
			targetSet->way[i].time = c->time;
			hit = 1;
			break;
		}
	}

	// 3. on a cache miss, find a victim block and allocate according to the
	//    current policies
	
	uint32_t evicted = 0;
	if ( hit == 0 ) {
		if ( type == 0 || (type == 1 && c->write_policy == WP_WRITEALLOC) ) {

			Line* victimLine = & ( targetSet->way[set_find_victim(c, targetSet)]);		//find victim

			victimLine->valid = 1;
			victimLine->tag = tag;
			victimLine->time = c->time;

			if ( targetSet->validLines < wayNum ) {
				++(targetSet->validLines);
				evicted = 0;
			} else {
				evicted = 1;
			}
			targetSet->roundRobin = ( targetSet->roundRobin +1 ) % wayNum;

		}
	}

	// 4. update statistics (# accesses, # hits, # misses)

	++(c->s_access);

	if ( hit == 1 ) ++(c->s_hit);
	else ++(c->s_miss);

	if ( evicted ) ++(c->s_evict);

	++(c->time);

}
