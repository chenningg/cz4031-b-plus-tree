#include "memory_pool.h"

#include <iostream>
#include <vector>


// Constructors

MemoryPool::MemoryPool(std::size_t poolSize, std::size_t blockSize)
{
  this->poolSize = poolSize;
  this->blockSize = blockSize;
  this->sizeUsed = 0;
  this->allocated = 0;
  this->available = poolSize / blockSize;

<<<<<<< Updated upstream
  // Allocate memory for pool
  this->pool = new unsigned char[poolSize];
  this->block = pool;
  this->free = pool;
=======
  // Create pool of blocks
  std::unordered_map<int, std::vector<Record> > blocks;

  this->pool = blocks;
  this->block = 0;
>>>>>>> Stashed changes
}

// Methods

bool MemoryPool::allocateBlock()
{
  // Allocate a new block and move free pointer to start of block.
  if (available > 0)
  {
    block = pool + (blockSize * allocated);
    free = block;

    // Updated variables
    allocated += 1;
    available -= 1;
    sizeUsed += blockSize;

    return true;
  }
  else
  {
    std::cout << "Error: No memory left to allocate (" << sizeUsed << "/" << poolSize << " used)." << '\n';
    return false;
  }
}

unsigned char *MemoryPool::allocate(Movie movie)
{
  // If record size exceeds block size, throw an error.
  if (sizeof(movie) > blockSize)
  {
    std::cout << "Error: Record size larger than block size (" << sizeof(Movie) << " vs " << blockSize << ")! Increase block size to store data." << '\n';
    return NULL;
  }

  // If no free blocks, make a new block.
  if (free - block + sizeof(movie) > blockSize || allocated == 0)
  {
    bool successful = allocateBlock();
    if (!successful)
    {
      return NULL;
    }
  }

  // Add record to the block and save its address for indexing.
  memcpy(free, &movie, sizeof(movie));
  unsigned char *record = free;

  // Move free pointer forward.
  free += sizeof(movie);

  return record;
}

bool MemoryPool::deallocate(Movie *movie)
{
  if (movie != nullptr)
  {
    movie->isDeleted = true;
    return true;
  }
  else
  {
    std::cout << "Error: No chunk found to deallocate at (" << &movie << ")." << '\n';
    return false;
  }
}

MemoryPool::~MemoryPool()
{
  // Clean up memory
  delete pool;
  pool = NULL;
  block = NULL;
  free = NULL;
}