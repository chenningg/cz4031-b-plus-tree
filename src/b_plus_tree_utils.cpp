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
  Address cursorAddress{cursorDiskAddress, 0};
  Node *cursor = (Node *)index->loadFromDisk(cursorAddress, nodeSize);

  if (cursorDiskAddress == rootAddress)
  {
    root = cursor;
  }

  Address childAddress{childDiskAddress, 0};
  Node *child = (Node *)index->loadFromDisk(childAddress, nodeSize);

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
    Address cursorAddress{cursorDiskAddress, 0};
    index->saveToDisk(cursor, nodeSize, cursorAddress);
  }
  // If parent node doesn't have space, we need to recursively split parent node and insert more parent nodes.
  else
  {
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
    Address cursorAddress{cursorDiskAddress, 0};
    index->saveToDisk(cursor, nodeSize, cursorAddress);

    // Address newInternalAddress{newInternal, 0};
    Address newInternalDiskAddress = index->saveToDisk(newInternal, nodeSize);

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
      Address newRootAddress = index->saveToDisk(root, nodeSize);

      // Update rootAddress
      rootAddress = newRootAddress.blockAddress;
    }
    // Otherwise, parent is internal, so we need to split and make a new parent internally again.
    // This is done recursively if needed.
    else
    {
      Node *parentDiskAddress = findParent((Node *)rootAddress, cursorDiskAddress, cursor->keys[0]);
      insertInternal(cursor->keys[cursor->numKeys], parentDiskAddress, (Node *)newInternalDiskAddress.blockAddress);
    }
  }
}

