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
    std::vector<Record> records;
  };

  // Constructor
  DB();

  // Methods
  // Add new block
  void addBlock();

  // Remove block
  void removeBlock();

  // Destructor
  ~DB();

private:
  // Stores all blocks
  void *store;
};

#endif