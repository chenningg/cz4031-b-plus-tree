#include "b_plus_tree.h"
#include "types.h"

#include <vector>
#include <cstring>
#include <iostream>

using namespace std;

// Find the parent of a node.
Node *BPlusTree::findParent(Node *cursorDiskAddress, Node *childDiskAddress, float lowerBoundKey)
{
  // Load in cursor into main memory, starting from root.
  Address cursorAddress{cursorDiskAddress, 0};
  Node *cursor = (Node *)index->loadFromDisk(cursorAddress, nodeSize);

  // If the root cursor passed in is a leaf node, there is no children, therefore no parent.
  if (cursor->isLeaf)
  {
    return nullptr;
  }

  // Maintain parentDiskAddress
  Node *parentDiskAddress = cursorDiskAddress;

  // While not leaf, keep following the nodes to correct key.
  while (cursor->isLeaf == false)
  {
    // Check through all pointers of the node to find match.
    for (int i = 0; i < cursor->numKeys + 1; i++)
    {
      if (cursor->pointers[i].blockAddress == childDiskAddress)
      {
        return parentDiskAddress;
      }
    }

    for (int i = 0; i < cursor->numKeys; i++)
    {
      // If key is lesser than current key, go to the left pointer's node.
      if (lowerBoundKey < cursor->keys[i])
      {
        // Load node in from disk to main memory.
        Node *mainMemoryNode = (Node *)index->loadFromDisk(cursor->pointers[i], nodeSize);

        // Update parent address.
        parentDiskAddress = (Node *)cursor->pointers[i].blockAddress;

        // Move to new node in main memory.
        cursor = (Node *)mainMemoryNode;
        break;
      }

      // Else if key larger than all keys in the node, go to last pointer's node (rightmost).
      if (i == cursor->numKeys - 1)
      {
        // Load node in from disk to main memory.
        Node *mainMemoryNode = (Node *)index->loadFromDisk(cursor->pointers[i + 1], nodeSize);

        // Update parent address.
        parentDiskAddress = (Node *)cursor->pointers[i + 1].blockAddress;

        // Move to new node in main memory.
        cursor = (Node *)mainMemoryNode;
        break;
      }
    }
  }

  // If we reach here, means cannot find already.
  return nullptr;
}


int BPlusTree::getLevels() {

  if (rootAddress == nullptr) {
    return 0;
  }

  // Load in the root node from disk
  Address rootDiskAddress{rootAddress, 0};
  root = (Node *)index->loadFromDisk(rootDiskAddress, nodeSize);
  Node *cursor = root;

  levels = 1;

  while (!cursor->isLeaf) {
    cursor = (Node *)index->loadFromDisk(cursor->pointers[0], nodeSize);
    levels++;
  }

  // Account for linked list (count as one level)
  levels++;

  return levels;
}
