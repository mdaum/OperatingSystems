/* Tar Heels Allocator
 *
 * Simple Hoard-style malloc/free implementation.
 * Not suitable for use for large allocatoins, or
 * in multi-threaded programs.
 *
 * to use:
 * $ export LD_PRELOAD=/path/to/th_alloc.so <your command>
 */

/* Hard-code some system parameters */

#define SUPER_BLOCK_SIZE 4096
#define SUPER_BLOCK_MASK (~(SUPER_BLOCK_SIZE-1))
#define MIN_ALLOC 32 /* Smallest real allocation.  Round smaller mallocs up */
#define MAX_ALLOC 2048 /* Fail if anything bigger is attempted.  
                        * Challenge: handle big allocations */
#define RESERVE_SUPERBLOCK_THRESHOLD 2

#define FREE_POISON 0xab
#define ALLOC_POISON 0xcd

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <string.h>

#define assert(cond) if (!(cond)) __asm__ __volatile__ ("int $3")

/* Object: One return from malloc/input to free. */
struct __attribute__((packed)) object {
  union {
    struct object *next; // For free list (when not in use)
    char * raw; // Actual data
  };
};

/* Super block bookeeping; one per superblock.  "steal" the first
 * object to store this structure
 */
struct __attribute__((packed)) superblock_bookkeeping {
  struct superblock_bookkeeping * next; // next super block
  struct object *free_list;
  // Free count in this superblock
  uint8_t free_count; // Max objects per superblock is 128-1, so a byte is sufficient
  uint8_t level;
};

/* Superblock: a chunk of contiguous virtual memory.
 * Subdivide into allocations of same power-of-two size. */
struct __attribute__((packed)) superblock {
  struct superblock_bookkeeping bkeep;
  void *raw;  // Actual data here
};


/* The structure for one pool of superblocks.
 * One of these per power-of-two */
struct superblock_pool {
  struct superblock_bookkeeping *next;
  uint64_t free_objects; // Total number of free objects across all superblocks
  uint64_t whole_superblocks; // Superblocks with all entries free
};

// 10^5 -- 10^11 == 7 levels
#define LEVELS 7
static struct superblock_pool levels[LEVELS] = {{NULL, 0, 0},
  {NULL, 0, 0},
  {NULL, 0, 0},
  {NULL, 0, 0},
  {NULL, 0, 0},
  {NULL, 0, 0},
  {NULL, 0, 0}};

static inline int size2level (ssize_t size) {
  /* Your code here.
   * Convert the size to the correct power of two. 
   * Recall that the 0th entry in levels is really 2^5, 
   * the second level represents 2^6, etc.
   */
  if (size <= 32) return 0;
  int i = -5;
  if (!((size != 0) && !(size & (size - 1)))) ++i;
  while (size >>= 1) ++i;
  return i;
}

static inline
struct superblock_bookkeeping * alloc_super (int power) {

  void *page;
  struct superblock* sb;
  int free_objects = 0, bytes_per_object = 0;
  char *cursor;
  // Your code here
  // Allocate a page of anonymous memory
  // WARNING: DO NOT use brk---use mmap, lest you face untold suffering

