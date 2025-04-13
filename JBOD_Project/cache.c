#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "cache.h"
#include "jbod.h"


//cache is a an array of cache_entry_t structs
static cache_entry_t *cache = NULL;
static int cache_size = 0;  
static int num_queries = 0;
static int num_hits = 0;
//keeps count of how many times the insert functon has been called 
//if the insert status is 1, 
static int current_entries = 0; 
//num entries is how much memory is being allocated  within our heap?
//is each cache considered its own individual entry?
 //


int cache_create(int num_entries) {
  //checks to see if the number of entries passed is less than 2 or greater than 4096
  if(num_entries < 2 || num_entries > 4096 ){
    printf("\tinvalid argument\n");
    return -1;
  }
  //cache has already been created in the heap
  if(cache_size > 0){
    printf("\tcannot call cache create twice, failing\n");
    return -1;
  }
  

  //dynamically allocate space for num_entries
  //what will the sizeof operator output for that data type?
  //malloc will allocate a specified amount of bytes within the heap, 
  //does malloc return a starting address
  cache = (cache_entry_t*)malloc(sizeof(cache_entry_t) * num_entries);
  cache_size = num_entries; //why is the cache size 
  //equal to the number of entries?
    return 1;
}

int cache_destroy(void) {
  //if there is no cache allocated in the heap there is nothing to destroy. 
  if(cache_size == 0){
    printf("\t nothing to destroy\n");
    return -1;
  }

  current_entries = 0;

  //delete the allocated space in heap
  free(cache);
  //setting the value within the pointer to NULL 
  cache = NULL; 
  cache_size = 0; 
  // c function call, deletes 
  //when passes by value 
  //when you pass a pointer to a function call 
  //we are passing the memory location 
  //

    return 0;
}

int cache_lookup(int disk_num, int block_num, uint8_t *buf) {
  if(disk_num < 0 || block_num < 0 || cache == NULL || buf == NULL || cache_size == 0){
    printf("\t disk number and or block number is not specified(not initalized) in lookup, failing \n");
    return -1 ;
  }

  num_queries++;

  for (int i = 0; i < current_entries; i++)
  {
      if (cache[i].disk_num == disk_num && cache[i].block_num == block_num && cache[i].valid == true)
      {
        memcpy(buf, cache[i].block, JBOD_BLOCK_SIZE);
        cache[i].num_accesses++;
        num_hits++;
        printf("\tlocated the cache entry\n");
        return 1;
      }
  }

  return -1;
}
//disk num and block num represents where the entry is in storage 
void cache_update(int disk_num, int block_num, const uint8_t *buf) {
  if(buf == NULL || cache == NULL || disk_num < 0 || block_num < 0 || disk_num > 16 || disk_num < 0 ||  block_num >= 256 || block_num < 0 ){
    return;
  }

  //loop through cache entries 
  for(int i = 0; i < cache_size; i++ ){
    if(cache[i].disk_num == disk_num && cache[i].block_num == block_num ){
      memcpy(cache[i].block, buf,JBOD_BLOCK_SIZE);
      printf("\tcache entry block is now updated\n");

      return;
    }


  }
}

int cache_insert(int disk_num, int block_num, const uint8_t *buf) {
  //the variable buffer is holding the memory address of what was passed to it

  if(buf == NULL || cache == NULL || disk_num < 0 || block_num < 0 || disk_num > 16 || disk_num < 0 ||  block_num >= 256 || block_num < 0 ){
    return -1; 
  }

  for(int i = 0; i < JBOD_BLOCK_SIZE; i++){
    
  }

  printf("\tat this point, all conditionals are complete, continuing code\n");
  //creation
  //why are we allocating memory in heap if we already did so in the create function?
  //entry = (cache_entry_t*)(sizeof(cache_entry_t));

  

// for insert function we must copy the buff array to the cache. 
  //struggling to create logic to memcpy each entry into cache. 
  //if the amount of entries are less than the capacity then..

// declare temporary buffer after if conditons 
// use lookup if current 
//parameters we must pass to lookup must be disknum and blocknum
//




//CURRENT PROBLEMS IN CODE:
  //must implement cache lookup in code
  //implement a temporary buffer in our insert function. 
  //implement a while loop?
  //incorrect logic within cache insert 
  //temporary buffer will be passed to cache lookup. 
  //need to understand what cache lookup does.

  //for every cache entry

  for (int i = 0; i < cache_size; i++)
  {
    //if the entry we want to insert is already in our cache 
    if (cache[i].disk_num == disk_num && cache[i].block_num == block_num && cache[i].valid == true)
    { //then fail
      printf("\tentry we want to insert is already in cache. Failed. \n");
      printf("contents of disk_num: %d, contents of block_num: %d, ", disk_num, block_num); 
      printf("contents of disk_num: %d, contents of block_num: %d, ", cache[i].disk_num, cache[i].block_num); 

      return -1;
    }
    
  }

  //if cache is not full, insert the next entry
  if(current_entries < cache_size){
    //insert the next entry
    //set an individual entry to valid
    cache[current_entries].valid = true; 
    //set entries disk number
    cache[current_entries].disk_num = disk_num;
    //set entries block number 
    cache[current_entries].block_num = block_num;
    //copying what we want to the cache block array in cache entry struct
    memcpy(cache[current_entries].block, buf, JBOD_BLOCK_SIZE);
    //resets the number of times an entry has been accessed to 1.
    cache[current_entries].num_accesses = 1;
    //variable that keeps track of the current entry we are at
    current_entries++;
    printf("\tcache not full, inserting\n"); 
    return 1; 

  } else { //if the cache is full, override least frequently used cache entry. 
    // will store the least frequently used num_access
    int smallest = cache[0].num_accesses;
    int entry_index = 0;

    //for each entry in cache 
    for(int i = 0; i < cache_size ; i++ ){
      //if an entry is the least frequently used 
      if(cache[i].num_accesses < smallest ){
        //store the smallest accessed count 
        smallest = cache[i].num_accesses;
        //stores current entry index
        entry_index = i;
        //new entry will override the least frequently used entry
      }
    }

    printf("least frequently used entry is %d\n", entry_index);

    printf("\tcache_size is full, now overriding\n");
    cache[entry_index].valid = true; 
    cache[entry_index].disk_num = disk_num;
    cache[entry_index].block_num = block_num;

    memcpy(cache[entry_index].block, buf, JBOD_BLOCK_SIZE);
    cache[entry_index].num_accesses = 1;

// declare temporary buffer after if conditons 
// 
  }

  return 1;
}

bool cache_enabled(void) {
	return cache != NULL && cache_size > 0;
}

void cache_print_hit_rate(void) {
	fprintf(stderr, "num_hits: %d, num_queries: %d\n", num_hits, num_queries);
	fprintf(stderr, "Hit rate: %5.1f%%\n", 100 * (float) num_hits / num_queries);
}
