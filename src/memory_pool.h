#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <vector>

// Defines a single movie record (read from data file).
struct Record
{
  bool isDeleted;      // Flag denoting this record has been deleted.
  float averageRating; // Average rating of this movie.
  int numVotes;        // Number of votes of this movie.
  char tconst[10];     // ID of the movie.
};

struct Block
{
  bool isAccessed; // Whether this block has been accessed before.
};

class MemoryPool
{
public:
  // =============== Methods ================ //

  // Creates a new memory pool with the following parameters:
  // poolSize: Size of the memory pool.
  // blockSize: Size of each block in the pool.
  MemoryPool(std::size_t poolSize, std::size_t blockSize);

  // Allocate a new block from the memory pool. Returns false if error.
  bool allocateBlock();

  // Allocates a new chunk to the memory pool.
  // Creates a new block if chunk is unable to fit in current free block.
  unsigned char *allocate(Record record);

  // Deallocates an existing block (Not implemented).
  void deallocateBlock();

  // Deallocates an existing chunk. Returns false if error.
  bool deallocate(Record *record);

  // Access a record given a block address and offset
  unsigned char *getRecord(Block *blockAddress, int offset);

  // Returns the size of the memory pool.
  std::size_t getPoolSize() const
  {
    return poolSize;
  };

  // Returns the size of a block in memory pool.
  std::size_t getBlockSize() const
  {
    return blockSize;
  };

  // Returns current size used in memory pool.
  std::size_t getSizeUsed() const
  {
    return sizeUsed;
  }

  // Returns number of currently allocated blocks.
  int getAllocated() const
  {
    return allocated;
  };

  // Returns number of blocks available to allocate.
  int getAvailable() const
  {
    return available;
  };

  // Destructor
  ~MemoryPool();

private:
  // =============== Data ================ //

  std::size_t poolSize;  // Size of memory pool.
  std::size_t blockSize; // Size of each block in pool in bytes.
  std::size_t sizeUsed;  // Current size used up for storage.

  int allocated;      // Number of currently allocated blocks.
  int available;      // Number of blocks available to allocate.
  int blocksAccessed; // Number of blocks accessed while getting records.

  unsigned char *pool;  // Pointer to start of memory pool.
  unsigned char *block; // Pointer to start of current block.
  unsigned char *free;  // Pointer to the next free address in the memory pool.
};

#endif