// Takes in the parent disk address, the child address to delete, and removes the child.
void BPlusTree::removeInternal(float key, Node *cursorDiskAddress, Node *childDiskAddress)
{
  // Load in cursor (parent) and child from disk to get latest copy.
  Address cursorAddress{cursorDiskAddress, 0};
  Node *cursor = (Node *)index->loadFromDisk(cursorAddress, nodeSize);

  // Check if cursor is root via disk address.
  if (cursorDiskAddress == rootAddress)
  {
    root = cursor;
  }

  // Get address of child to delete.
  Address childAddress{childDiskAddress, 0};

  // If current parent is root
  if (cursor == root)
  {
    // If we have to remove all keys in root (as parent) we need to change the root to its child.
    if (cursor->numKeys == 1)
    {
      // If the larger pointer points to child, make it the new root.
      if (cursor->pointers[1].blockAddress == childDiskAddress)
      {
        // Delete the child completely
        index->deallocate(childAddress, nodeSize);

        // Set new root to be the parent's left pointer
        // Load left pointer into main memory and update root.
        root = (Node *)index->loadFromDisk(cursor->pointers[0], nodeSize);
        rootAddress = (Node *)cursor->pointers[0].blockAddress;

        // We can delete the old root (parent).
        index->deallocate(cursorAddress, nodeSize);

        // Nothing to save to disk. All updates happened in main memory.
        std::cerr << "Root node changed." << endl;
        return;
      }
      // Else if left pointer in root (parent) contains the child, delete from there.
      else if (cursor->pointers[0].blockAddress == childDiskAddress)
      {
        // Delete the child completely
        index->deallocate(childAddress, nodeSize);

        // Set new root to be the parent's right pointer
        // Load right pointer into main memory and update root.
        root = (Node *)index->loadFromDisk(cursor->pointers[1], nodeSize);
        rootAddress = (Node *)cursor->pointers[1].blockAddress;

        // We can delete the old root (parent).
        index->deallocate(cursorAddress, nodeSize);

        // Nothing to save to disk. All updates happened in main memory.
        std::cerr << "Root node changed." << endl;
        return;
      }
    }
  }

  // If reach here, means parent is NOT the root.
  // Aka we need to delete an internal node (possibly recursively).
  int pos;

  // Search for key to delete in parent based on child's lower bound key.
  for (pos = 0; pos < cursor->numKeys; pos++)
  {
    if (cursor->keys[pos] == key)
    {
      break;
    }
  }

  // Delete the key by shifting all keys forward
  for (int i = pos; i < cursor->numKeys; i++)
  {
    cursor->keys[i] = cursor->keys[i + 1];
  }

  // Search for pointer to delete in parent
  // Remember pointers are on the RIGHT for non leaf nodes.
  for (pos = 0; pos < cursor->numKeys + 1; pos++)
  {
    if (cursor->pointers[pos].blockAddress == childDiskAddress)
    {
      break;
    }
  }

  // Now move all pointers from that point on forward by one to delete it.
  for (int i = pos; i < cursor->numKeys + 1; i++)
  {
    cursor->pointers[i] = cursor->pointers[i + 1];
  }

  // Update numKeys
  cursor->numKeys--;

  // ================== KIV!!!!!!!!!!!!!!!!================
  // TODO: CHECK IS CHILD DELETED HERE? WHY NO CHILD DELETE?
  // ================== KIV!!!!!!!!!!!!!!!!================

  // Check if there's underflow in parent
  // No underflow, life is good.
  if (cursor->numKeys >= (maxKeys + 1) / 2 - 1)
  {
    return;
  }

  // If we reach here, means there's underflow in parent's keys.
  // Try to steal some from neighbouring nodes.
  // If we are the root, we are screwed. Just give up.
  if (cursorDiskAddress == rootAddress)
  {
    return;
  }

  // If not, we need to find the parent of this parent to get our siblings.
  // Pass in lower bound key of our child to search for it.
  Node *parentDiskAddress = findParent((Node *)rootAddress, cursorDiskAddress, cursor->keys[0]);
  int leftSibling, rightSibling;

  // Load parent into main memory.
  Address parentAddress{parentDiskAddress, 0};
  Node *parent = (Node *)index->loadFromDisk(parentAddress, nodeSize);

  // Find left and right sibling of cursor, iterate through pointers.
  for (pos = 0; pos < parent->numKeys + 1; pos++)
  {
    if (parent->pointers[pos].blockAddress == cursorDiskAddress)
    {
      leftSibling = pos - 1;
      rightSibling = pos + 1;
      break;
    }
  }

  // Try to borrow a key from either the left or right sibling.
  // Check if left sibling exists. If so, try to borrow.
  if (leftSibling >= 0)
  {
    // Load in left sibling from disk.
    Node *leftNode = (Node *)index->loadFromDisk(parent->pointers[leftSibling], nodeSize);

    // Check if we can steal (ahem, borrow) a key without underflow.
    // Non leaf nodes require a minimum of ⌊n/2⌋
    if (leftNode->numKeys >= (maxKeys + 1) / 2)
    {
      // We will insert this borrowed key into the leftmost of current node (smaller).
      // KIV ============================================
      // TODO: Test if this is correct, shifting one from leftnode to cursor.
      // KIV ============================================

      // Shift all remaining keys and pointers back by one.
      for (int i = cursor->numKeys; i > 0; i--)
      {
        cursor->keys[i] = cursor->keys[i - 1];
      }

      // Transfer borrowed key and pointer to cursor from left node.
      // Basically duplicate cursor lower bound key to keep pointers correct.
      cursor->keys[0] = parent->keys[leftSibling];
      parent->keys[leftSibling] = leftNode->keys[leftNode->numKeys - 1];

      // Move all pointers back to fit new one
      for (int i = cursor->numKeys + 1; i > 0; i--)
      {
        cursor->pointers[i] = cursor->pointers[i - 1];
      }

      // Add pointers to cursor from left node.
      cursor->pointers[0] = leftNode->pointers[leftNode->numKeys];

      // Change key numbers
      cursor->numKeys++;
      leftNode->numKeys--;

      // Update left sibling (shift pointers left)
      leftNode->pointers[cursor->numKeys] = leftNode->pointers[cursor->numKeys + 1];

      // Save parent to disk.
      Address parentAddress{parentDiskAddress, 0};
      index->saveToDisk(parent, nodeSize, parentAddress);

      // Save left sibling to disk.
      index->saveToDisk(leftNode, nodeSize, parent->pointers[leftSibling]);

      // Save current node to disk.
      Address cursorAddress = {cursorDiskAddress, 0};
      index->saveToDisk(cursor, nodeSize, cursorAddress);
      return;
    }
  }

  // If we can't take from the left sibling, take from the right.
  // Check if we even have a right sibling.
  if (rightSibling <= parent->numKeys)
  {
    // If we do, load in right sibling from disk.
    Node *rightNode = (Node *)index->loadFromDisk(parent->pointers[rightSibling], nodeSize);

    // Check if we can steal (ahem, borrow) a key without underflow.
    if (rightNode->numKeys >= (maxKeys + 1) / 2)
    {
      // No need to shift remaining pointers and keys since we are inserting on the rightmost.
      // Transfer borrowed key and pointer (leftmost of right node) over to rightmost of current node.
      cursor->keys[cursor->numKeys] = parent->keys[pos];
      parent->keys[pos] = rightNode->keys[0];

      // Update right sibling (shift keys and pointers left)
      for (int i = 0; i < rightNode->numKeys - 1; i++)
      {
        rightNode->keys[i] = rightNode->keys[i + 1];
      }

      // Transfer first pointer from right node to cursor
      cursor->pointers[cursor->numKeys + 1] = rightNode->pointers[0];

      // Shift pointers left for right node as well to delete first pointer
      for (int i = 0; i < rightNode->numKeys; ++i)
      {
        rightNode->pointers[i] = rightNode->pointers[i + 1];
      }

      // Update numKeys
      cursor->numKeys++;
      rightNode->numKeys--;

      // Save parent to disk.
      Address parentAddress{parentDiskAddress, 0};
      index->saveToDisk(parent, nodeSize, parentAddress);

      // Save right sibling to disk.
      index->saveToDisk(rightNode, nodeSize, parent->pointers[rightSibling]);

      // Save current node to disk.
      Address cursorAddress = {cursorDiskAddress, 0};
      index->saveToDisk(cursor, nodeSize, cursorAddress);
      return;
    }
  }

  // If we reach here, means no sibling we can steal from.
  // To resolve underflow, we must merge nodes.

  // If left sibling exists, merge with it.
  if (leftSibling >= 0)
  {
    // Load in left sibling from disk.
    Node *leftNode = (Node *)index->loadFromDisk(parent->pointers[leftSibling], nodeSize);

    // Make left node's upper bound to be cursor's lower bound.
    leftNode->keys[leftNode->numKeys] = parent->keys[leftSibling];

    // Transfer all keys from current node to left node.
    // Note: Merging will always suceed due to ⌊(n)/2⌋ (left) + ⌊(n-1)/2⌋ (current).
    for (int i = leftNode->numKeys + 1, j = 0; j < cursor->numKeys; j++)
    {
      leftNode->keys[i] = cursor->keys[j];
    }

    // Transfer all pointers too.
    Address nullAddress{nullptr, 0};
    for (int i = leftNode->numKeys + 1, j = 0; j < cursor->numKeys + 1; j++)
    {
      leftNode->pointers[i] = cursor->pointers[j];
      cursor->pointers[j] = nullAddress;
    }

    // Update variables, make left node last pointer point to the next leaf node pointed to by current.
    leftNode->numKeys += cursor->numKeys + 1;
    cursor->numKeys = 0;

    // Save left node to disk.
    index->saveToDisk(leftNode, nodeSize, parent->pointers[leftSibling]);

    // Delete current node (cursor)
    // We need to update the parent in order to fully remove the current node.
    removeInternal(parent->keys[leftSibling], (Node *)parentDiskAddress, (Node *)cursorDiskAddress);
  }
  // If left sibling doesn't exist, try to merge with right sibling.
  else if (rightSibling <= parent->numKeys)
  {
    // Load in right sibling from disk.
    Node *rightNode = (Node *)index->loadFromDisk(parent->pointers[rightSibling], nodeSize);

    // Set upper bound of cursor to be lower bound of right sibling.
    cursor->keys[cursor->numKeys] = parent->keys[rightSibling - 1];

    // Note we are moving right node's stuff into ours.
    // Transfer all keys from right node into current.
    // Note: Merging will always suceed due to ⌊(n)/2⌋ (left) + ⌊(n-1)/2⌋ (current).
    for (int i = cursor->numKeys + 1, j = 0; j < rightNode->numKeys; j++)
    {
      cursor->keys[i] = rightNode->keys[j];
    }

    // Transfer all pointers from right node into current.
    Address nullAddress = {nullptr, 0};
    for (int i = cursor->numKeys + 1, j = 0; j < rightNode->numKeys + 1; j++)
    {
      cursor->pointers[i] = rightNode->pointers[j];
      rightNode->pointers[j] = nullAddress;
    }

    // Update variables
    cursor->numKeys += rightNode->numKeys + 1;
    rightNode->numKeys = 0;

    // Save current node to disk.
    Address cursorAddress{cursorDiskAddress, 0};
    index->saveToDisk(cursor, nodeSize, cursorAddress);

    // Delete right node.
    // We need to update the parent in order to fully remove the right node.
    void *rightNodeAddress = parent->pointers[rightSibling].blockAddress;
    removeInternal(parent->keys[rightSibling - 1], (Node *)parentDiskAddress, (Node *)rightNodeAddress);
  }
}

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
