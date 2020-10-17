#include "memory_pool.h"

#include <iostream>

// int main()
// {
//   MemoryPool db(500000000, 100);

//   std::cout << "Pool size: " << db.getPoolSize() << " bytes" << '\n';
//   std::cout << "Block size: " << db.getBlockSize() << " bytes" << '\n';

//   Movie test = {false, 5.6, 12312, "tt0000001"};
  // Movie test2 = {false, 7.0, 6246472, "tt0000002"};
//   std::cout << "Size of a record: " << sizeof(test) << " bytes" << '\n';

//   std::cout << "Current blocks available: " << db.getAvailable() << " blocks" << '\n';
//   std::cout << "Inserting 9 instances of test (expect 3 blocks to be used):" << '\n';
//   std::cout << "Inserted record 1 at: " << reinterpret_cast<void *>(db.allocate(test)) << '\n';
//   std::cout << "Inserted record 2 at: " << reinterpret_cast<void *>(db.allocate(test)) << '\n';
//   std::cout << "Inserted record 3 at: " << reinterpret_cast<void *>(db.allocate(test2)) << '\n';
//   std::cout << "Inserted record 4 at: " << reinterpret_cast<void *>(db.allocate(test)) << '\n';
//   std::cout << "Inserted record 5 at: " << reinterpret_cast<void *>(db.allocate(test)) << '\n';
//   std::cout << "Inserted record 6 at: " << reinterpret_cast<void *>(db.allocate(test2)) << '\n';
//   std::cout << "Inserted record 7 at: " << reinterpret_cast<void *>(db.allocate(test)) << '\n';
//   std::cout << "Inserted record 8 at: " << reinterpret_cast<void *>(db.allocate(test2)) << '\n';
//   std::cout << "Inserted record 9 at: " << reinterpret_cast<void *>(db.allocate(test)) << '\n';
//   std::cout << "Current blocks available: " << db.getAvailable() << " blocks" << '\n';
//   std::cout << "Size used: " << db.getSizeUsed() << " bytes" << '\n';
//   return 0;
// }


#include <fstream>
#include <string>
#include <sstream>

using namespace std;

int main()
{
  MemoryPool db(500000000, 100);

  cout << "Pool size: " << db.getPoolSize() << " bytes" << '\n';
  cout << "Block size: " << db.getBlockSize() << " bytes" << '\n';
  std::ifstream file("../data/testdata.tsv");
  if (file.is_open()) {
    std::string line;
    int recordNum = 0;
    while (std::getline(file, line)) {
        float averageRating;
        int numVotes;
        char tconst;
        Movie temp={false,averageRating,numVotes,tconst};
        stringstream linestream(line);
        string data;
        strcpy(temp.tconst,line.substr(0,line.find("\t")).c_str());
        std::getline(linestream, data, '\t');
        linestream>>temp.averageRating>>temp.numVotes;
        cout <<"Reading line " <<recordNum +1<<" of data"<<'\n';
        cout << "Size of a record: " << sizeof(temp) << " bytes" << '\n';
        cout << "Inserted record "<< recordNum <<" at: " << reinterpret_cast<void *>(db.allocate(temp)) << '\n';
        recordNum +=1;
        // cout<<temp.tconst<<","<<temp.averageRating<<","<<temp.numVotes<<'\n';
    }
    file.close();
  }
  cout << "Current blocks available: " << db.getAvailable() << " blocks" << '\n';
  cout << "Size used: " << db.getSizeUsed() << " bytes" << '\n';
  return 0;
}