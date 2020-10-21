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
    // Address nullAddress{NULL, 0};
    // pointers[i] = nullAddress;
    Address nullAddress{(void *)myNullPtr, 0};
    pointers[i] = nullAddress;
  }
  numKeys = 0;
}

BPlusTree::BPlusTree(std::size_t blockSize, MemoryPool *disk)
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
  index = new MemoryPool(200000000, 100);
  this->disk = disk;
}

void b_plus_tree_test()
{
  // Create memory pools for the disk.
  MemoryPool *test = new MemoryPool(300000000, 100);

  BPlusTree tree = BPlusTree(100, test);
  std::cerr << "Max keys: " << tree.getMaxKeys() << endl;

  for (int j = 0; j < 10; j++)
  {
    for (int i = 1; i < 7; i++)
    {
      Record record1 = {"tt000001", 1.0, 80};
      Address addr = test->saveToDisk(&record1, sizeof(Record));
      tree.insert(addr, float(i));
    }
  }

  for (int i = 1; i < 7; i++)
  {
    Record record1 = {"tt000002", 2.0, 80};
    Address addr = test->saveToDisk(&record1, sizeof(Record));
    tree.insert(addr, float(i));
  }

  for (int i = 1; i < 7; i++)
  {
    Record record1 = {"tt000003", 3.0, 80};
    Address addr = test->saveToDisk(&record1, sizeof(Record));
    tree.insert(addr, float(i));
  }

  for (int i = 1; i < 7; i++)
  {
    Record record1 = {"tt000004", 3.0, 80};
    Address addr = test->saveToDisk(&record1, sizeof(Record));
    tree.insert(addr, float(i));
  }

  tree.search(float(2), float(6));

  tree.remove(float(2));
  tree.display(tree.getRoot(), 1);
}