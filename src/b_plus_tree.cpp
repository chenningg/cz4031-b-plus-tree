#include "b_plus_tree.h"
#include "memory_pool.h"
#include "types.h"

#include <tuple>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstring>

using namespace std;

Node::Node(int maxKeys)
{
  // Initialize empty array of keys and pointers.
  this->keys = new float[maxKeys];
  this->pointers = new Address[maxKeys + 1];

  Address nullAddress{nullptr, 0};
  for (int i = 0; i < maxKeys; i++)
  {
    this->pointers[i] = nullAddress;
  }

  this->numKeys = 0;
}

BPlusTree::BPlusTree(std::size_t blockSize)
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

  // Initialize disk space for index.
  index = new MemoryPool(1500000, 100);
}

vector<Record> BPlusTree::select(float lowerBoundKey, float upperBoundKey)
{
  // Tree is empty.
  if (rootAddress == nullptr)
  {
    throw std::logic_error("Tree is empty!");
  }
  // Else iterate through root node and follow the keys to find the correct key.
  else
  {
    // Load in root from disk.
    void *rootMainMemory = operator new(nodeSize);
    std::memcpy(rootMainMemory, rootAddress, nodeSize);
    root = (Node *)rootMainMemory;
    Node *cursor = root;

    int indexNodesAccessed = 1; // Count number of index nodes accessed. Assume root already accessed.
    int dataBlocksAccessed = 0; // Count number of data blocks accessed.

    // While we haven't hit a leaf node, search for the corresponding lowerBoundKey within the cursor node's keys.
    while (cursor->isLeaf == false)
    {
      // Iterate through each key in the current node. We need to load nodes from the disk whenever we want to traverse to another node.
      for (int i = 0; i < cursor->numKeys; i++)
      {
        // If lowerBoundKey is lesser than current key, go to the left pointer's node to continue searching.
        if (lowerBoundKey < cursor->keys[i])
        {
          // Load node from disk to main memory.
          void *mainMemoryNode = operator new(nodeSize);

          // We just need to load the whole block from disk since block size = node size.
          void *blockAddress = cursor->pointers[i].blockAddress;
          std::memcpy(mainMemoryNode, blockAddress, nodeSize);

          // Set cursor to the child node, now loaded in main memory.
          cursor = (Node *)mainMemoryNode;
          indexNodesAccessed += 1;
          break;
        }
        // If we reached the end of all keys in this node (larger than all), then go to the right pointer's node to continue searching.
        if (i == cursor->numKeys - 1)
        {
          // Load node from disk to main memory.
          void *mainMemoryNode = operator new(nodeSize);

          // In this case, we load pointer + 1 (next pointer in node since the lowerBoundKey is larger than all keys in this node).
          void *blockAddress = cursor->pointers[i + 1].blockAddress;
          std::memcpy(mainMemoryNode, blockAddress, nodeSize);

          // Set cursor to the child node, now loaded in main memory.
          cursor = (Node *)mainMemoryNode;
          indexNodesAccessed += 1;
          break;
        }
      }
    }

    // When we reach here, we have hit a leaf node corresponding to the lowerBoundKey.
    // Again, search each of the leaf node's keys to find a match.
    vector<Record> results;
    unordered_map<void *, void *> loadedBlocks; // Maintain a reference to all loaded blocks in main memory.

    // Keep searching whole range until we find a key that is out of range.
    bool stop = false;
    while (!stop)
    {
      for (int i = 0; i < cursor->numKeys; i++)
      {
        // Found a key within range, now we need to iterate through the entire range until the upperBoundKey.
        if (cursor->keys[i] >= lowerBoundKey && cursor->keys[i] <= upperBoundKey)
        {
          // Check if this block hasn't been loaded from disk yet.
          void *blockAddress = (void *)(cursor->pointers[i]).blockAddress;
          short int offset = cursor->pointers[i].offset;

          if (loadedBlocks.find(blockAddress) == loadedBlocks.end())
          {
            // Load block into main memory.
            void *mainMemoryBlock = operator new(nodeSize);
            std::memcpy(mainMemoryBlock, blockAddress, nodeSize);

            // Keep track of loaded blocks so we don't have to reload them from disk.
            loadedBlocks[blockAddress] = mainMemoryBlock;

            dataBlocksAccessed += 1;
          }

          // Here, we can access the loaded block (in main memory) to get the record that fits our search range.
          // Add the corresponding record to the results list using its main memory block address + offset.
          Record result = *(Record *)((char *)(loadedBlocks[blockAddress]) + offset);
          results.push_back(result);
        }
        // If we find a key that is out of range, stop searching.
        else
        {
          stop = true;
          break;
        }

        // If we reached the end of all keys in this node, go to the next linked leaf node to keep searching.
        if (i == cursor->numKeys - 1)
        {
          // Load next node from disk to main memory.
          void *mainMemoryNode = operator new(nodeSize);

          // Load pointer + 1 (next pointer in node).
          void *blockAddress = cursor->pointers[i + 1].blockAddress;
          std::memcpy(mainMemoryNode, blockAddress, nodeSize);

          // Set cursor to the child node, now loaded in main memory.
          cursor = (Node *)mainMemoryNode;
          indexNodesAccessed += 1;
          break;
        }
      }
    }

    // Report on blocks searched.
    std::cerr << "Number of index nodes accessed: " << indexNodesAccessed << '\n';
    std::cerr << "Number of data blocks accessed: " << dataBlocksAccessed << '\n';

    // If nothing found, throw an error.
    if (results.size() < 1)
    {
      throw std::logic_error("Could not find any matching records within the given range.");
    }
    // Else return the list of records found corresponding to the search range.
    else
    {
      return results;
    }
  }
}

