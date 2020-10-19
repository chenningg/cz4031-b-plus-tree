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
  // Initialize empty array of keys.
  this->keys = new float[maxKeys];

  // Initialize empty array of pointers (there is one more pointer than the number of keys).
  // Note that a Node pointer in this case is the same size as a block.
  // Therefore, a Node pointer is simply the Node in disk copied out to a volatile Node in main memory.
  this->pointers = new Address[maxKeys + 1];
}

BPlusTree::BPlusTree(std::size_t blockSize)
{
  // Get size left for keys and pointers in a node after accounting for node's isLeaf and numKeys attributes.
  size_t nodeBufferSize = blockSize - sizeof(bool) - sizeof(int);

  // Set max keys available in a node. Each key is a float, each pointer is a struct of {void *blockAddress, short int offset}.
  // Therefore, each key is 4 bytes. Each pointer is around 16 bytes.

  // Initialize node buffer with a pointer.
  int sum = sizeof(Address);
  int maxKeys = 0;

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
  root = nullptr;

  // Set node size to be equal to block size.
  nodeSize = blockSize;

  // Initialize initial variables
  levels = 0;
  numNodes = 0;

  // Initialize disk space for index.
  MemoryPool pool(150000000, 100);
  index = &pool;
}

vector<Record> BPlusTree::select(float lowerBoundKey, float upperBoundKey)
{
  // Tree is empty.
  if (root == nullptr)
  {
    throw std::logic_error("Tree is empty!");
  }
  // Else iterate through root node and follow the keys to find the correct key.
  else
  {
    // Set cursor to root (root must be in main memory to access B+ Tree).
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
          memcpy(mainMemoryNode, blockAddress, nodeSize);

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
          memcpy(mainMemoryNode, blockAddress, nodeSize);

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
            memcpy(mainMemoryBlock, blockAddress, nodeSize);

            // Keep track of loaded blocks so we don't have to reload them from disk.
            loadedBlocks[blockAddress] = mainMemoryBlock;

            dataBlocksAccessed += 1;
          }

          // Here, we can access the loaded block (in main memory) to get the record that fits our search range.
          // Add the corresponding record to the results list using its main memory block address + offset.
          Record result = *(Record *)(loadedBlocks[blockAddress] + offset);
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
          memcpy(mainMemoryNode, blockAddress, nodeSize);

          // Set cursor to the child node, now loaded in main memory.
          cursor = (Node *)mainMemoryNode;
          indexNodesAccessed += 1;
          break;
        }
      }
    }

    // Report on blocks searched.
    cout << "Number of index nodes accessed: " << indexNodesAccessed << '\n';
    cout << "Number of data blocks accessed: " << dataBlocksAccessed << '\n';

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
  cout << "|";
  while (i < node->numKeys)
  {
    cout << node->pointers[i].blockAddress << "|";
    cout << node->keys[i] << "|";
    i += 1;
  }

  // Print last filled pointer
  cout << node->pointers[i + 1].blockAddress << "|";

  while (i < maxKeys)
  {
    cout << " x |";      // Remaining empty keys
    cout << "  Null  |"; // Remaining empty pointers
    i += 1;
  }

  cout << '\n';
}

// Display a block and its contents in the disk. Assume it's already loaded in main memory.
void BPlusTree::displayBlock(void *block)
{
  cout << "--------------- Start block -----------------\n";
  if (*(unsigned char *)&block == '\0')
  {
    cout << "Empty block!" << '\n';
  }
  else
  {
    void *endOfBlock = &block + nodeSize;
    while (*(unsigned char *)&block != '\0' && block < endOfBlock)
    {
      Record *record = (Record *)block;

      cout << "|" << record->tconst << "|" << record->averageRating << "|" << record->numVotes << "|" << '\n';
      block = &block + sizeof(Record);
    }
  }
  cout << "---------------- End block ------------------\n";
}

