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
  std::cout <<"=========================================================================================="<<endl;
  std::cout <<"Select Block size:           "<<endl;
  int choice=0;
  while(choice!= 1 && choice != 2){
    std::cout <<"Enter a choice: "<<endl;
    std::cout <<"1. 100 B "<<endl;
    std::cout <<"2. 500 B"<<endl;
    cin >> choice;
    if(int(choice) == 1){
      BLOCKSIZE = int(100);
    }else if (int(choice) == 2){
      BLOCKSIZE = int(500);
    }else{
    cin.clear();
    std::cout <<"Invalid input, input either 1 or 2"<<endl;
    }
  }
  // =============================================================
  // Experiment 1:
  // Store the data (which is about IMDb movives and described in Part 4) on the disk and report the following statistics:
  // - The number of blocks;
  // - The size of database;
  // =============================================================
  
  // Create memory pools for the disk and the index, total 500MB
  std::cout << "creating the disk on the stack for records, index" << endl;
  // MemoryPool disk(100000, BLOCKSIZE);  
  // MemoryPool index(100000, BLOCKSIZE);  
  MemoryPool disk(150000000, BLOCKSIZE);  
  MemoryPool index(350000000, BLOCKSIZE);

  // Creating the tree 
  BPlusTree tree = BPlusTree(BLOCKSIZE, &disk, &index);
  std::cout << "Max keys for a B+ tree node: " << tree.getMaxKeys() << endl;

  // Reset the number of blocks accessed to zero
  disk.resetBlocksAccessed();
  index.resetBlocksAccessed();
  std::cout << "Number of record blocks accessed in search operation reset to: 0" << endl;
  std::cout << "Number of index blocks accessed in search operation reset to: 0" << endl;    


  // Open test data
  std::cout <<"Reading in test data ... "<<endl;
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
  
  std::cout <<"=================================Main Menu=============================================="<<endl;
    
    choice=0;
    while(choice!= 1 && choice != 2 && choice!= 3 && choice!= 4 && choice !=5 && choice != 6){
      std::cout <<"Enter a choice: "<<endl;
      std::cout <<"1. View Experiment 1 results (Database Information)                     :"<<endl;
      std::cout <<"2. View Experiment 2 results (B+ Tree Information)                      :"<<endl;
      std::cout <<"3. View Experiment 3 results (Retrieve movies with averageRating 8)     :"<<endl;
      std::cout <<"4. View Experiment 4 results (Retrieve movies with averageRating 7 to 9):"<<endl;
      std::cout <<"5. View Experiment 5 results (Delete movies with averageRating 7)       :"<<endl;
      std::cout <<"6. Exit "<<endl;
      cin >> choice;
      if(int(choice)==1){
        // call experiment 1
        std::cout <<"=====================================Experiment 1=========================================="<<endl;
        std::cout << "Number of records per record block --- " << BLOCKSIZE / sizeof(Record) << endl;
        std::cout << "Number of keys per index block --- " << tree.getMaxKeys() << endl;
        std::cout << "Number of record blocks --- " << disk.getAllocated() << endl;
        std::cout << "Number of index blocks --- " << index.getAllocated() << endl;
        std::cout << "Size of actual record data stored --- " << disk.getActualSizeUsed() << endl;
        std::cout << "Size of actual index data stored --- " << index.getActualSizeUsed() << endl;
        std::cout << "Size of record blocks --- " << disk.getSizeUsed() << endl;
        std::cout << "Size of index blocks --- " << index.getSizeUsed() << endl;
        std::cout <<"Total number of blocks   : "<<disk.getAllocated() + index.getAllocated()<<endl;
        std::cout <<"Actual size of database : "<<disk.getActualSizeUsed() + index.getActualSizeUsed()<<endl;
        std::cout <<"Size of database (size of all blocks): "<<disk.getSizeUsed()+index.getSizeUsed()<<endl;
        std::cout <<"Press any integer to go back to menu"<<endl;
        cin >> choice;
        choice =0;
        continue;
      }else if(int(choice) == 2){
        // call experiment 2
        std::cout <<"=====================================Experiment 2=========================================="<<endl;
        std::cout <<"Parameter n of the B+ tree    : "<<tree.getMaxKeys()<<endl;
        std::cout <<"Number of nodes of the B+ tree: "<<tree.getNumNodes()<<endl;
        std::cout <<"Height of the B+ tree         : "<<tree.getLevels()<<endl;
        std::cout << "Root nodes and child nodes :"<<endl;
        tree.display(tree.getRoot(),1);
        std::cout <<endl;
        std::cout <<"Enter any integer to go back to menu"<<endl;
        cin >> choice;
        choice =0;
        continue;
      }else if (int(choice) == 3){
        // call experiment 3
        std::cout <<"=====================================Experiment 3=========================================="<<endl;
        std::cout <<"Retrieving the attribute tconst of those movies with averageRating equal to 8..."<<endl;
        std::cout <<"Number and content of index blocks the process accesses: "<<index.resetBlocksAccessed()<<endl; 
        std::cout <<"Number and content of record blocks the process accesses: "<<disk.resetBlocksAccessed()<<endl;
        std::cout <<"Attribute tconst of the records that are returned   : "<<endl;
        tree.search(8.0,8.0);
        std::cout << "\nNo more records found for range " << 8.0 << " to " << 8.0 << endl;
        std::cout <<"Enter any integer to go back to menu"<<endl;
        cin >> choice;
        choice =0;
        continue;
      }else if (int(choice) == 4){
        // call experiment 4
        std::cout <<"=====================================Experiment 4=========================================="<<endl;
        std::cout <<"Retrieving the attribute tconst of those movies with averageRating from 7 to 9 (inclusively)..."<<endl;
        std::cout <<"Number and content of index blocks the process accesses: "<<index.resetBlocksAccessed()<<endl; 
        std::cout <<"Number and content of data blocks the process accesses: "<<disk.resetBlocksAccessed()<<endl;
        std::cout <<"Attribute tconst of the records that are returned   : "<<endl;
        tree.search(7,9);
        std::cout <<endl;
        std::cout <<"Enter any integer to go back to menu"<<endl;
        cin >> choice;
        choice =0;
        continue;
      }else if (int(choice) == 5){
        // call experiment 5
        std::cout <<"=====================================Experiment 5=========================================="<<endl;
        std::cout << "Original B+ Tree before deletion" << endl;
        std::cout << "Number of nodes in B+ Tree --- " << tree.getNumNodes() << endl;
        std::cout << "Height of tree --- " << tree.getLevels() << endl;
        std::cout << endl;
        tree.display(tree.getRoot(), 1);
        std::cout << endl;
        std::cout<<"Deleting those movies with the attribute averageRating equal to 7...\n";

        tree.remove(7.0);

        std::cout << "B+ Tree after deletion" << endl;
        std::cout <<"Number of times that a node is deleted (or two nodes are merged): "<<tree.getNumNodesDeleted()<<endl; 
        std::cout << "Number of nodes in updated B+ Tree --- " << tree.getNumNodes() << endl;
        std::cout << "Height of updated B+ tree --- " << tree.getLevels() << endl;
        std::cout << endl;
        tree.display(tree.getRoot(), 1);
        std::cout << endl;
        
        std::cout <<"Enter any integer to go back to menu"<<endl;
        cin >> choice;
        choice =0;
        continue;
      }else if (int(choice) == 6){
        std::cout <<"Exiting...";
        break;
      }else{
        cin.clear();
        std::cout <<"Invalid input, input 1 to 6\n";
      }
    }

  // std::cout << "\n\n================ TREEPRINT ================\n";
  // tree.display(tree.getRoot(), 1);
  // std::cout << "\n================ END OF REPORT ================\n\n";



  // std::cout << "\n\n================ SIZE REPORT ================\n";
  // std::cout << "Number of record blocks --- " << disk.getAllocated() << endl;
  // std::cout << "Size of record blocks --- " << disk.getSizeUsed() << endl;
  // std::cout << "Size of actual record data stored --- " << disk.getActualSizeUsed() << endl;
  // std::cout << "Number of records per record block --- " << BLOCKSIZE / sizeof(Record) << endl;

  // std::cout << endl;

  // std::cout << "Number of index blocks --- " << index.getAllocated() << endl;
  // std::cout << "Size of index blocks --- " << index.getSizeUsed() << endl;
  // std::cout << "Size of actual index data stored --- " << index.getActualSizeUsed() << endl;
  // std::cout << "Number of keys per index block --- " << tree.getMaxKeys() << endl;
  // std::cout << "\n================ END OF REPORT ================\n\n";




  // std::cout << "\n\n================ INSERT REPORT ================\n";
  // std::cout << "Insertion complete " << endl;
  // std::cout << "Record blocks accessed --- " << disk.resetBlocksAccessed() << endl;
  // std::cout << "Index blocks accessed --- " << index.resetBlocksAccessed() << endl;
  // std::cout << "\n================ END OF REPORT ================\n\n";







  // std::cout << "\n\n================ SEARCH REPORT ================\n";
  // tree.search(0, 10);  
  // std::cout << "\nNo more records found for range " << 0 << " to " << 10 << endl;
  // std::cout << "Record blocks accessed --- " << disk.resetBlocksAccessed() << endl;
  // std::cout << "Index blocks accessed --- " << index.resetBlocksAccessed() << endl;
  // std::cout << "\n================ END OF REPORT ================\n\n";



  // std::cout << "\n\n================ DELETE REPORT ================\n";
  // std::cout << "Original B+ Tree before deletion" << endl;
  // std::cout << "Height of tree --- " << tree.getLevels() << endl;
  // std::cout << "Number of nodes in B+ Tree --- " << tree.getNumNodes() << endl;
  // std::cout << endl;
  // tree.display(tree.getRoot(), 1);
  // std::cout << endl;

  // tree.remove(7.0);  

  // std::cout << "B+ Tree after deletion" << endl;
  // std::cout << "Height of tree --- " << tree.getLevels() << endl;
  // std::cout << "Number of nodes in B+ Tree --- " << tree.getNumNodes() << endl;
  // std::cout << endl;
  // tree.display(tree.getRoot(), 1);
  // std::cout << endl;
  // std::cout << "\n================ END OF REPORT ================\n\n";


  
  

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