// Display a node and its contents in the B+ Tree.
void BPlusTree::displayNode(Node *node)
{
  // Print out all contents in the node as such |pointer|key|pointer|
  int i = 0;
  std::cerr << "|";
  for (int i = 0; i < node->numKeys; i++)
  {
    std::cerr << node->pointers[i].blockAddress << "|";
    std::cerr << node->keys[i] << "|";
  }

  // Print last filled pointer
  std::cerr << node->pointers[node->numKeys].blockAddress << "|";

  for (int i = node->numKeys; i < maxKeys; i++)
  {
    std::cerr << " x |";      // Remaining empty keys
    std::cerr << "  Null  |"; // Remaining empty pointers
  }

  std::cerr << endl;
}

// Display a block and its contents in the disk. Assume it's already loaded in main memory.
void BPlusTree::displayBlock(void *block)
{
  std::cerr << "--------------- Start block -----------------" << '\n';
  if (*(unsigned char *)&block == '\0')
  {
    std::cerr << "Empty block!" << '\n';
  }
  else
  {
    void *endOfBlock = &block + nodeSize;
    while (*(unsigned char *)&block != '\0' && block < endOfBlock)
    {
      Record *record = (Record *)block;

      std::cerr << "|" << record->tconst << "|" << record->averageRating << "|" << record->numVotes << "|" << '\n';
      block = &block + sizeof(Record);
    }
  }
  std::cerr << "---------------- End block ------------------" << '\n';
}

