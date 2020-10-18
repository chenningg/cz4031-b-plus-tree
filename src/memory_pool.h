#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include "address_type.h"
#include "types.h"

#include <vector>
#include <unordered_map>
#include <tuple>

class MemoryPool
{
public:
  // =============== Methods ================ //

  // Creates a new memory pool with the following parameters:
  // maxPoolSize: Maximum size of the memory pool.
  // blockSize: The fixed size of each block in the pool.
  MemoryPool(std::size_t maxPoolSize, std::size_t blockSize);

  // Allocate a new block from the memory pool. Returns false if error.
  bool allocateBlock();

  // Allocates a new chunk to the memory pool.
  // Creates a new block if chunk is unable to fit in current free block.
  // Returns a struct with blockID and the record's offset within the block.
  Address allocate(std::size_t sizeRequired);

  // Deallocates an existing record and block if block becomes empty. Returns false if error.
  bool deallocate(void *blockAddress, short int offset, std::size_t sizeToDelete);

  // Returns the maximum size of this memory pool.
  std::size_t getMaxPoolSize() const
  {
    return maxPoolSize;
  }

  // Returns the size of a block in memory pool.
  std::size_t getBlockSize() const
  {
    return blockSize;
  };

  // Returns the size used in the current block.
  std::size_t getBlockSizeUsed() const
  {
    return blockSizeUsed;
  };

  // Returns current size used in memory pool (total blocks size).
  std::size_t getSizeUsed() const
  {
    return sizeUsed;
  }

  // Returns actual size of all records stored in memory pool.
  std::size_t getActualSizeUsed() const
  {
    return actualSizeUsed;
  }

  // Returns number of currently allocated blocks.
  int getAllocated() const
  {
    return allocated;
  };

  // Destructor
  ~MemoryPool();

private:
  // =============== Data ================ //

  std::size_t maxPoolSize;    // Maximum size allowed for pool.
  std::size_t blockSize;      // Size of each block in pool in bytes.
  std::size_t sizeUsed;       // Current size used up for storage (total block size).
  std::size_t actualSizeUsed; // Actual size used based on records stored in storage.
  std::size_t blockSizeUsed;  // Size used up within the curent block we are pointing to.

  int allocated; // Number of currently allocated blocks.

  void *pool;  // Pointer to the memory pool.
  void *block; // Current block pointer we are inserting to.
};

#endif