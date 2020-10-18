#ifndef B_PLUS_TREE_H
#define B_PLUS_TREE_H

#include "types.h"
#include "memory_pool.h"

#include <cstddef>
#include <vector>

// A node in the B+ Tree.
class Node
{
private:
  // Variables
  bool isLeaf;            // Whether this node is a leaf node.
  float *keys;            // Pointer to an array of keys in this node.
  int numKeys;            // Current number of keys in this node.
  Address *pointers;      // A pointer to an array of struct {void *blockAddress, short int offset} containing other nodes in disk.
  friend class BPlusTree; // Let the BPlusTree class access this class' private variables.

public:
  // Methods

  // Constructor
  Node(int maxKeys); // Takes in max keys in a node.
};

// The B+ Tree itself.
class BPlusTree
{
private:
  // Variables
  MemoryPool *index;    // Pointer to a memory pool in disk for index.
  Node *root;           // Pointer to root of the B+ Tree.
  int maxKeys;          // Maximum keys in a node.
  std::size_t nodeSize; // Size of a node = Size of block.

  // Methods

  void insertInternal(int, Node *, Node *);

  // Finds the direct parent of a node in the B+ Tree.
  Node *findParent(Node *, Node *);

public:
  // Methods

  // Constructor, takes in block size to determine max keys/pointers in a node.
  BPlusTree(std::size_t blockSize);

  // Search for keys corresponding to a range in the B+ Tree given a lower and upper bound. Returns a list of matching Records.
  std::vector<Record> select(float lowerBoundKey, float upperBoundKey);

  // Inserts a record into the B+ Tree.
  void insert(Address address, float key);

  // Prints out the B+ Tree in the console.
  void display(Node *, int level);

  // Prints out a specific node and its contents in the B+ Tree.
  void displayNode(Node *node);

  // Prints out a data block and its contents in the disk.
  void displayBlock(void *block);

  // Returns a pointer to the root of the B+ Tree.
  Node *getRoot();
};

#endif