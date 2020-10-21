#include "b_plus_tree.h"
#include "types.h"

#include <vector>
#include <cstring>
#include <iostream>

using namespace std;

void BPlusTree::search(float lowerBoundKey, float upperBoundKey)
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
    Address rootDiskAddress{rootAddress, 0};
    root = (Node *)index->loadFromDisk(rootDiskAddress, nodeSize);

    // for displaying to output file
    std::cout << "Index node accessed. Content is -----";
    displayNode(root);

    Node *cursor = root;

    bool found = false;

    // While we haven't hit a leaf node, and haven't found a range.
    while (cursor->isLeaf == false)
    {
      // Iterate through each key in the current node. We need to load nodes from the disk whenever we want to traverse to another node.
      for (int i = 0; i < cursor->numKeys; i++)
      {
        // If lowerBoundKey is lesser than current key, go to the left pointer's node to continue searching.
        if (lowerBoundKey < cursor->keys[i])
        {
          // Load node from disk to main memory.
          cursor = (Node *)index->loadFromDisk(cursor->pointers[i], nodeSize);

          // for displaying to output file
          std::cout << "Index node accessed. Content is -----";
          displayNode(cursor);

          break;
        }
        // If we reached the end of all keys in this node (larger than all), then go to the right pointer's node to continue searching.
        if (i == cursor->numKeys - 1)
        {
          // Load node from disk to main memory.
          // Set cursor to the child node, now loaded in main memory.
          cursor = (Node *)index->loadFromDisk(cursor->pointers[i + 1], nodeSize);

          // for displaying to output file
          std::cout << "Index node accessed. Content is -----";
          displayNode(cursor);
          break;
        }
      }
    }

    // When we reach here, we have hit a leaf node corresponding to the lowerBoundKey.
    // Again, search each of the leaf node's keys to find a match.
    // vector<Record> results;
    // unordered_map<void *, void *> loadedBlocks; // Maintain a reference to all loaded blocks in main memory.

    // Keep searching whole range until we find a key that is out of range.
    bool stop = false;

    while (stop == false)
    {
      int i;
      for (i = 0; i < cursor->numKeys; i++)
      {
        // Found a key within range, now we need to iterate through the entire range until the upperBoundKey.
        if (cursor->keys[i] > upperBoundKey)
        {
          stop = true;
          break;
        }
        if (cursor->keys[i] >= lowerBoundKey && cursor->keys[i] <= upperBoundKey)
        {
          // for displaying to output file
          std::cout << "Index node (LLNode) accessed. Content is -----";
          displayNode(cursor);

          // Add new line for each leaf node's linked list printout.
          std::cout << endl;
          std::cout << "LLNode: tconst for average rating: " << cursor->keys[i] << " > ";          

          // Access the linked list node and print records.
          displayLL(cursor->pointers[i]);
        }
      }

      // On the last pointer, check if last key is max, if it is, stop. Also stop if it is already equal to the max
      if (cursor->pointers[cursor->numKeys].blockAddress != nullptr && cursor->keys[i] != upperBoundKey)
      {
        // Set cursor to be next leaf node (load from disk).
        cursor = (Node *)index->loadFromDisk(cursor->pointers[cursor->numKeys], nodeSize);

        // for displaying to output file
        std::cout << "Index node accessed. Content is -----";
        displayNode(cursor);

      }
      else
      {
        stop = true;
      }
    }
  }
  return;
}