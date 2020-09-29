#include <iostream>

#include "bplustree.h"
#include "db.h"

int main()
{
  std::cout << "The size of a record is " << sizeof(DB::Record) << '\n';
  DB();
  return 0;
}