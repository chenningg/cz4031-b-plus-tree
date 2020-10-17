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
  this->blocksAccessed = 0;
  this->available = poolSize / blockSize;

  // Allocate memory for pool
  this->pool = new unsigned char[poolSize];
  this->block = pool;
  this->free = pool;
}

// Methods

bool MemoryPool::allocateBlock()
{
  // Allocate a new block and move free pointer to start of block.
  if (available > 0)
  {
    block = pool + (blockSize * allocated);

    Block newBlock;
    newBlock.isAccessed = false;

    // Add block header to check isAccessed flag
    block = (unsigned char *)(memcpy(block, &newBlock, sizeof(Block)));

    // Assign free pointer to point after block header to insert records to
    free = block + sizeof(Block);

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

unsigned char *MemoryPool::allocate(Record record)
{
  // If record size exceeds block size, throw an error.
  if (sizeof(record) > blockSize)
  {
    std::cout << "Error: Record size larger than block size (" << sizeof(Record) << " vs " << blockSize << ")! Increase block size to store data." << '\n';
    return NULL;
  }

  // If no free blocks, make a new block.
  if (free - block + sizeof(record) > blockSize || allocated == 0)
  {
    bool successful = allocateBlock();
    if (!successful)
    {
      return NULL;
    }
  }

  // Add record to the block and save its address for indexing.
  memcpy(free, &record, sizeof(record));
  unsigned char *recordAddress = free;

  // Move free pointer forward.
  free += sizeof(record);

  return recordAddress;
}

bool MemoryPool::deallocate(Record *record)
{
  if (record != nullptr)
  {
    record->isDeleted = true;
    return true;
  }
  else
  {
    std::cout << "Error: No chunk found to deallocate at (" << &record << ")." << '\n';
    return false;
  }
}

unsigned char *MemoryPool::read(Block *blockAddress, int offset)
{
  if (blockAddress)
  {
    if (blockAddress->isAccessed != true)
    {
      blockAddress->isAccessed = true;
      blocksAccessed += 1;
    }
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