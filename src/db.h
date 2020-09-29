#ifndef DB_H
#define DB_H

#include <string>
#include <vector>

class DB
{
public:
  // A singular movie record with ID, avgRating and number of votes
  struct Record
  {
    char tconst[10];
    float averageRating;
    int numVotes;
  };

  // A block of fixed size (user defined below) that can fit more than one record
  struct Block
  {
    std::size_t size;
  };

  // Constructor
  DB(){

  };

private:
  // Stores all the blocks
  std::vector<DB::Record> store;
};

#endif