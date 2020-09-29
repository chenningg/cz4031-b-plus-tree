#include <iostream>

#include "bplustree.h"
#include "db.h"

int main()
{
  Movie test{"t13r5121", 5.7, 231312};

  std::cout << "The sum of 3 and 4 is: " << add(3, 4) << '\n';
  std::cout << test.tconst << '\n';
  return 0;
}