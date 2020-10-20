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
  Address *pointers;      // A pointer to an array of struct {void *blockAddress, short int offset} containing other nodes in disk.
  float *keys;            // Pointer to an array of keys in this node.
  Address *parent;        // A pointer to the parent of this node in the disk.
  int numKeys;            // Current number of keys in this node.
  bool isLeaf;            // Whether this node is a leaf node.
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
  Node *root;           // Pointer to the main memory root (if it's loaded).
  void *rootAddress;    // Pointer to root's address on disk.
  int maxKeys;          // Maximum keys in a node.
  int levels;           // Number of levels in this B+ Tree.
  int numNodes;         // Number of nodes in this B+ Tree.
  std::size_t nodeSize; // Size of a node = Size of block.

  // Methods

  // Updates the parent node to point at both child nodes, and adds a parent node if needed.
  void insertInternal(float key, Node *cursorDiskAddress, Node *childDiskAddress);

  // Finds the direct parent of a node in the B+ Tree.
  // Takes in root and a node to find parent for, returns parent's disk address.
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

  // Getters and setters

  // Returns a pointer to the root of the B+ Tree.
  Node *getRoot()
  {
    return root;
  };

  // Returns the number of levels in this B+ Tree.
  int getLevels()
  {
    return levels;
  }

  int getNumNodes()
  {
    return numNodes;
  }

  int getMaxKeys()
  {
    return maxKeys;
  }
};

void b_plus_tree_test();

#endif