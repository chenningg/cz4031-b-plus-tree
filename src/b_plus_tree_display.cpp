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
  std::cout << "|";
  for (int i = 0; i < node->numKeys; i++)
  {
    std::cout << node->pointers[i].blockAddress << " | ";
    std::cout << node->keys[i] << " | ";
  }

  // Print last filled pointer
  if (node->pointers[node->numKeys].blockAddress == nullptr) {
    std::cout << " Null |";
  }
  else {
    std::cout << node->pointers[node->numKeys].blockAddress << "|";
  }

  for (int i = node->numKeys; i < maxKeys; i++)
  {
    std::cout << " x |";      // Remaining empty keys
    std::cout << "  Null  |"; // Remaining empty pointers
  }

  std::cout << endl;
}

// Display a block and its contents in the disk. Assume it's already loaded in main memory.
void BPlusTree::displayBlock(void *blockAddress)
{
  // Load block into memory
  void *block = operator new(nodeSize);
  std::memcpy(block, blockAddress, nodeSize);

  unsigned char testBlock[nodeSize];
  memset(testBlock, '\0', nodeSize);

  // Block is empty.
  if (memcmp(testBlock, block, nodeSize) == 0)
  {
    std::cout << "Empty block!" << '\n';
    return;
  }

  unsigned char *blockChar = (unsigned char *)block;

  int i = 0;
  while (i < nodeSize)
  {
    // Load each record
    void *recordAddress = operator new(sizeof(Record));
    std::memcpy(recordAddress, blockChar, sizeof(Record));

    Record *record = (Record *)recordAddress;

    std::cout << "[" << record->tconst << "|" << record->averageRating << "|" << record->numVotes << "]  ";
    blockChar += sizeof(Record);
    i += sizeof(Record);
  }
  
}

// Print the tree
void BPlusTree::display(Node *cursorDiskAddress, int level)
{
  // Load in cursor from disk.
  Address cursorMainMemoryAddress{cursorDiskAddress, 0};
  Node *cursor = (Node *)index->loadFromDisk(cursorMainMemoryAddress, nodeSize);

  // If tree exists, display all nodes.
  if (cursor != nullptr)
  {
    for (int i = 0; i < level; i++)
    {
      std::cout << "   ";
    }
    std::cout << " level " << level << ": ";

    displayNode(cursor);

    if (cursor->isLeaf != true)
    {
      for (int i = 0; i < cursor->numKeys + 1; i++)
      {
        // Load node in from disk to main memory.
        Node *mainMemoryNode = (Node *)index->loadFromDisk(cursor->pointers[i], nodeSize);

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
  for (int i = 0; i < head->numKeys; i++)
  {
    // Load the block from disk.
    // void *blockMainMemoryAddress = operator new(nodeSize);
    // std::memcpy(blockMainMemoryAddress, head->pointers[i].blockAddress, nodeSize);

    std::cout << "\nData block accessed. Content is -----";
    displayBlock(head->pointers[i].blockAddress);
    std::cout << endl;

    Record result = *(Record *)(disk->loadFromDisk(head->pointers[i], sizeof(Record)));
    std::cout << result.tconst << " | ";


  }

  // Print empty slots
  for (int i = head->numKeys; i < maxKeys; i++)
  {
    std::cout << "x | ";
  }
  
  // End of linked list
  if (head->pointers[head->numKeys].blockAddress == nullptr)
  {
    std::cout << "End of linked list" << endl;
    return;
  }

  // Move to next node in linked list.
  if (head->pointers[head->numKeys].blockAddress != nullptr)
  {
    displayLL(head->pointers[head->numKeys]);
  }
}
