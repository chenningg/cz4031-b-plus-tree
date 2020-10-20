#include "b_plus_tree.h"
#include "types.h"

#include <vector>
#include <cstring>
#include <iostream>

using namespace std;

// Updates the parent node to point at both child nodes, and adds a parent node if needed.
// Takes the lower bound of the right child, and the main memory address of the parent and the new child,
// as well as disk address of parent and new child.
void BPlusTree::insertInternal(float key, Node *cursorDiskAddress, Node *childDiskAddress)
{
  // Load in cursor (parent) and child from disk to get latest copy.
  void *cursorMainMemory = operator new(nodeSize);
  std::memcpy(cursorMainMemory, cursorDiskAddress, nodeSize);
  Node *cursor = (Node *)cursorMainMemory;

  if (cursorDiskAddress == rootAddress)
  {
    root = cursor;
  }

  void *childMainMemory = operator new(nodeSize);
  std::memcpy(childMainMemory, childDiskAddress, nodeSize);
  Node *child = (Node *)childMainMemory;

  std::cerr << "cursor insertInternal:" << endl;
  for (int i = 0; i < cursor->numKeys; i++)
  {
    std::cerr << cursor->keys[i] << " | ";
  }
  std::cerr << endl;

  std::cerr << "child right insertInternal:" << endl;
  for (int i = 0; i < child->numKeys; i++)
  {
    std::cerr << child->keys[i] << " | ";
  }
  std::cerr << endl;

  // If parent (cursor) still has space, we can simply add the child node as a pointer.
  // We don't have to load parent from the disk again since we still have a main memory pointer to it.
  if (cursor->numKeys < maxKeys)
  {
    // Iterate through the parent to see where to put in the lower bound key for the new child.
    int i = 0;
    while (key > cursor->keys[i] && i < cursor->numKeys)
    {
      i++;
    }

    // Now we have i, the index to insert the key in. Bubble swap all keys back to insert the new child's key.
    // We use numKeys as index since we are going to be inserting a new key.
    for (int j = cursor->numKeys; j > i; j--)
    {
      cursor->keys[j] = cursor->keys[j - 1];
    }

    // Shift all pointers one step right (right pointer of key points to lower bound of key).
    for (int j = cursor->numKeys + 1; j > i + 1; j--)
    {
      cursor->pointers[j] = cursor->pointers[j - 1];
    }

    // Add in new child's lower bound key and pointer to the parent.
    cursor->keys[i] = key;
    cursor->numKeys++;

    // Right side pointer of key of parent will point to the new child node.
    Address childAddress{childDiskAddress, 0};
    cursor->pointers[i + 1] = childAddress;

    // Write the updated parent (cursor) to the disk.
    std::memcpy(cursorDiskAddress, cursor, nodeSize);

    std::cerr << "insertInternal - Updating parent: " << cursor->keys[0] << endl;
  }
  // If parent node doesn't have space, we need to recursively split parent node and insert more parent nodes.
  else
  {
    std::cerr << "insertInternal overflow: Splitting..." << endl;
    // Make new internal node (split this parent node into two).
    // Note: We DO NOT add a new key, just a new pointer!
    Node *newInternal = new Node(maxKeys);

    // Increment nodes in the tree
    numNodes++;

    // Same logic as above, keep a temp list of keys and pointers to insert into the split nodes.
    // Now, we have one extra pointer to keep track of (new child's pointer).
    float tempKeyList[maxKeys + 1];
    Address tempPointerList[maxKeys + 2];

    // Copy all keys into a temp key list.
    // Note all keys are filled so we just copy till maxKeys.
    for (int i = 0; i < maxKeys; i++)
    {
      tempKeyList[i] = cursor->keys[i];
    }

    // Copy all pointers into a temp pointer list.
    // There is one more pointer than keys in the node so maxKeys + 1.
    for (int i = 0; i < maxKeys + 1; i++)
    {
      tempPointerList[i] = cursor->pointers[i];
    }

    // Find index to insert key in temp key list.
    int i = 0;
    while (key > tempKeyList[i] && i < maxKeys)
    {
      i++;
    }

    // Swap all elements higher than index backwards to fit new key.
    int j;
    for (int j = maxKeys; j > i; j--)
    {
      tempKeyList[j] = tempKeyList[j - 1];
    }

    // Insert new key into array in the correct spot (sorted).
    tempKeyList[i] = key;

    // Move all pointers back to fit new child's pointer as well.
    for (int j = maxKeys + 1; j > i + 1; j--)
    {
      tempPointerList[j] = tempPointerList[j - 1];
    }

    // Insert a pointer to the child to the right of its key.
    Address childAddress = {childDiskAddress, 0};
    tempPointerList[i + 1] = childAddress;
    newInternal->isLeaf = false; // Can't be leaf as it's a parent.

    // Split the two new nodes into two. ⌊(n+1)/2⌋ keys for left.
    // For right, we drop the rightmost key since we only need to represent the pointer.
    cursor->numKeys = (maxKeys + 1) / 2;
    newInternal->numKeys = maxKeys - (maxKeys + 1) / 2;

    // Insert new keys into the new internal parent node.
    for (i = 0, j = cursor->numKeys + 1; i < newInternal->numKeys; i++, j++)
    {
      std::cerr << "Insert key into newInternal: " << tempKeyList[i] << endl;
      newInternal->keys[i] = tempKeyList[j];
    }

    // Insert pointers into the new internal parent node.
    for (i = 0, j = cursor->numKeys + 1; i < newInternal->numKeys + 1; i++, j++)
    {
      newInternal->pointers[i] = tempPointerList[j];
    }

    // Note that we don't have to modify keys in the old parent cursor.
    // Because we already reduced its numKeys as we are only adding to the right bound.

    // Save the old parent and new internal node to disk.
    std::memcpy(cursorDiskAddress, cursor, nodeSize);

    Address newInternalDiskAddress = index->allocate(nodeSize);
    std::memcpy(newInternalDiskAddress.blockAddress, newInternal, nodeSize);

    std::cerr << "newInternal:" << endl;
    for (int i = 0; i < newInternal->numKeys; i++)
    {
      std::cerr << newInternal->keys[i] << " | ";
    }
    std::cerr << endl;

    std::cerr << "cursorLeft:" << endl;
    for (int i = 0; i < cursor->numKeys; i++)
    {
      std::cerr << cursor->keys[i] << " | ";
    }
    std::cerr << endl;

    std::cerr << "current root:" << endl;
    for (int i = 0; i < root->numKeys; i++)
    {
      std::cerr << root->keys[i] << " | ";
    }
    std::cerr << endl;

    // If current cursor is the root of the tree, we need to create a new root.
    if (cursor == root)
    {

      Node *newRoot = new Node(nodeSize);

      // Update number of nodes and level.
      numNodes++;
      levels++;

      // Update newRoot to hold the children.
      // Take the rightmost key of the old parent to be the root.
      // Although we threw it away, we are still using it to denote the leftbound of the old child.
      newRoot->keys[0] = cursor->keys[cursor->numKeys];

      // Update newRoot's children to be the previous two nodes
      Address cursorAddress = {cursorDiskAddress, 0};
      newRoot->pointers[0] = cursorAddress;
      newRoot->pointers[1] = newInternalDiskAddress;

      // Update variables for newRoot
      newRoot->isLeaf = false;
      newRoot->numKeys = 1;

      root = newRoot;

      // Save newRoot into disk.
      Address newRootAddress = index->allocate(nodeSize);
      std::memcpy(newRootAddress.blockAddress, root, nodeSize);

      // Update rootAddress
      rootAddress = newRootAddress.blockAddress;

      std::cerr << "insertInternal - Making new root: " << newInternal->keys[0] << endl;
    }
    // Otherwise, parent is internal, so we need to split and make a new parent internally again.
    // This is done recursively if needed.
    else
    {
      Node *parentDiskAddress = findParent((Node *)rootAddress, cursorDiskAddress, cursor->keys[0]);
      std::cerr << parentDiskAddress << endl;
      insertInternal(cursor->keys[cursor->numKeys], parentDiskAddress, (Node *)newInternalDiskAddress.blockAddress);
    }
  }
}