  page = mmap(NULL, SUPER_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  sb = (struct superblock*) page;
  // Put this one the list.
  sb->bkeep.next = levels[power].next;
  levels[power].next = &sb->bkeep;
  levels[power].whole_superblocks++;
  levels[power].next->level = power;
  levels[power].next->free_list = NULL;

  // Your code here: Calculate and fill the number of free objects in this superblock
  //  Be sure to add this many objects to levels[power]->free_objects, reserving
  //  the first one for the bookkeeping.
  free_objects = (SUPER_BLOCK_SIZE >> (power + 5)) - 1;
  levels[power].next->free_count = free_objects;
  levels[power].free_objects += free_objects;
  bytes_per_object = 2 << (power + 4);

  // The following loop populates the free list with some atrocious
  // pointer math.  You should not need to change this, provided that you
  // correctly calculate free_objects.
  cursor = (char *) sb;
  // skip the first object
  for (cursor += bytes_per_object; free_objects--; cursor += bytes_per_object) {
    // Place the object on the free list
    struct object* tmp = (struct object *) cursor;
    tmp->next = sb->bkeep.free_list;
    sb->bkeep.free_list = tmp;
  }
  return &sb->bkeep;
}

void *malloc(size_t size) {
  struct superblock_pool *pool;
  struct superblock_bookkeeping *bkeep;
  void *rv = NULL;
  int power = size2level(size);

  // Check that the allocation isn't too big
  if (size > MAX_ALLOC) {
    errno = -ENOMEM;
    return NULL;
  }

  pool = &levels[power];

  if (!pool->free_objects) {
    bkeep = alloc_super(power);
  } else bkeep = pool->next;

  while (bkeep != NULL) {
    if (bkeep->free_count) {
      struct object *next = bkeep->free_list;
      /* Remove an object from the free list. */
      // Your code here
      //
      // NB: If you take the first object out of a whole
      //     superblock, decrement levels[power]->whole_superblocks
      bkeep->free_list = next->next;
      rv = next;
      if ((SUPER_BLOCK_SIZE >> (power+5)) - 1 == bkeep->free_count) pool->whole_superblocks--;
      --pool->free_objects;
      --bkeep->free_count;
      break;
    }
    bkeep = bkeep->next;
  }

  // assert that rv doesn't end up being NULL at this point
  assert(rv != NULL);

  /* Exercise 3: Poison a newly allocated object to detect init errors.
   * Hint: use ALLOC_POISON
   */

  memset(rv, ALLOC_POISON, 2 << (bkeep->level + 4));
  return rv;
}

static inline
struct superblock_bookkeeping * obj2bkeep (void *ptr) {
  uint64_t addr = (uint64_t) ptr;
  addr &= SUPER_BLOCK_MASK;
  return (struct superblock_bookkeeping *) addr;
}

void free(void *ptr) {
  struct superblock_bookkeeping *bkeep = obj2bkeep(ptr);
  memset(ptr, FREE_POISON, 2 << (bkeep->level + 4));
  printf("Free in level %d before:\n Total free objects: %lu\n Whole superblocks: %lu\n Free objects in superblock: %d\n",
      bkeep->level,
      levels[bkeep->level].free_objects,
      levels[bkeep->level].whole_superblocks,
      bkeep->free_count);
  // Your code here.
  //   Be sure to put this back on the free list, and update the
  //   free count.  If you add the final object back to a superblock,
  //   making all objects free, increment whole_superblocks.
  struct object * tmp = (struct object *)ptr;
  tmp->next = bkeep->free_list;
  bkeep->free_list = tmp;
  bkeep->free_count++;
  levels[bkeep->level].free_objects++;
  if (bkeep->free_count == ((SUPER_BLOCK_SIZE >> (bkeep->level + 5)) - 1))
    levels[bkeep->level].whole_superblocks++;
  while (levels[bkeep->level].whole_superblocks > RESERVE_SUPERBLOCK_THRESHOLD) {
    // Exercise 4: Your code here
    // Remove a whole superblock from the level
    // Return that superblock to the OS, using mmunmap
    struct superblock_bookkeeping **bktmp = &levels[bkeep->level].next;
    while ((*bktmp)->free_count != (SUPER_BLOCK_SIZE >> (bkeep->level + 5)) - 1) bktmp = &(*bktmp)->next;
    *bktmp = (*bktmp)->next;
    munmap((*bktmp) - (sizeof(struct superblock_bookkeeping*) + sizeof(void*)), SUPER_BLOCK_SIZE);
    --levels[bkeep->level].whole_superblocks;
    levels[bkeep->level].free_objects -= (SUPER_BLOCK_SIZE >> (bkeep->level + 5)) - 1;
  }
  printf("Free in level %d after:\n Total free objects: %lu\n Whole superblocks: %lu\n Free objects in superblock: %d\n",
      bkeep->level,
      levels[bkeep->level].free_objects,
      levels[bkeep->level].whole_superblocks,
      bkeep->free_count);
}

// Do NOT touch this - this will catch any attempt to load this into a multi-threaded app
int pthread_create(void __attribute__((unused)) *x, ...) {
  exit(-ENOSYS);
}