// Insert a record into the B+ Tree index. Key: Record's avgRating, Value: {blockAddress, offset}.
void BPlusTree::insert(Address address, float key)
{
  // If no root exists, create a new B+ Tree root.
  if (root == nullptr)
  {
    // Create new node in main memory, set it to root, and add the key and values to it.
    root = new Node(maxKeys);
    root->keys[0] = key;
    root->isLeaf = true; // It is both the root and a leaf.
    root->numKeys = 1;
    root->pointers[0] = address; // Add record's disk address to pointer.

    // Write the node into disk.
    void *diskNode = index->allocate(nodeSize).blockAddress;
    memcpy(diskNode, (void *)root, nodeSize);

    // Update number of nodes and levels
    numNodes++;
    levels++;
  }
  // Else if root exists already, traverse the nodes to find the proper place to insert the key.
  else
  {
    Node *cursor = root;
    Node *parent;
    void *diskAddress; // Store current node's disk address in case we need to update it in disk.

    // While not leaf, keep following the nodes to correct key.
    while (cursor->isLeaf == false)
    {
      // Set the parent of the node (in case we need to assign new child later).
      parent = cursor;

      // Check through all keys of the node to find key and pointer to follow downwards.
      for (int i = 0; i < cursor->numKeys; i++)
      {
        // If key is lesser than current key, go to the left pointer's node.
        if (key < cursor->keys[i])
        {
          // Load node in from disk to main memory.
          void *mainMemoryNode = operator new(nodeSize);
          memcpy(mainMemoryNode, cursor->pointers[i].blockAddress, nodeSize);

          // Update diskAddress to maintain address in disk if we need to update nodes.
          diskAddress = cursor->pointers[i].blockAddress;

          // Move to new node in main memory.
          cursor = (Node *)mainMemoryNode;
          break;
        }
        // Else if key larger than all keys in the node, go to last pointer's node (rightmost).
        if (i == cursor->numKeys - 1)
        {
          // Load node in from disk to main memory.
          void *mainMemoryNode = operator new(nodeSize);
          memcpy(mainMemoryNode, cursor->pointers[i + i].blockAddress, nodeSize);

          // Update diskAddress to maintain address in disk if we need to update nodes.
          diskAddress = cursor->pointers[i + i].blockAddress;

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
      // Update the last pointer to point to the previous last pointer's node. Aka maintain cursor -> Y linked list.
      cursor->pointers[cursor->numKeys + 1] = cursor->pointers[cursor->numKeys];

      int i = 0;
      // While we haven't reached the last key and the key we want to insert is larger than current key, keep moving forward.
      while (key > cursor->keys[i] && i < cursor->numKeys)
      {
        i += 1;
      }

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

      // Now insert operation is complete, we need to store this updated node to disk.
      // diskAddress is the address of node in disk, cursor is the address of node in main memory.
      // In this case, we count read/writes as 1 I/O only (Assume block remains in main memory).
      memcpy(diskAddress, cursor, nodeSize);
    }
    // Overflow: If there's no space to insert new key, we have to split this node into two and update the parent if required.
    else
    {
      // Create a new leaf node to put half the keys and pointers in.
      Node *newLeaf = new Node(maxKeys);
      newLeaf->isLeaf = true; // New node is a leaf node.

      // Update nodes count
      numNodes++;

      // Copy all current keys and pointers (including new key to insert) to a temporary list.
      float tempKeyList[maxKeys + 1];
      Address tempPointerList[maxKeys + 1];

      int i = 0;
      for (i = 0; i < maxKeys; i++)
      {
        tempKeyList[i] = cursor->keys[i];
        tempPointerList[i] = cursor->pointers[i];
      }

      // Add the last pointer into the temp pointer list for good measure.
      tempPointerList[i] = cursor->pointers[i];

      // Insert the new key into the temp key list, making sure that it remains sorted.
      i = 0;
      while (key > tempKeyList[i] && i < maxKeys)
      {
        i++;
      }

      // The key should be inserted at index i in the temp key list. Move all elements back.
      for (int j = maxKeys + 1; j > i; j--)
      {
        // Bubble swap all elements backwards by one index.
        tempKeyList[j] = tempKeyList[j - 1];
      }

      // Insert the new key into the temp key list.
      tempKeyList[i] = key;

      // Split the two new nodes into two. ⌊(n+1)/2⌋ keys for left, n+1 - ⌊(n+1)/2⌋ (aka remaining) keys for right.
      cursor->numKeys = (maxKeys + 1) / 2;
      newLeaf->numKeys = (maxKeys + 1) - (maxKeys + 1) / 2;

      // Set the last pointer of the new leaf node to point to the previous last pointer of the existing node (cursor).
      // Essentially newLeaf -> Y, where Y is some other leaf node pointer wherein cursor -> Y previously.
      // We use maxKeys since cursor was previously full, so last pointer's index is maxKeys.
      newLeaf->pointers[newLeaf->numKeys] = cursor->pointers[maxKeys];

      // Set the new last pointer of the existing cursor to point to the new leaf node (linked list).
      // Effectively, it was cursor -> Y, now it's cursor -> newLeaf -> Y, where Y is some other leaf node.
      // We need to save the new leaf node to the disk and store that disk address in the pointer.
      Address newLeafAddress = index->allocate(nodeSize);
      cursor->pointers[cursor->numKeys] = newLeafAddress;

      // Now we need to deal with the rest of the keys and pointers.
      // Note that since we are at a leaf node, pointers point directly to records on disk.

      for (i = cursor->numKeys; i < maxKeys + 1; i++)
      {
        cursor->pointers[i] = NULL;
      }
      for (i = 0; i < cursor->num_keys; i++)
      {
        cursor->keys[i] = tempKeyList[i];
      }
      for (i = 0, j = cursor->num_keys; i < newLeaf->num_keys; i++, j++)
      {
        newLeaf->keys[i] = tempKeyList[j];
      }

      cursor->pointers[cursor->num_keys + 1] = newLeaf;
      if (cursor == root)
      {
        Node *newRoot = new Node;
        newRoot->keys[0] = newLeaf->keys[0];
        newRoot->pointers[0] = cursor;
        newRoot->pointers[1] = newLeaf;
        newRoot->IS_LEAF = false;
        newRoot->num_keys = 1;
        root = newRoot;
      }
      else
      {
        insertInternal(newLeaf->keys[0], parent, newLeaf);
      }
    }
  }
}

// Insert Operation
void BPTree::insertInternal(int x, Node *cursor, Node *child)
{
  if (cursor->num_keys < MAX_KEYS)
  {
    int i = 0;
    while (x > cursor->keys[i] && i < cursor->num_keys)
      i++;
    for (int j = cursor->num_keys; j > i; j--)
    {
      cursor->keys[j] = cursor->keys[j - 1];
    }
    for (int j = cursor->num_keys + 1; j > i + 1; j--)
    {
      cursor->pointers[j] = cursor->pointers[j - 1];
    }
    cursor->keys[i] = x;
    cursor->num_keys++;
    cursor->pointers[i + 1] = child;
  }
  else
  {
    Node *newInternal = new Node;
    int virtualKey[MAX_KEYS + 1];
    Node *virtualPtr[MAX_KEYS + 2];
    for (int i = 0; i < MAX_KEYS; i++)
    {
      virtualKey[i] = cursor->keys[i];
    }
    for (int i = 0; i < MAX_KEYS + 1; i++)
    {
      virtualPtr[i] = cursor->pointers[i];
    }
    int i = 0, j;
    while (x > virtualKey[i] && i < MAX_KEYS)
      i++;
    for (int j = MAX_KEYS + 1; j > i; j--)
    {
      virtualKey[j] = virtualKey[j - 1];
    }
    virtualKey[i] = x;
    for (int j = MAX_KEYS + 2; j > i + 1; j--)
    {
      virtualPtr[j] = virtualPtr[j - 1];
    }
    virtualPtr[i + 1] = child;
    newInternal->IS_LEAF = false;
    cursor->num_keys = (MAX_KEYS + 1) / 2;
    newInternal->num_keys = MAX_KEYS - (MAX_KEYS + 1) / 2;
    for (i = 0, j = cursor->num_keys + 1; i < newInternal->num_keys; i++, j++)
    {
      newInternal->keys[i] = virtualKey[j];
    }
    for (i = 0, j = cursor->num_keys + 1; i < newInternal->num_keys + 1; i++, j++)
    {
      newInternal->pointers[i] = virtualPtr[j];
    }
    if (cursor == root)
    {
      Node *newRoot = new Node;
      newRoot->keys[0] = cursor->keys[cursor->num_keys];
      newRoot->pointers[0] = cursor;
      newRoot->pointers[1] = newInternal;
      newRoot->IS_LEAF = false;
      newRoot->num_keys = 1;
      root = newRoot;
    }
    else
    {
      insertInternal(cursor->keys[cursor->num_keys], findParent(root, cursor), newInternal);
    }
  }
}

// Find the parent
Node *BPTree::findParent(Node *cursor, Node *child)
{
  Node *parent;
  if (cursor->IS_LEAF || (cursor->pointers[0])->IS_LEAF)
  {
    return NULL;
  }
  for (int i = 0; i < cursor->num_keys + 1; i++)
  {
    if (cursor->pointers[i] == child)
    {
      parent = cursor;
      return parent;
    }
    else
    {
      parent = findParent(cursor->pointers[i], child);
      if (parent != NULL)
        return parent;
    }
  }
  return parent;
}

// Print the tree
void BPTree::display(Node *cursor, int level)
{
  if (cursor != NULL)
  {
    for (int i = 0; i < level; i++)
    {
      cout << "   ";
    }
    cout << "level " << level << ": ";

    for (int i = 0; i < cursor->num_keys; i++)
    {
      cout << cursor->keys[i] << " ";
    }

    for (int i = cursor->num_keys; i < MAX_KEYS; i++)
    {
      cout << "x ";
    }

    cout << "\n";
    if (cursor->IS_LEAF != true)
    {
      for (int i = 0; i < cursor->num_keys + 1; i++)
      {
        display(cursor->pointers[i], level + 1);
      }
    }
  }
}