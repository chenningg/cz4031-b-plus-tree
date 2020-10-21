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
  int BLOCKSIZE=0;
  std::cerr <<"=========================================================================================="<<endl;
  std::cerr <<"Select Block size:           "<<endl;
  int choice=0;
  while(choice!= 1 && choice != 2){
    std::cerr <<"Enter a choice: "<<endl;
    std::cerr <<"1. 100 B "<<endl;
    std::cerr <<"2. 500 B"<<endl;
    cin >> choice;
    if(int(choice) == 1){
      BLOCKSIZE = int(100);
    }else if (int(choice) == 2){
      BLOCKSIZE = int(500);
    }else{
    cin.clear();
    std::cerr <<"Invalid input, input either 1 or 2"<<endl;
    }
  }
  // =============================================================
  // Experiment 1:
  // Store the data (which is about IMDb movives and described in Part 4) on the disk and report the following statistics:
  // - The number of blocks;
  // - The size of database;
  // =============================================================
  
  // Create memory pools for the disk and the index, total 500MB
  std::cerr << "creating the disk on the stack for records, index" << endl;
  // MemoryPool disk(100000, BLOCKSIZE);  
  // MemoryPool index(100000, BLOCKSIZE);  
  MemoryPool disk(150000000, BLOCKSIZE);  
  MemoryPool index(350000000, BLOCKSIZE);

  // Creating the tree 
  BPlusTree tree = BPlusTree(BLOCKSIZE, &disk, &index);
  std::cerr << "Max keys for a B+ tree node: " << tree.getMaxKeys() << endl;

  // Reset the number of blocks accessed to zero
  disk.resetBlocksAccessed();
  index.resetBlocksAccessed();
  std::cerr << "Number of record blocks accessed in search operation reset to: 0" << endl;
  std::cerr << "Number of index blocks accessed in search operation reset to: 0" << endl;    


  // Open test data
  std::cerr <<"Reading in test data ... "<<endl;
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
      // cout << "Inserted record " << recordNum + 1 << " at block address: " << &tempAddress.blockAddress << " and offset " << &tempAddress.offset << endl;
      recordNum += 1;
    }
    file.close();
  }
  
  std::cerr <<"=================================Main Menu=============================================="<<endl;
    
    choice=0;
    while(choice!= 1 && choice != 2 && choice!= 3 && choice!= 4 && choice !=5 && choice != 6){
      std::cerr <<"Enter a choice: "<<endl;
      std::cerr <<"1. View Experiment 1 results (Database Information)                     :"<<endl;
      std::cerr <<"2. View Experiment 2 results (B+ Tree Information)                      :"<<endl;
      std::cerr <<"3. View Experiment 3 results (Retrieve movies with averageRating 8)     :"<<endl;
      std::cerr <<"4. View Experiment 4 results (Retrieve movies with averageRating 7 to 9):"<<endl;
      std::cerr <<"5. View Experiment 5 results (Delete movies with averageRating 7)       :"<<endl;
      std::cerr <<"6. Exit "<<endl;
      cin >> choice;
      if(int(choice)==1){
        // call experiment 1
        std::cerr <<"=====================================Experiment 1=========================================="<<endl;
        std::cerr << "Number of records per record block --- " << BLOCKSIZE / sizeof(Record) << endl;
        std::cerr << "Number of keys per index block --- " << tree.getMaxKeys() << endl;
        std::cerr << "Number of record blocks --- " << disk.getAllocated() << endl;
        std::cerr << "Number of index blocks --- " << index.getAllocated() << endl;
        std::cerr << "Size of actual record data stored --- " << disk.getActualSizeUsed() << endl;
        std::cerr << "Size of actual index data stored --- " << index.getActualSizeUsed() << endl;
        std::cerr << "Size of record blocks --- " << disk.getSizeUsed() << endl;
        std::cerr << "Size of index blocks --- " << index.getSizeUsed() << endl;
        std::cerr <<"Total number of blocks   : "<<disk.getAllocated() + index.getAllocated()<<endl;
        std::cerr <<"Actual size of database : "<<disk.getActualSizeUsed() + index.getActualSizeUsed()<<endl;
        std::cerr <<"Size of database (size of all blocks): "<<disk.getSizeUsed()+index.getSizeUsed()<<endl;
        std::cerr <<"Press any integer to go back to menu"<<endl;
        cin >> choice;
        choice =0;
        continue;
      }else if(int(choice) == 2){
        // call experiment 2
        std::cerr <<"=====================================Experiment 2=========================================="<<endl;
        std::cerr <<"Parameter n of the B+ tree    : "<<tree.getMaxKeys()<<endl;
        std::cerr <<"Number of nodes of the B+ tree: "<<tree.getNumNodes()<<endl;
        std::cerr <<"Height of the B+ tree         : "<<tree.getLevels()<<endl;
        std::cerr << "Root nodes and child nodes :"<<endl;
        tree.display(tree.getRoot(),1);
        std::cerr <<endl;
        std::cerr <<"Enter any integer to go back to menu"<<endl;
        cin >> choice;
        choice =0;
        continue;
      }else if (int(choice) == 3){
        // call experiment 3
        std::cerr <<"=====================================Experiment 3=========================================="<<endl;
        std::cerr <<"Retrieving the attribute “tconst” of those movies with averageRating equal to 8..."<<endl;
        std::cerr <<"Number and content of index blocks the process accesses: "<<index.resetBlocksAccessed()<<endl; 
        std::cerr <<"Number and content of record blocks the process accesses: "<<disk.resetBlocksAccessed()<<endl;
        std::cerr <<"Attribute “tconst” of the records that are returned   : "<<endl;
        tree.search(8.0,8.0);
        std::cerr << "\nNo more records found for range " << 8.0 << " to " << 8.0 << endl;
        std::cerr <<"Enter any integer to go back to menu"<<endl;
        cin >> choice;
        choice =0;
        continue;
      }else if (int(choice) == 4){
        // call experiment 4
        std::cerr <<"=====================================Experiment 4=========================================="<<endl;
        std::cerr <<"Retrieving the attribute “tconst” of those movies with averageRating from 7 to 9 (inclusively)..."<<endl;
        std::cerr <<"Number and content of index blocks the process accesses: "<<index.resetBlocksAccessed()<<endl; 
        std::cerr <<"Number and content of data blocks the process accesses: "<<disk.resetBlocksAccessed()<<endl;
        std::cerr <<"Attribute “tconst” of the records that are returned   : "<<endl;
        tree.search(7,9);
        std::cerr <<endl;
        std::cerr <<"Enter any integer to go back to menu"<<endl;
        cin >> choice;
        choice =0;
        continue;
      }else if (int(choice) == 5){
        // call experiment 5
        std::cerr <<"=====================================Experiment 5=========================================="<<endl;
        std::cerr << "Original B+ Tree before deletion" << endl;
        std::cerr << "Number of nodes in B+ Tree --- " << tree.getNumNodes() << endl;
        std::cerr << "Height of tree --- " << tree.getLevels() << endl;
        std::cerr << endl;
        tree.display(tree.getRoot(), 1);
        std::cerr << endl;
        std::cerr<<"Deleting those movies with the attribute “averageRating” equal to 7...\n";

        tree.remove(7.0);

        std::cerr << "B+ Tree after deletion" << endl;
        std::cerr <<"Number of times that a node is deleted (or two nodes are merged): "<<tree.getNumNodesDeleted()<<endl; 
        std::cerr << "Number of nodes in updated B+ Tree --- " << tree.getNumNodes() << endl;
        std::cerr << "Height of updated B+ tree --- " << tree.getLevels() << endl;
        std::cerr << endl;
        tree.display(tree.getRoot(), 1);
        std::cerr << endl;
        
        std::cerr <<"Enter any integer to go back to menu"<<endl;
        cin >> choice;
        choice =0;
        continue;
      }else if (int(choice) == 6){
        std::cerr <<"Exiting...";
        break;
      }else{
        cin.clear();
        std::cerr <<"Invalid input, input 1 to 6\n";
      }
    }

  // std::cerr << "\n\n================ TREEPRINT ================\n";
  // tree.display(tree.getRoot(), 1);
  // std::cerr << "\n================ END OF REPORT ================\n\n";



  // std::cerr << "\n\n================ SIZE REPORT ================\n";
  // std::cerr << "Number of record blocks --- " << disk.getAllocated() << endl;
  // std::cerr << "Size of record blocks --- " << disk.getSizeUsed() << endl;
  // std::cerr << "Size of actual record data stored --- " << disk.getActualSizeUsed() << endl;
  // std::cerr << "Number of records per record block --- " << BLOCKSIZE / sizeof(Record) << endl;

  // std::cerr << endl;

  // std::cerr << "Number of index blocks --- " << index.getAllocated() << endl;
  // std::cerr << "Size of index blocks --- " << index.getSizeUsed() << endl;
  // std::cerr << "Size of actual index data stored --- " << index.getActualSizeUsed() << endl;
  // std::cerr << "Number of keys per index block --- " << tree.getMaxKeys() << endl;
  // std::cerr << "\n================ END OF REPORT ================\n\n";




  // std::cerr << "\n\n================ INSERT REPORT ================\n";
  // std::cerr << "Insertion complete " << endl;
  // std::cerr << "Record blocks accessed --- " << disk.resetBlocksAccessed() << endl;
  // std::cerr << "Index blocks accessed --- " << index.resetBlocksAccessed() << endl;
  // std::cerr << "\n================ END OF REPORT ================\n\n";







  // std::cerr << "\n\n================ SEARCH REPORT ================\n";
  // tree.search(0, 10);  
  // std::cerr << "\nNo more records found for range " << 0 << " to " << 10 << endl;
  // std::cerr << "Record blocks accessed --- " << disk.resetBlocksAccessed() << endl;
  // std::cerr << "Index blocks accessed --- " << index.resetBlocksAccessed() << endl;
  // std::cerr << "\n================ END OF REPORT ================\n\n";



  // std::cerr << "\n\n================ DELETE REPORT ================\n";
  // std::cerr << "Original B+ Tree before deletion" << endl;
  // std::cerr << "Height of tree --- " << tree.getLevels() << endl;
  // std::cerr << "Number of nodes in B+ Tree --- " << tree.getNumNodes() << endl;
  // std::cerr << endl;
  // tree.display(tree.getRoot(), 1);
  // std::cerr << endl;

  // tree.remove(7.0);  

  // std::cerr << "B+ Tree after deletion" << endl;
  // std::cerr << "Height of tree --- " << tree.getLevels() << endl;
  // std::cerr << "Number of nodes in B+ Tree --- " << tree.getNumNodes() << endl;
  // std::cerr << endl;
  // tree.display(tree.getRoot(), 1);
  // std::cerr << endl;
  // std::cerr << "\n================ END OF REPORT ================\n\n";


  
  

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
