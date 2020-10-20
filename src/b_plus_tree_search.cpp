#include "b_plus_tree.h"
#include "types.h"

#include <vector>
#include <cstring>
#include <iostream>

using namespace std;

vector<Record> BPlusTree::search(float lowerBoundKey, float upperBoundKey)
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