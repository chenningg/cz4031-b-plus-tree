#include "b_plus_tree.h"
#include "memory_pool.h"
#include "types.h"

#include <tuple>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstring>

using namespace std;

Node::Node(int maxKeys)
{
  // Initialize empty array of keys and pointers.
  this->keys = new float[maxKeys];
  this->pointers = new Address[maxKeys + 1];

  Address addr{nullptr, 0};
  for (int i = 0; i < maxKeys + 1; i++)
  {
    this->pointers[i] = addr;
  }
  this->numKeys = 0;
}

BPlusTree::BPlusTree(std::size_t blockSize)
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

  // Initialize disk space for index.
  index = new MemoryPool(200000000, 100);
}

void b_plus_tree_test()
{
  // Create memory pools for the disk.
  BPlusTree tree = BPlusTree(100);
  std::cerr << "Max keys: " << tree.getMaxKeys() << endl;

  MemoryPool *test = new MemoryPool(300000000, 100);

  for (int i = 1; i < 20; i++)
  {
    Record record = {"tt000001", 1.0, 80};
    Address addr = test->allocate(sizeof(record));
    std::memcpy(addr.blockAddress, &record, sizeof(record));
    tree.insert(addr, float(i));
  }

  tree.display(tree.getRoot(), 1);
}