#include "b_plus_tree.h"
#include "types.h"

#include <vector>
#include <cstring>
#include <iostream>

using namespace std;

// Insert a record into the B+ Tree index. Key: Record's avgRating, Value: {blockAddress, offset}.
void BPlusTree::insert(Address address, float key)
{
  // If no root exists, create a new B+ Tree root.
  if (rootAddress == nullptr)
  {
    // Create a new linked list (for duplicates) at the key.
    Node *LLNode = new Node(maxKeys);
    LLNode->keys[0] = key;
    LLNode->isLeaf = false; // So we will never search it
    LLNode->numKeys = 1;
    LLNode->pointers[0] = address; // The disk address of the key just inserted

    // Allocate LLNode and root address
    Address LLNodeAddress = index->saveToDisk((void *)LLNode, nodeSize);

    // Create new node in main memory, set it to root, and add the key and values to it.
    root = new Node(maxKeys);
    root->keys[0] = key;
    root->isLeaf = true; // It is both the root and a leaf.
    root->numKeys = 1;
    root->pointers[0] = LLNodeAddress; // Add record's disk address to pointer.

    // Write the root node into disk and track of root node's disk address.
    rootAddress = index->saveToDisk(root, nodeSize).blockAddress;
  }
  // Else if root exists already, traverse the nodes to find the proper place to insert the key.
  else
  {
    Node *cursor = root;
    Node *parent;                          // Keep track of the parent as we go deeper into the tree in case we need to update it.
    void *parentDiskAddress = rootAddress; // Keep track of parent's disk address as well so we can update parent in disk.
    void *cursorDiskAddress = rootAddress; // Store current node's disk address in case we need to update it in disk.

    // While not leaf, keep following the nodes to correct key.
    while (cursor->isLeaf == false)
    {

      // Set the parent of the node (in case we need to assign new child later), and its disk address.
      parent = cursor;
      parentDiskAddress = cursorDiskAddress;

      // Check through all keys of the node to find key and pointer to follow downwards.
      for (int i = 0; i < cursor->numKeys; i++)
      {
        // If key is lesser than current key, go to the left pointer's node.
        if (key < cursor->keys[i])
        {
          // Load node in from disk to main memory.
          Node *mainMemoryNode = (Node *)index->loadFromDisk(cursor->pointers[i], nodeSize);

          // Update cursorDiskAddress to maintain address in disk if we need to update nodes.
          cursorDiskAddress = cursor->pointers[i].blockAddress;

          // Move to new node in main memory.
          cursor = mainMemoryNode;
          break;
        }
        // Else if key larger than all keys in the node, go to last pointer's node (rightmost).
        if (i == cursor->numKeys - 1)
        {
          // Load node in from disk to main memory.
          Node *mainMemoryNode = (Node *)index->loadFromDisk(cursor->pointers[i + 1], nodeSize);

          // Update diskAddress to maintain address in disk if we need to update nodes.
          cursorDiskAddress = cursor->pointers[i + 1].blockAddress;

          // Move to new node in main memory.
          cursor = (Node *)mainMemoryNode;
          break;
        }
      }
    }

    // When we reach here, it means we have hit a leaf node. Let's find a place to put our new record in.
    // If this leaf node still has space to insert a key, then find out where to put it.
    if (cursor->numKeys < maxKeys)
    {
      int i = 0;
      // While we haven't reached the last key and the key we want to insert is larger than current key, keep moving forward.
      while (key > cursor->keys[i] && i < cursor->numKeys)
      {
        i++;
      }

      // i is where our key goes in. Check if it's already there (duplicate).
      if (cursor->keys[i] == key)
      {
        // If it's a duplicate, linked list already exists. Insert into linked list.
        // Insert and update the linked list head.
        cursor->pointers[i] = insertLL(cursor->pointers[i], address, key);
      }
      else
      {
        // Update the last pointer to point to the previous last pointer's node. Aka maintain cursor -> Y linked list.
        Address next = cursor->pointers[cursor->numKeys];

        // Now i represents the index we want to put our key in. We need to shift all keys in the node back to fit it in.
        // Swap from number of keys + 1 (empty key) backwards, moving our last key back and so on. We also need to swap pointers.
        for (int j = cursor->numKeys; j > i; j--)
        {
          // Just do a simple bubble swap from the back to preserve index order.
          cursor->keys[j] = cursor->keys[j - 1];
          cursor->pointers[j] = cursor->pointers[j - 1];
        }

        // Insert our new key and pointer into this node.
        cursor->keys[i] = key;

        // We need to make a new linked list to store our record.
        // Create a new linked list (for duplicates) at the key.
        Node *LLNode = new Node(maxKeys);
        LLNode->keys[0] = key;
        LLNode->isLeaf = false; // So we will never search it
        LLNode->numKeys = 1;
        LLNode->pointers[0] = address; // The disk address of the key just inserted

        // Allocate LLNode into disk.
        Address LLNodeAddress = index->saveToDisk((void *)LLNode, nodeSize);

        // Update variables
        cursor->pointers[i] = LLNodeAddress;
        cursor->numKeys++;
  
        // Update leaf node pointer link to next node
        cursor->pointers[cursor->numKeys] = next;

        // Now insert operation is complete, we need to store this updated node to disk.
        // cursorDiskAddress is the address of node in disk, cursor is the address of node in main memory.
        // In this case, we count read/writes as 1/O only (Assume block remains in main memory).
        Address cursorOriginalAddress{cursorDiskAddress, 0};
        index->saveToDisk(cursor, nodeSize, cursorOriginalAddress);
      }
    }
    // Overflow: If there's no space to insert new key, we have to split this node into two and update the parent if required.
    else
    {
      // Create a new leaf node to put half the keys and pointers in.
      Node *newLeaf = new Node(maxKeys);

      // Copy all current keys and pointers (including new key to insert) to a temporary list.
      float tempKeyList[maxKeys + 1];

      // We only need to store pointers corresponding to records (ignore those that points to other nodes).
      // Those that point to other nodes can be manipulated by themselves without this array later.
      Address tempPointerList[maxKeys + 1];
      Address next = cursor->pointers[cursor->numKeys];

      // Copy all keys and pointers to the temporary lists.
      int i = 0;
      for (i = 0; i < maxKeys; i++)
      {
        tempKeyList[i] = cursor->keys[i];
        tempPointerList[i] = cursor->pointers[i];
      }

      // Insert the new key into the temp key list, making sure that it remains sorted. Here, we find where to insert it.
      i = 0;
      while (key > tempKeyList[i] && i < maxKeys)
      {
        i++;
      }

      // KIVVVVVVVVVV OUT OF RANGE

      // i is where our key goes in. Check if it's already there (duplicate).
      // make sure it is not the last one 
      if (i < cursor->numKeys) {
        if (cursor->keys[i] == key)
        {
          // If it's a duplicate, linked list already exists. Insert into linked list.
          // Insert and update the linked list head.
          cursor->pointers[i] = insertLL(cursor->pointers[i], address, key);
          return;
        } 
      }

      // Else no duplicate, insert new key.
      // The key should be inserted at index i in the temporary lists. Move all elements back.
      for (int j = maxKeys; j > i; j--)
      {
        // Bubble swap all elements (keys and pointers) backwards by one index.
        tempKeyList[j] = tempKeyList[j - 1];
        tempPointerList[j] = tempPointerList[j - 1];
      }

      // Insert the new key and pointer into the temporary lists.
      tempKeyList[i] = key;

      // The address to insert will be a new linked list node.
      // Create a new linked list (for duplicates) at the key.
      Node *LLNode = new Node(maxKeys);
      LLNode->keys[0] = key;
      LLNode->isLeaf = false; // So we will never search it
      LLNode->numKeys = 1;
      LLNode->pointers[0] = address; // The disk address of the key just inserted

      // Allocate LLNode into disk.
      Address LLNodeAddress = index->saveToDisk((void *)LLNode, nodeSize);
      tempPointerList[i] = LLNodeAddress;
      
      newLeaf->isLeaf = true; // New node is a leaf node.

      // Split the two new nodes into two. ⌊(n+1)/2⌋ keys for left, n+1 - ⌊(n+1)/2⌋ (aka remaining) keys for right.
      cursor->numKeys = (maxKeys + 1) / 2;
      newLeaf->numKeys = (maxKeys + 1) - ((maxKeys + 1) / 2);

      // Set the last pointer of the new leaf node to point to the previous last pointer of the existing node (cursor).
      // Essentially newLeaf -> Y, where Y is some other leaf node pointer wherein cursor -> Y previously.
      // We use maxKeys since cursor was previously full, so last pointer's index is maxKeys.
      newLeaf->pointers[newLeaf->numKeys] = next;

      // Now we need to deal with the rest of the keys and pointers.
      // Note that since we are at a leaf node, pointers point directly to records on disk.

      // Add in keys and pointers in both the existing node, and the new leaf node.
      // First, the existing node (cursor).
      for (i = 0; i < cursor->numKeys; i++)
      {
        cursor->keys[i] = tempKeyList[i];
        cursor->pointers[i] = tempPointerList[i];
      }

      // Then, the new leaf node. Note we keep track of the i index, since we are using the remaining keys and pointers.
      for (int j = 0; j < newLeaf->numKeys; i++, j++)
      {
        newLeaf->keys[j] = tempKeyList[i];
        newLeaf->pointers[j] = tempPointerList[i];
      }

      // Now that we have finished updating the two new leaf nodes, we need to write them to disk.
      Address newLeafAddress = index->saveToDisk(newLeaf, nodeSize);

      // Now to set the cursors' pointer to the disk address of the leaf and save it in place
      cursor->pointers[cursor->numKeys] = newLeafAddress;

      // wipe out the wrong pointers and keys from cursor
      for (int i = cursor->numKeys; i < maxKeys; i++) {
        cursor->keys[i] = float();
      }
      for (int i = cursor->numKeys+1; i < maxKeys + 1; i++) {
        Address nullAddress{nullptr, 0};
        cursor->pointers[i] = nullAddress;
      }

      Address cursorOriginalAddress{cursorDiskAddress, 0};
      index->saveToDisk(cursor, nodeSize, cursorOriginalAddress);

      // If we are at root (aka root == leaf), then we need to make a new parent root.
      if (cursor == root)
      {
        Node *newRoot = new Node(maxKeys);

        // We need to set the new root's key to be the left bound of the right child.
        newRoot->keys[0] = newLeaf->keys[0];

        // Point the new root's children as the existing node and the new node.
        Address cursorDisk{cursorDiskAddress, 0};

        newRoot->pointers[0] = cursorDisk;
        newRoot->pointers[1] = newLeafAddress;

        // Update new root's variables.
        newRoot->isLeaf = false;
        newRoot->numKeys = 1;

        // Write the new root node to disk and update the root disk address stored in B+ Tree.
        Address newRootAddress = index->saveToDisk(newRoot, nodeSize);

        // Update the root address
        rootAddress = newRootAddress.blockAddress;
        root = newRoot;
      }
      // If we are not at the root, we need to insert a new parent in the middle levels of the tree.
      else
      {
        insertInternal(newLeaf->keys[0], (Node *)parentDiskAddress, (Node *)newLeafAddress.blockAddress);
      }
    }
  }

  // update numnodes 
  numNodes = index->getAllocated();
}

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

    // Split the two new nodes into two. ⌊(n)/2⌋ keys for left.
    // For right, we drop the rightmost key since we only need to represent the pointer.
    cursor->numKeys = (maxKeys + 1) / 2;
    newInternal->numKeys = maxKeys - (maxKeys + 1) / 2;

    // Reassign keys into cursor from tempkeyslist to account for new child node
    for (int i = 0; i < cursor->numKeys; i++)
    {
      cursor->keys[i] = tempKeyList[i];
    }
    
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

    // KIVVVVVVV

    // Get rid of unecessary cursor keys and pointers
    for (int i = cursor->numKeys; i < maxKeys; i++) 
    {
      cursor->keys[i] = float();
    }

    for (int i = cursor->numKeys + 1; i < maxKeys + 1; i++)
    {
      Address nullAddress{nullptr, 0};
      cursor->pointers[i] = nullAddress;
    }

    // assign the new child to the original parent
    cursor->pointers[cursor->numKeys] = childAddress;

    // Save the old parent and new internal node to disk.
    Address cursorAddress{cursorDiskAddress, 0};
    index->saveToDisk(cursor, nodeSize, cursorAddress);

    // Address newInternalAddress{newInternal, 0};
    Address newInternalDiskAddress = index->saveToDisk(newInternal, nodeSize);

    // If current cursor is the root of the tree, we need to create a new root.
    if (cursor == root)
    {
      Node *newRoot = new Node(nodeSize);
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

      // KIVVVVV
      // insertInternal(cursor->keys[cursor->numKeys], parentDiskAddress, (Node *)newInternalDiskAddress.blockAddress);
      insertInternal(tempKeyList[cursor->numKeys], parentDiskAddress, (Node *)newInternalDiskAddress.blockAddress);
    }
  }
}

