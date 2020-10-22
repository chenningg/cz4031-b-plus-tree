#ifndef B_PLUS_TREE_H
#define B_PLUS_TREE_H

#include "types.h"
#include "memory_pool.h"

#include <cstddef>
#include <array>

// A node in the B+ Tree.
class Node
{
private:
  // Variables
  Address *pointers;      // A pointer to an array of struct {void *blockAddress, short int offset} containing other nodes in disk.
  float *keys;            // Pointer to an array of keys in this node.
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
  MemoryPool *disk;     // Pointer to a memory pool for data blocks.
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

  // Helper function for deleting records.
  void removeInternal(float key, Node *cursorDiskAddress, Node *childDiskAddress);

  // Finds the direct parent of a node in the B+ Tree.
  // Takes in root and a node to find parent for, returns parent's disk address.
  Node *findParent(Node *, Node *, float lowerBoundKey);

public:
  // Methods

  // Constructor, takes in block size to determine max keys/pointers in a node.
  BPlusTree(std::size_t blockSize, MemoryPool *disk, MemoryPool *index);

  // Search for keys corresponding to a range in the B+ Tree given a lower and upper bound. Returns a list of matching Records.
  void search(float lowerBoundKey, float upperBoundKey);

  // Inserts a record into the B+ Tree.
  void insert(Address address, float key);

  // Inserts a record into a linked list. Returns the address of the new linked list head (if any).
  Address insertLL(Address LLHead, Address address, float key);

  // Prints out the B+ Tree in the console.
  void display(Node *, int level);

  // Prints out a specific node and its contents in the B+ Tree.
  void displayNode(Node *node);

  // Prints out a data block and its contents in the disk.
  void displayBlock(void *block);

  // Prints out an entire linked list's records.
  void displayLL(Address LLHeadAddress);

  // Remove a range of records from the disk (and B+ Tree).
  // Accepts a key to delete.
  int remove(float key);

  // Remove an entire linked list from the start to the end for a given linked list head
  void removeLL(Address LLHeadAddress);

  // Getters and setters

  // Returns a pointer to the root of the B+ Tree.
  Node *getRoot()
  {
    return root;
  };

  // Returns the number of levels in this B+ Tree.
  int getLevels();

  int getNumNodes()
  {
    return numNodes;
  }

  int getMaxKeys()
  {
    return maxKeys;
  }
};

#endif