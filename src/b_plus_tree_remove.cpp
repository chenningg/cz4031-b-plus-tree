#include "b_plus_tree.h"
#include "types.h"

#include <vector>
#include <cstring>
#include <iostream>

using namespace std;

int BPlusTree::remove(float key)
{
  // set numNodes before deletion
  numNodes = index->getAllocated();

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
    int leftSibling, rightSibling; // Index of left and right child to borrow from.

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
      std::cout << "Can't find specified key " << key << " to delete!" << endl;
      
      // update numNodes and numNodesDeleted after deletion
      int numNodesDeleted = numNodes - index->getAllocated();
      numNodes = index->getAllocated();
      return numNodesDeleted;
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

    // // Change the key removed to empty float
    // for (int i = cursor->numKeys; i < maxKeys; i++) {
    //   cursor->keys[i] = float();
    // }

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
        std::cout << "Congratulations! You deleted the entire index!" << endl;

        // Deallocate block used to store root node.
        Address rootDiskAddress{rootAddress, 0};
        index->deallocate(rootDiskAddress, nodeSize);

        // Reset root pointers in the B+ Tree.
        root = nullptr;
        rootAddress = nullptr;
        
      }
      std::cout << "Successfully deleted " << key << endl;
      
      // update numNodes and numNodesDeleted after deletion
      int numNodesDeleted = numNodes - index->getAllocated();
      numNodes = index->getAllocated();

      // Save to disk.
      Address cursorAddress = {cursorDiskAddress, 0};
      index->saveToDisk(cursor, nodeSize, cursorAddress);
      
      return numNodesDeleted;
    }

    // If we didn't delete from root, we check if we have minimum keys ⌊(n+1)/2⌋ for leaf.
    if (cursor->numKeys >= (maxKeys + 1) / 2)
    {
      // No underflow, so we're done.
      std::cout << "Successfully deleted " << key << endl;

      // update numNodes and numNodesDeleted after deletion
      int numNodesDeleted = numNodes - index->getAllocated();
      numNodes = index->getAllocated();

      // Save to disk.
      Address cursorAddress = {cursorDiskAddress, 0};
      index->saveToDisk(cursor, nodeSize, cursorAddress);

      return numNodesDeleted;
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
    
        // update numNodes and numNodesDeleted after deletion
        int numNodesDeleted = numNodes - index->getAllocated();
        numNodes = index->getAllocated();
        return numNodesDeleted;
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

        // update numNodes and numNodesDeleted after deletion
        int numNodesDeleted = numNodes - index->getAllocated();
        numNodes = index->getAllocated();
        return numNodesDeleted;        
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

  // update numNodes and numNodesDeleted after deletion
  int numNodesDeleted = numNodes - index->getAllocated();
  numNodes = index->getAllocated();
  return numNodesDeleted;
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
        std::cout << "Root node changed." << endl;
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
        std::cout << "Root node changed." << endl;
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
    std::cout << "End of linked list";
    return;
  }

  if (head->pointers[head->numKeys].blockAddress != nullptr)
  {

    removeLL(head->pointers[head->numKeys]);
  }
}
