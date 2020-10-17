#include "memory_pool.h"

#include <iostream>
#include <vector>
#include <unordered_map>
#include <tuple>

// Constructors

MemoryPool::MemoryPool(std::size_t maxPoolSize, std::size_t blockSize)
{
  this->maxPoolSize = maxPoolSize;
  this->blockSize = blockSize;
  this->sizeUsed = 0;
  this->actualSizeUsed = 0;
  this->allocated = 0;

  // Create pool of blocks
  std::unordered_map<int, std::vector<Record>> blocks;

  this->pool = blocks;
  this->block = 0;
}

// Methods

bool MemoryPool::allocateBlock()
{
  // Allocate a new block and move free pointer to start of block.
  if (sizeUsed + blockSize <= maxPoolSize)
  {
    std::vector<Record> newBlock;

    // Assign new block with blockID
    pool[allocated] = newBlock;

    // Updated variables
    block = allocated;
    allocated += 1;
    sizeUsed += blockSize;

    return true;
  }
  else
  {
    std::cout << "Error: No memory left to allocate new block (" << sizeUsed << "/" << maxPoolSize << " used)." << '\n';
    return false;
  }
}

std::tuple<int, int> MemoryPool::allocate(Record record)
{
  // If record size exceeds block size, throw an error.
  if (sizeof(record) > blockSize)
  {
    std::cout << "Error: Record size larger than block size (" << sizeof(record) << " vs " << blockSize << ")! Increase block size to store data." << '\n';
    throw std::invalid_argument("Record size too large!");
  }

  // If no free blocks, make a new block.
  if (allocated == 0 || pool[block].size() * sizeof(Record) + sizeof(record) > blockSize)
  {
    bool isSuccessful = allocateBlock();
    if (!isSuccessful)
    {
      throw std::logic_error("Failed to allocate new block!");
    }
  }

  // Add record to the block and save its address for indexing.
  pool[block].push_back(record);
  int offset = pool[block].size() - 1;

  // Update actual size used
  actualSizeUsed += sizeof(record);

  std::tuple<int, int> recordAddress(block, offset);

  return recordAddress;
}

bool MemoryPool::deallocate(int blockID, int offset)
{
  try
  {
    std::vector<Record> recordBlock = pool.at(blockID);
    recordBlock.erase(recordBlock.begin() + offset);

    // Update actual size used
    actualSizeUsed -= sizeof(Record);

    // If block is empty, just remove it
    if (recordBlock.size() < 1)
    {
      pool.erase(blockID);

      // Update size used
      sizeUsed -= blockSize;
    }
    return true;
  }
  catch (...)
  {
    std::cout << "Error: Could not remove record/block at given blockID (" << blockID << ") and offset (" << offset << ")." << '\n';
    return false;
  };
}

Record MemoryPool::read(int blockID, int offset) const
{
  Record record = pool.at(blockID).at(offset);
  return record;
}

MemoryPool::~MemoryPool(){

};