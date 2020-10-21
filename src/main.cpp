#include "memory_pool.h"
#include "b_plus_tree.h"
#include "types.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <unordered_map>

using namespace std;

int main()
{
  // =============================================================
  // Experiment 1:
  // Store the data (which is about IMDb movives and described in Part 4) on the disk and report the following statistics:
  // - The number of blocks;
  // - The size of database;
  // =============================================================
  
  // Create memory pools for the disk and the index, total 500MB
  std::cerr << "creating the disk on the stack for records, index" << endl;
  MemoryPool disk(100000, 100);  
  MemoryPool index(100000, 100);  
  // MemoryPool disk(250000000, 100);  
  // MemoryPool index(250000000, 100);

  // Creating the tree 
  BPlusTree tree = BPlusTree(100, &disk, &index);
  std::cerr << "Max keys for a B+ tree node: " << tree.getMaxKeys() << endl;

  // Reset the number of blocks accessed to zero
  disk.resetBlocksAccessed();
  index.resetBlocksAccessed();
  std::cerr << "Number of record blocks accessed in search operation reset to: 0" << endl;
  std::cerr << "Number of index blocks accessed in search operation reset to: 0" << endl;    


  // Open test data
  std::ifstream file("../data/testdata.tsv");
  // std::ifstream file("../data/data.tsv");

  // Insert data into database and populate list of addresses
  if (file.is_open())
  {
    std::string line;
    int recordNum = 0;
    while (std::getline(file, line))
    {
      //temporary struct Record
      Record temp;
      stringstream linestream(line);
      string data;

      //assigning temp.tconst value
      strcpy(temp.tconst, line.substr(0, line.find("\t")).c_str());
      std::getline(linestream, data, '\t');

      //assigning temp.averageRating & temp.numVotes values
      linestream >> temp.averageRating >> temp.numVotes;

      //insert this record into the database
      Address tempAddress = disk.saveToDisk(&temp, sizeof(Record));

      //build the bplustree as we insert records
      tree.insert(tempAddress, float(temp.averageRating));

      //logging
      cout << "Inserted record " << recordNum + 1 << " at block address: " << &tempAddress.blockAddress << " and offset " << &tempAddress.offset << endl;
      recordNum += 1;
    }
    file.close();

    // tree.display(tree.getRoot(), 1);
    // cout<<"recordNum: "<<recordNum<<'\n';
    // cout<<"max Keys: "<<tree.getMaxKeys()<<'\n';
    // cout<<"tree levels: "<<tree.getLevels()<<'\n';
    // cout<<"number of nodes: "<<tree.getNumNodes()<<'\n';
  }
  // cout << "Number of blocks used: " << disk.getAllocated() << " blocks" << '\n';
  // cout << "Actual size used: " << disk.getActualSizeUsed() << " bytes" << '\n';
  // cout << "Total size occupied: " << disk.getSizeUsed() << " bytes" << '\n';


  std::cerr << "\n\n================ TREEPRINT ================\n";
  tree.display(tree.getRoot(), 1);
  std::cerr << "Record blocks accessed --- " << disk.resetBlocksAccessed() << endl;
  std::cerr << "Index blocks accessed --- " << index.resetBlocksAccessed() << endl;
  std::cerr << "\n================ END OF REPORT ================\n\n";





  std::cerr << "\n\n================ INSERT REPORT ================\n";
  std::cerr << "Insertion complete " << endl;
  std::cerr << "Record blocks accessed --- " << disk.resetBlocksAccessed() << endl;
  std::cerr << "Index blocks accessed --- " << index.resetBlocksAccessed() << endl;
  std::cerr << "\n================ END OF REPORT ================\n\n";





  // Reset the number of blocks accessed to zero
  disk.resetBlocksAccessed();
  index.resetBlocksAccessed();
  std::cerr << "Number of record blocks accessed in search operation reset to: 0" << endl;
  std::cerr << "Number of index blocks accessed in search operation reset to: 0" << endl;    

  tree.search(0, 10);

  std::cerr << "\n\n================ SEARCH REPORT ================\n";
  std::cerr << "\nNo more records found for range " << 0 << " to " << 10 << endl;
  std::cerr << "Record blocks accessed --- " << disk.resetBlocksAccessed() << endl;
  std::cerr << "Index blocks accessed --- " << index.resetBlocksAccessed() << endl;
  std::cerr << "\n================ END OF REPORT ================\n\n";



  // call experiment 2
  // call experiment 3
  // call experiment 4
  // call experiment 5

  // call experiment 1-5 with 500B block size
  

  return 0;
}



  // // // =============================================================
  // // // Experiment 2:
  // // // Build a B+ tree on the attribute "averageRating" by inserting the records sequentially and report the following statistics:
  // // // - The parameter n of the B + tree;
  // // // - The number of nodes of the B + tree;
  // // // - The height of the B + tree, i.e., the number of levels of the B + tree;
  // // // - The root node and its child nodes(actual content);
  // // // =============================================================

  // // // Here, we are trying to fake the overhead of loading from disk to main memory.
  // // // Therefore, we copy out the data from disk to a heap in main memory.

  // // // Create a map to detect which blocks we have already loaded into main memory.
  // // // The key is the block's address (on disk), the value is the block's address (in main memory).
  // // unordered_map<void *, void *> loadedBlocks;

  // // // Iterate through all records stored in database.
  // // for (Record = records.begin(); it != records.end(); ++it)
  // // {
  // //   // This mimics the loading of disk data to main memory, pointing at the disk block.
  // //   void *blockAddress = std::get<0>(*it);
  // //   std::size_t offset = std::get<1>(*it);

  // //   // Check if this block hasn't been loaded yet.
  // //   if (loadedBlocks.find(blockAddress) == loadedBlocks.end())
  // //   {
  // //     void *mainMemoryBlock = operator new(disk.getBlockSize());
  // //     memcpy(mainMemoryBlock, blockAddress, disk.getBlockSize());

  // //     loadedBlocks[blockAddress] = mainMemoryBlock;
  // //   }

  // //   // If block has been loaded, just read it.
  // //   void *recordAddress = loadedBlocks.at(blockAddress) + offset;

  // //   std::cout << (*(Record *)recordAddress).tconst << " at " << recordAddress << '\n';
  // // }

  // // // =============================================================
  // // // Experiment 3:
  // // // Retrieve the attribute “tconst” of those movies with the "averageRating" equal to 8 and report the following statistics:
  // // // - The number and the content of index nodes the process accesses;
  // // // - The number and the content of data blocks the process accesses;
  // // // - The attribute “tconst” of the records that are returned;
  // // // =============================================================

  // // // =============================================================
  // // // Experiment 3:
  // // // Retrieve the attribute “tconst” of those movies with the "averageRating" equal to 8 and report the following statistics:
  // // // - The number and the content of index nodes the process accesses;
  // // // - The number and the content of data blocks the process accesses;
  // // // - The attribute “tconst” of the records that are returned;
  // // // =============================================================

  // // // =============================================================
  // // // Experiment 4:
  // // // Retrieve the attribute “tconst” of those movies with the attribute “averageRating” from 7 to 9, both inclusively
  // // // and report the following statistics:
  // // // - The number and the content of index nodes the process accesses;
  // // // - The number and the content of data blocks the process accesses;
  // // // - The attribute “tconst” of the records that are returned;
  // // // =============================================================

  // // // =============================================================
  // // // Experiment 5:
  // // // Delete those movies with the attribute “averageRating” equal to 7, update the B + tree accordingly,
  // // // and report the following statistics:
  // // // - The number of times that a node is deleted(or two nodes are merged) during the process of the updating the B + tree;
  // // // - The number nodes of the updated B + tree;
  // // // - The height of the updated B + tree;
  // // // - The root node and its child nodes of the updated B + tree;
  // // // =============================================================
