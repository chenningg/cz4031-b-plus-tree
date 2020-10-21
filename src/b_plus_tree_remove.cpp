#include "b_plus_tree.h"
#include "types.h"

#include <vector>
#include <cstring>
#include <iostream>

using namespace std;

void BPlusTree::remove(float key)
{
  // Tree is empty.
  if (rootAddress == nullptr)
  {
    throw std::logic_error("Tree is empty!");
  }
  else
  {
    // Load in root from the disk.
    Address rootDiskAddress{rootAddress, 0};
    root = (Node *)index->loadFromDisk(rootDiskAddress, nodeSize);

    Node *cursor = root;
    Node *parent;                          // Keep track of the parent as we go deeper into the tree in case we need to update it.
    void *parentDiskAddress = rootAddress; // Keep track of parent's disk address as well so we can update parent in disk.
    void *cursorDiskAddress = rootAddress; // Store current node's disk address in case we need to update it in disk.
    int leftSibling, rightSibling;         // Index of left and right child to borrow from.

    // While not leaf, keep following the nodes to correct key.
    while (cursor->isLeaf == false)
    {
      // Set the parent of the node (in case we need to assign new child later), and its disk address.
      parent = cursor;
      parentDiskAddress = cursorDiskAddress;

      // Check through all keys of the node to find key and pointer to follow downwards.
      for (int i = 0; i < cursor->numKeys; i++)
      {
        // Keep track of left and right to borrow.
        leftSibling = i - 1;
        rightSibling = i + 1;

        // If key is lesser than current key, go to the left pointer's node.
        if (key < cursor->keys[i])
        {
          // Load node in from disk to main memory.
          Node *mainMemoryNode = (Node *)index->loadFromDisk(cursor->pointers[i], nodeSize);

          // Update cursorDiskAddress to maintain address in disk if we need to update nodes.
          cursorDiskAddress = cursor->pointers[i].blockAddress;

          // Move to new node in main memory.
          cursor = (Node *)mainMemoryNode;
          break;
        }
        // Else if key larger than all keys in the node, go to last pointer's node (rightmost).
        if (i == cursor->numKeys - 1)
        {
          leftSibling = i;
          rightSibling = i + 2;

          // Load node in from disk to main memory.
          Node *mainMemoryNode = (Node *)index->loadFromDisk(cursor->pointers[i + 1], nodeSize);

          // Update cursorDiskAddress to maintain address in disk if we need to update nodes.
          cursorDiskAddress = cursor->pointers[i + 1].blockAddress;

          // Move to new node in main memory.
          cursor = (Node *)mainMemoryNode;
          break;
        }
      }
    }

    // now that we have found the leaf node that might contain the key, we will try and find the position of the key here (if exists)
    // search if the key to be deleted exists in this bplustree
    bool found = false;
    int pos;
    // also works for duplicates
    for (pos = 0; pos < cursor->numKeys; pos++)
    {
      if (cursor->keys[pos] == key)
      {
        found = true;
        break;
      }
    }

    // If key to be deleted does not exist in the tree, return error.
    if (!found)
    {
      std::cerr << "Can't find specified key " << key << " to delete!" << endl;
      return;
    }

    // pos is the position where we found the key.
    // We must delete the entire linked-list before we delete the key, otherwise we lose access to the linked list head.
    // Delete the linked list stored under the key.
    removeLL(cursor->pointers[pos]);

    // Now, we can delete the key. Move all keys/pointers forward to replace its values.
    for (int i = pos; i < cursor->numKeys; i++)
    {
      cursor->keys[i] = cursor->keys[i + 1];
      cursor->pointers[i] = cursor->pointers[i + 1];
    }

    cursor->numKeys--;

    // Move the last pointer forward (if any).
    cursor->pointers[cursor->numKeys] = cursor->pointers[cursor->numKeys + 1];

    // Set all forward pointers from numKeys onwards to nullptr.
    for (int i = cursor->numKeys + 1; i < maxKeys + 1; i++)
    {
      Address nullAddress{nullptr, 0};
      cursor->pointers[i] = nullAddress;
    }

    // If current node is root, check if tree still has keys.
    if (cursor == root)
    {
      if (cursor->numKeys == 0)
      {
        // Delete the entire root node and deallocate it.
        std::cerr << "Congratulations! You deleted the entire index!" << endl;

        // Deallocate block used to store root node.
        Address rootDiskAddress{rootAddress, 0};
        index->deallocate(rootDiskAddress, nodeSize);

        // Reset root pointers in the B+ Tree.
        root = nullptr;
        rootAddress = nullptr;
      }
      std::cerr << "Successfully deleted " << key << endl;
      return;
    }

    // If we didn't delete from root, we check if we have minimum keys ⌊(n+1)/2⌋ for leaf.
    if (cursor->numKeys >= (maxKeys + 1) / 2)
    {
      // No underflow, so we're done.
      std::cerr << "Successfully deleted " << key << endl;
      return;
    }

    // If we reach here, means we have underflow (not enough keys for balanced tree).
    // Try to take from left sibling (node on same level) first.
    // Check if left sibling even exists.
    if (leftSibling >= 0)
    {
      // Load in left sibling from disk.
      Node *leftNode = (Node *)index->loadFromDisk(parent->pointers[leftSibling], nodeSize);

      // Check if we can steal (ahem, borrow) a key without underflow.
      if (leftNode->numKeys >= (maxKeys + 1) / 2 + 1)
      {
        // We will insert this borrowed key into the leftmost of current node (smaller).

        // Shift last pointer back by one first.
        cursor->pointers[cursor->numKeys + 1] = cursor->pointers[cursor->numKeys];

        // Shift all remaining keys and pointers back by one.
        for (int i = cursor->numKeys; i > 0; i--)
        {
          cursor->keys[i] = cursor->keys[i - 1];
          cursor->pointers[i] = cursor->pointers[i - 1];
        }

        // Transfer borrowed key and pointer (rightmost of left node) over to current node.
        cursor->keys[0] = leftNode->keys[leftNode->numKeys - 1];
        cursor->pointers[0] = leftNode->pointers[leftNode->numKeys - 1];
        cursor->numKeys++;
        leftNode->numKeys--;

        // Update left sibling (shift pointers left)
        leftNode->pointers[cursor->numKeys] = leftNode->pointers[cursor->numKeys + 1];

        // Update parent node's key
        parent->keys[leftSibling] = cursor->keys[0];

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
      if (rightNode->numKeys >= (maxKeys + 1) / 2 + 1)
      {

        // We will insert this borrowed key into the rightmost of current node (larger).
        // Shift last pointer back by one first.
        cursor->pointers[cursor->numKeys + 1] = cursor->pointers[cursor->numKeys];

        // No need to shift remaining pointers and keys since we are inserting on the rightmost.
        // Transfer borrowed key and pointer (leftmost of right node) over to rightmost of current node.
        cursor->keys[cursor->numKeys] = rightNode->keys[0];
        cursor->pointers[cursor->numKeys] = rightNode->pointers[0];
        cursor->numKeys++;
        rightNode->numKeys--;

        // Update right sibling (shift keys and pointers left)
        for (int i = 0; i < rightNode->numKeys; i++)
        {
          rightNode->keys[i] = rightNode->keys[i + 1];
          rightNode->pointers[i] = rightNode->pointers[i + 1];
        }

        // Move right sibling's last pointer left by one too.
        rightNode->pointers[cursor->numKeys] = rightNode->pointers[cursor->numKeys + 1];

        // Update parent node's key to be new lower bound of right sibling.
        parent->keys[rightSibling - 1] = rightNode->keys[0];

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

      // Transfer all keys and pointers from current node to left node.
      // Note: Merging will always suceed due to ⌊(n)/2⌋ (left) + ⌊(n-1)/2⌋ (current).
      for (int i = leftNode->numKeys, j = 0; j < cursor->numKeys; i++, j++)
      {
        leftNode->keys[i] = cursor->keys[j];
        leftNode->pointers[i] = cursor->pointers[j];
      }

      // Update variables, make left node last pointer point to the next leaf node pointed to by current.
      leftNode->numKeys += cursor->numKeys;
      leftNode->pointers[leftNode->numKeys] = cursor->pointers[cursor->numKeys];

      // Save left node to disk.
      index->saveToDisk(leftNode, nodeSize, parent->pointers[leftSibling]);

      // We need to update the parent in order to fully remove the current node.
      removeInternal(parent->keys[leftSibling], (Node *)parentDiskAddress, (Node *)cursorDiskAddress);

      // Now that we have updated parent, we can just delete the current node from disk.
      Address cursorAddress{cursorDiskAddress, 0};
      index->deallocate(cursorAddress, nodeSize);
    }
    // If left sibling doesn't exist, try to merge with right sibling.
    else if (rightSibling <= parent->numKeys)
    {
      // Load in right sibling from disk.
      Node *rightNode = (Node *)index->loadFromDisk(parent->pointers[rightSibling], nodeSize);

      // Note we are moving right node's stuff into ours.
      // Transfer all keys and pointers from right node into current.
      // Note: Merging will always suceed due to ⌊(n)/2⌋ (left) + ⌊(n-1)/2⌋ (current).
      for (int i = cursor->numKeys, j = 0; j < rightNode->numKeys; i++, j++)
      {
        cursor->keys[i] = rightNode->keys[j];
        cursor->pointers[i] = rightNode->pointers[j];
      }

      // Update variables, make current node last pointer point to the next leaf node pointed to by right node.
      cursor->numKeys += rightNode->numKeys;
      cursor->pointers[cursor->numKeys] = rightNode->pointers[rightNode->numKeys];

      // Save current node to disk.
      Address cursorAddress{cursorDiskAddress, 0};
      index->saveToDisk(cursor, nodeSize, cursorAddress);

      // We need to update the parent in order to fully remove the right node.
      void *rightNodeAddress = parent->pointers[rightSibling].blockAddress;
      removeInternal(parent->keys[rightSibling - 1], (Node *)parentDiskAddress, (Node *)rightNodeAddress);

      // Now that we have updated parent, we can just delete the right node from disk.
      Address rightNodeDiskAddress{rightNodeAddress, 0};
      index->deallocate(rightNodeDiskAddress, nodeSize);
    }
  }
}

void BPlusTree::removeLL(Address LLHeadAddress)
{
  // Load in first node from disk.
  Node *head = (Node *)index->loadFromDisk(LLHeadAddress, nodeSize);

  // Removing the current head. Simply deallocate the entire block since it is safe to do so for the linked list
  // Keep going down the list until no more nodes to deallocate.

  // Deallocate the current node.
  index->deallocate(LLHeadAddress, nodeSize);

  // End of linked list
  if (head->pointers[head->numKeys].blockAddress == nullptr)
  {
    std::cerr << "End of linked list";
    return;
  }

  if (head->pointers[head->numKeys].blockAddress != nullptr)
  {

    removeLL(head->pointers[head->numKeys]);
  }
}
