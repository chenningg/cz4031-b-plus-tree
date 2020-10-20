#ifndef TYPES_H
#define TYPES_H

// Defines an address of a record stored as a block address with an offset.
struct Address
{
  void *blockAddress;
  short int offset;
};

// Defines a single movie record (read from data file).
struct Record
{
  char tconst[10];     // ID of the movie.
  float averageRating; // Average rating of this movie.
  int numVotes;        // Number of votes of this movie.
};

#endif