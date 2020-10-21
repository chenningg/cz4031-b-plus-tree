#include "b_plus_tree.h"
#include "memory_pool.h"
#include "types.h"

#include <tuple>
#include <iostream>
#include <array>
#include <unordered_map>
#include <cstring>

using namespace std;

bool myNullPtr = false;

Node::Node(int maxKeys)
{
  // Initialize empty array of keys and pointers.
  keys = new float[maxKeys];
  pointers = new Address[maxKeys + 1];

  for (int i = 0; i < maxKeys + 1; i++)
  {
    Address nullAddress{(void *)myNullPtr, 0};
    pointers[i] = nullAddress;
  }
  numKeys = 0;
}

BPlusTree::BPlusTree(std::size_t blockSize, MemoryPool *disk, MemoryPool *index)
{
  // Get size left for keys and pointers in a node after accounting for node's isLeaf and numKeys attributes.
  size_t nodeBufferSize = blockSize - sizeof(bool) - sizeof(int);

  // Set max keys available in a node. Each key is a float, each pointer is a struct of {void *blockAddress, short int offset}.
  // Therefore, each key is 4 bytes. Each pointer is around 16 bytes.

  // Initialize node buffer with a pointer. P | K | P , always one more pointer than keys.
  size_t sum = sizeof(Address);
  maxKeys = 0;

  // Try to fit as many pointer key pairs as possible into the node block.
  while (sum + sizeof(Address) + sizeof(float) <= nodeBufferSize)
  {
    sum += (sizeof(Address) + sizeof(float));
    maxKeys += 1;
  }

  if (maxKeys == 0)
  {
    throw std::overflow_error("Error: Keys and pointers too large to fit into a node!");
  }

  // Initialize root to NULL
  rootAddress = nullptr;
  root = nullptr;

  // Set node size to be equal to block size.
  nodeSize = blockSize;

  // Initialize initial variables
  levels = 0;
  numNodes = 0;

  // Initialize disk space for index and set reference to disk.
  
  this->disk = disk;
  this->index = index;
}