// Insert a record into the B+ Tree index. Key: Record's avgRating, Value: {blockAddress, offset}.
void BPlusTree::insert(Address address, float key)
{
  std::cerr << "Inserting: " << key << endl;
  // If no root exists, create a new B+ Tree root.
  if (rootAddress == nullptr)
  {
    // Create new node in main memory, set it to root, and add the key and values to it.
    root = new Node(maxKeys);
    root->keys[0] = key;
    root->isLeaf = true; // It is both the root and a leaf.
    root->numKeys = 1;
    root->pointers[0] = address; // Add record's disk address to pointer.

    // Write the root node into disk.
    void *rootDiskAddress = index->allocate(nodeSize).blockAddress;
    std::memcpy(rootDiskAddress, root, nodeSize);

    // Keep track of root node's disk address.
    rootAddress = rootDiskAddress;

    // Update number of nodes and levels
    numNodes++;
    levels++;

    std::cerr << "(root) Inserted: " << key << endl;
  }
  // Else if root exists already, traverse the nodes to find the proper place to insert the key.
  else
  {
    // Load in root from the disk
    void *rootMainMemory = operator new(nodeSize);
    std::memcpy(rootMainMemory, rootAddress, nodeSize);
    root = (Node *)rootMainMemory;

    Node *cursor = root;
    Node *parent;                          // Keep track of the parent as we go deeper into the tree in case we need to update it.
    void *parentDiskAddress = rootAddress; // Keep track of parent's disk address as well so we can update parent in disk.
    void *cursorDiskAddress = rootAddress; // Store current node's disk address in case we need to update it in disk.

    for (int i = 0; i < cursor->numKeys; i++)
    {
      std::cerr << cursor->keys[i] << "__";
    }
    std::cerr << endl;

    // While not leaf, keep following the nodes to correct key.
    while (cursor->isLeaf == false)
    {

      // Set the parent of the node (in case we need to assign new child later), and its disk address.
      parent = cursor;
      parentDiskAddress = cursorDiskAddress;

      std::cerr << "Finding where to put in key " << key << endl;
      for (int i = 0; i < cursor->numKeys; i++)
      {
        std::cerr << cursor->keys[i] << "_";
      }

      std::cerr << endl;

      for (int i = 0; i < cursor->numKeys + 1; i++)
      {
        std::cerr << cursor->pointers[i].blockAddress << "_";
      }

      std::cerr << endl;

      // Check through all keys of the node to find key and pointer to follow downwards.
      for (int i = 0; i < cursor->numKeys; i++)
      {
        // If key is lesser than current key, go to the left pointer's node.
        if (key < cursor->keys[i])
        {
          // Load node in from disk to main memory.
          void *mainMemoryNode = operator new(nodeSize);
          std::memcpy(mainMemoryNode, cursor->pointers[i].blockAddress, nodeSize);

          // Update cursorDiskAddress to maintain address in disk if we need to update nodes.
          cursorDiskAddress = cursor->pointers[i].blockAddress;

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

          // Update diskAddress to maintain address in disk if we need to update nodes.
          cursorDiskAddress = cursor->pointers[i + 1].blockAddress;

          std::cerr << "Going to address: " << cursor->pointers[i + 1].blockAddress;

          // Move to new node in main memory.
          cursor = (Node *)mainMemoryNode;

          std::cerr << "isLeaf:" << cursor->isLeaf << endl;

          break;
        }
      }
    }

    // When we reach here, it means we have hit a leaf node. Let's find a place to put our new record in.
    // If this leaf node still has space to insert a key, then find out where to put it.
    if (cursor->numKeys < maxKeys)
    {
      std::cerr << "Node to insert in: " << cursor->keys[0] << endl;

      std::cerr << "Inserting....." << key << endl;

      std::cerr << "cursor keys...." << endl;
      for (int i = 0; i < cursor->numKeys; i++)
      {
        std::cerr << cursor->keys[i] << " | ";
      }
      std::cerr << endl;

      std::cerr << "cursor pointers...." << endl;
      for (int i = 0; i < cursor->numKeys + 1; i++)
      {
        std::cerr << cursor->pointers[i].blockAddress << " | ";
      }
      std::cerr << endl;

      int i = 0;
      // While we haven't reached the last key and the key we want to insert is larger than current key, keep moving forward.
      while (key > cursor->keys[i] && i < cursor->numKeys)
      {
        i++;
      }

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
      cursor->pointers[i] = address;
      cursor->numKeys++;

      // Update leaf node pointer link to next node
      cursor->pointers[cursor->numKeys] = next;

      std::cerr << "cursor keys...." << endl;
      for (int i = 0; i < cursor->numKeys; i++)
      {
        std::cerr << cursor->keys[i] << " | ";
      }
      std::cerr << endl;

      std::cerr << "cursor pointers...." << endl;
      for (int i = 0; i < cursor->numKeys + 1; i++)
      {
        std::cerr << cursor->pointers[i].blockAddress << " | ";
      }
      std::cerr << endl;

      std::cerr << "(not root) Inserted: " << key << endl;

      // Now insert operation is complete, we need to store this updated node to disk.
      // cursorDiskAddress is the address of node in disk, cursor is the address of node in main memory.
      // In this case, we count read/writes as 1/O only (Assume block remains in main memory).
      std::memcpy(cursorDiskAddress, cursor, nodeSize);
    }
    // Overflow: If there's no space to insert new key, we have to split this node into two and update the parent if required.
    else
    {
      std::cerr << "Overflow! Splitting..." << endl;

      // Create a new leaf node to put half the keys and pointers in.
      Node *newLeaf = new Node(maxKeys);

      // Update nodes count
      numNodes++;

      // Copy all current keys and pointers (including new key to insert) to a temporary list.
      float tempKeyList[maxKeys + 1];

      // We only need to store pointers corresponding to records (ignore those that points to other nodes).
      // Those that point to other nodes can be manipulated by themselves without this array later.
      Address tempPointerList[maxKeys + 2];
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

      // The key should be inserted at index i in the temporary lists. Move all elements back.
      for (int j = maxKeys; j > i; j--)
      {
        // Bubble swap all elements (keys and pointers) backwards by one index.
        tempKeyList[j] = tempKeyList[j - 1];
        tempPointerList[j] = tempPointerList[j - 1];
      }

      // Insert the new key and pointer into the temporary lists.
      tempKeyList[i] = key;
      tempPointerList[i] = address;

      newLeaf->isLeaf = true; // New node is a leaf node.

      // Split the two new nodes into two. ⌊(n+1)/2⌋ keys for left, n+1 - ⌊(n+1)/2⌋ (aka remaining) keys for right.
      cursor->numKeys = (maxKeys + 1) / 2;
      newLeaf->numKeys = (maxKeys + 1) - ((maxKeys + 1) / 2);

      // Set the last pointer of the new leaf node to point to the previous last pointer of the existing node (cursor).
      // Essentially newLeaf -> Y, where Y is some other leaf node pointer wherein cursor -> Y previously.
      // We use maxKeys since cursor was previously full, so last pointer's index is maxKeys.
      newLeaf->pointers[newLeaf->numKeys] = next;

      // Set the new last pointer of the existing cursor to point to the new leaf node (linked list).
      // Effectively, it was cursor -> Y, now it's cursor -> newLeaf -> Y, where Y is some other leaf node.
      // We need to save the new leaf node to the disk and store that disk address in the pointer.
      Address newLeafAddress = index->allocate(nodeSize);
      cursor->pointers[cursor->numKeys] = newLeafAddress;

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
      std::cerr << "Moving: " << cursorDiskAddress << endl;
      std::cerr << "Moving: " << newLeafAddress.blockAddress << endl;

      std::memcpy(cursorDiskAddress, cursor, nodeSize);

      std::memcpy(newLeafAddress.blockAddress, newLeaf, nodeSize);

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

        // Add new node and level to the tree.
        numNodes++;
        levels++;

        // Write the new root node to disk and update the root disk address stored in B+ Tree.
        Address newRootAddress = index->allocate(nodeSize);
        std::memcpy(newRootAddress.blockAddress, newRoot, nodeSize);

        rootAddress = newRootAddress.blockAddress;
        root = newRoot;

        std::cerr << "Making new root:" << newLeaf->keys[0] << endl;
      }
      // If we are not at the root, we need to insert a new parent in the middle levels of the tree.
      else
      {
        insertInternal(newLeaf->keys[0], (Node *)parentDiskAddress, (Node *)newLeafAddress.blockAddress);
      }
    }
  }
}

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
    for (int j = maxKeys + 1; j > i; j--)
    {
      tempKeyList[j] = tempKeyList[j - 1];
    }

    // Insert new key into array in the correct spot (sorted).
    tempKeyList[i] = key;

    // Move all pointers back to fit new child's pointer as well.
    for (int j = maxKeys + 2; j > i + 1; j--)
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
      Node *parentDiskAddress = findParent((Node *)rootAddress, cursorDiskAddress);
      insertInternal(cursor->keys[cursor->numKeys], parentDiskAddress, (Node *)newInternalDiskAddress.blockAddress);
    }
  }
}

