#include "memory_pool.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>

using namespace std;

int main()
{
  MemoryPool db(500000000, 100);

  // Open test data
  std::ifstream file("../data/testdata.tsv");

  if (file.is_open())
  {
    std::string line;
    int recordNum = 0;
    while (std::getline(file, line))
    {
      Record temp;
      stringstream linestream(line);
      string data;

      strcpy(temp.tconst, line.substr(0, line.find("\t")).c_str());

      std::getline(linestream, data, '\t');
      linestream >> temp.averageRating >> temp.numVotes;

      std::tuple<void *, std::size_t> record = db.allocate(sizeof(temp));

      memcpy(std::get<0>(record) + std::get<1>(record), &temp, sizeof(temp));

      cout << "Reading line " << recordNum + 1 << " of data" << '\n';
      cout << "Size of a record: " << sizeof(temp) << " bytes" << '\n';
      cout << "Inserted record " << recordNum + 1 << " at address: " << std::get<0>(record) + std::get<1>(record) << '\n';

      Record check = *(Record *)(std::get<0>(record) + std::get<1>(record));
      cout << check.tconst << '\n';

      recordNum += 1;
    }
    file.close();
  };

  cout << "Current blocks used: " << db.getAllocated() << " blocks" << '\n';
  cout << "Actual size used: " << db.getActualSizeUsed() << " bytes" << '\n';
  cout << "Total size occupied: " << db.getSizeUsed() << " bytes" << '\n';
  return 0;
}