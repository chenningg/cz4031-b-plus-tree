#include <iostream>

#include "bplustree.h"
#include "db.h"

int main()
{
  std::cout << "The size of Movie is " << sizeof(DB::Record) << '\n';
  return 0;
}