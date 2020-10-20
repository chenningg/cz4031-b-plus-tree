#include "b_plus_tree.h"
#include "types.h"

#include <iostream>
#include <cstring>

using namespace std;

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

void BPlusTree::displayLL(Address LLHeadAddress)
{
  // Load linked list head into main memory.
  Node *head = (Node *)index->loadFromDisk(LLHeadAddress, nodeSize);

  // Print all records in the linked list.
  if (head == nullptr)
  {
    std::cerr << "\nEnd of linked list!\n";
  }
  else
  {
    for (int i = 0; i < head->numKeys; i++)
    {
      // Load the block from disk.
      Record result = *(Record *)(disk->loadFromDisk(head->pointers[i], sizeof(Record)));
      std::cerr << result.tconst << " | ";
    }

    // Print empty slots
    for (int i = head->numKeys; i < maxKeys; i++)
    {
      std::cerr << "x | ";
    }

    // Move to next node in linked list.
    if (head->pointers[head->numKeys].blockAddress != nullptr)
    {
      displayLL(head->pointers[head->numKeys]);
    }
  }
}