// Find the parent of a node.
Node *BPlusTree::findParent(Node *cursorDiskAddress, Node *childDiskAddress, float lowerBoundKey)
{
  // Load in cursor into main memory, starting from root.
  void *cursorMainMemory = operator new(nodeSize);
  std::memcpy(cursorMainMemory, cursorDiskAddress, nodeSize);
  Node *cursor = (Node *)cursorMainMemory;

  std::cerr << "Target address:" << childDiskAddress << endl;
  std::cerr << "Target key:" << lowerBoundKey << endl;

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
      std::cerr << "Check: " << cursor->pointers[i].blockAddress << endl;
      std::cerr << "Target: " << childDiskAddress << endl;
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
        void *mainMemoryNode = operator new(nodeSize);
        std::memcpy(mainMemoryNode, cursor->pointers[i].blockAddress, nodeSize);

        // Update parent address.
        parentDiskAddress = (Node *)cursor->pointers[i].blockAddress;

        std::cerr << "Going to address: " << cursor->pointers[i].blockAddress;

        // Move to new node in main memory.
        cursor = (Node *)mainMemoryNode;
        break;
      }

      // Else if key larger than all keys in the node, go to last pointer's node (rightmost).
      if (i == cursor->numKeys - 1)
      {
        // Load node in from disk to main memory.
        void *mainMemoryNode = operator new(nodeSize);
        std::memcpy(mainMemoryNode, cursor->pointers[i + 1].blockAddress, nodeSize);

        // Update parent address.
        parentDiskAddress = (Node *)cursor->pointers[i + 1].blockAddress;

        std::cerr << "Going to address: " << cursor->pointers[i + 1].blockAddress;

        // Move to new node in main memory.
        cursor = (Node *)mainMemoryNode;
        break;
      }
    }
  }

  // If we reach here, means cannot find already.
  return nullptr;
}

// // Iterate through all keys to find child.
// for (int i = 0; i < cursor->numKeys + 1; i++)
// {
//   // Check if pointer exists
//   if (cursor->pointers[i].blockAddress == nullptr)
//   {
//     return nullptr;
//   }
//   // Check if any pointers match the child's disk address we are looking for.
//   else if (cursor->pointers[i].blockAddress == childDiskAddress)
//   {
//     // If it matches, then we have found the parent.
//     return parentDiskAddress;
//   }
//   // If don't match, we need to recursively search children.
//   else
//   {
//     // Load the child from disk and pass in its main memory address.
//     std::cerr << "Now going to:" << (Node *)(cursor->pointers[i].blockAddress) << endl;
//     parentDiskAddress = findParent((Node *)(cursor->pointers[i].blockAddress), childDiskAddress);
//     if (parentDiskAddress != nullptr)
//     {
//       return parentDiskAddress;
//     }
//   }
// }