// Inserts a record into an existing linked list.
Address BPlusTree::insertLL(Address LLHead, Address address, float key)
{
  // Load the linked list head node into main memory.
  Node *head = (Node *)index->loadFromDisk(LLHead, nodeSize);

  // Check if the head node has space to put record.
  if (head->numKeys < maxKeys)
  {

    // Move all keys back to insert at the head.
    for (int i = head->numKeys; i > 0; i--)
    {
      head->keys[i] = head->keys[i - 1];
    }

    // Move all pointers back to insert at the head.
    for (int i = head->numKeys + 1; i > 0; i--)

    {
      head->pointers[i] = head->pointers[i - 1];
    }

    // Insert new record into the head of linked list.
    head->keys[0] = key;
    head->pointers[0] = address; // the disk address of the key just inserted
    head->numKeys++;
    
    // Write head back to disk.
    LLHead = index->saveToDisk((void *)head, nodeSize, LLHead);

    // Return head address
    return LLHead;
  }
  // No space in head node, need a new linked list node.
  else
  {
    // Make a new node and add variables
    Node *LLNode = new Node(maxKeys);
    LLNode->isLeaf = false;
    LLNode->keys[0] = key;
    LLNode->numKeys = 1;

    // Insert key into head of linked list node.
    LLNode->pointers[0] = address;

    // Now this node is head of linked list, point to the previous head's disk address as next.
    LLNode->pointers[1] = LLHead;

    // Write new linked list node to disk.
    Address LLNodeAddress = index->saveToDisk((void *)LLNode, nodeSize);

    // Return disk address of new linked list head
    return LLNodeAddress;
  }
}