// Find the parent of a node.
Node *BPlusTree::findParent(Node *cursorDiskAddress, Node *childDiskAddress)
{
  // Load in cursor into main memory
  void *cursorMainMemory = operator new(nodeSize);
  std::memcpy(cursorMainMemory, cursorDiskAddress, nodeSize);
  Node *cursor = (Node *)cursorMainMemory;

  std::cerr << "findParent root:" << endl;
  for (int i = 0; i < cursor->numKeys; i++)
  {
    std::cerr << cursor->keys[i] << " | ";
  }
  std::cerr << endl;

  // If the root cursor passed in is a leaf node, there is no children, therefore no parent.
  if (cursor->isLeaf)
  {
    return nullptr;
  }

  // Load the cursor's first child (if any). If it's a leaf, an error must have occured.
  // Since we do not call this function for insertInternal on a level 2 node.
  void *tempChild = operator new(nodeSize);
  std::memcpy(tempChild, cursor->pointers[0].blockAddress, nodeSize);

  if (((Node *)tempChild)->isLeaf)
  {
    return nullptr;
  }

  // Maintain parentDiskAddress
  Node *parentDiskAddress = cursorDiskAddress;

  // Iterate through all pointers of current cursor to find child.
  for (int i = 0; i < cursor->numKeys + 1; i++)
  {
    // Check if any pointers match the child's disk address we are looking for.
    if (cursor->pointers[i].blockAddress == childDiskAddress)
    {
      // If it matches, then we have found the parent.
      return parentDiskAddress;
    }
    // If don't match, we need to recursively search children.
    else
    {
      // Load the child from disk and pass in its main memory address.
      parentDiskAddress = findParent((Node *)cursor->pointers[i].blockAddress, childDiskAddress);
      if (parentDiskAddress != nullptr)
      {
        return parentDiskAddress;
      }
    }
  }

  // Catch all case
  return parentDiskAddress;
}

// Print the tree
void BPlusTree::display(Node *cursorDiskAddress, int level)
{
  // Load in cursor from disk.
  void *cursorMainMemory = operator new(nodeSize);
  std::memcpy(cursorMainMemory, cursorDiskAddress, nodeSize);
  Node *cursor = (Node *)cursorMainMemory;

  // If tree exists, display all nodes.
  if (cursor != nullptr)
  {
    for (int i = 0; i < level; i++)
    {
      std::cerr << "   ";
    }
    std::cerr << " level " << level << ": ";

    displayNode(cursor);

    std::cerr << "\n";
    if (cursor->isLeaf != true)
    {
      for (int i = 0; i < cursor->numKeys + 1; i++)
      {
        // Load node in from disk to main memory.
        void *mainMemoryNode = operator new(nodeSize);
        std::memcpy(mainMemoryNode, cursor->pointers[i].blockAddress, nodeSize);

        display((Node *)mainMemoryNode, level + 1);
      }
    }
  }
}

void b_plus_tree_test()
{
  // Create memory pools for the disk.
  BPlusTree tree = BPlusTree(100);
  std::cerr << "Max keys: " << tree.getMaxKeys() << endl;

  MemoryPool *test = new MemoryPool(1000000, 100);

  for (int i = 1; i < 15; i++)
  {
    Record record = {"tt000001", 1.0, 80};
    Address addr = test->allocate(sizeof(record));
    std::memcpy(addr.blockAddress, &record, sizeof(record));
    tree.insert(addr, float(i));
  }

  // tree.display(tree.getRoot(), 1);
}