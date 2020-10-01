#include "memory_pool.h"

#include <iostream>

int main()
{
  MemoryPool db{500000000, 100};

  std::cout << "Pool size: " << db.getPoolSize() << " bytes" << '\n';
  std::cout << "Block size: " << db.getBlockSize() << " bytes" << '\n';

  Movie test{false, 5.6, 12312, "tt2351125"};
  std::cout << "Size of a record: " << sizeof(test) << " bytes" << '\n';

  std::cout << "Current blocks available: " << db.getAvailable() << " blocks" << '\n';
  std::cout << "Inserting 9 instances of test (expect 3 blocks to be used):" << '\n';
  std::cout << "Inserted record 1 at: " << reinterpret_cast<void *>(db.allocate(test)) << '\n';
  std::cout << "Inserted record 2 at: " << reinterpret_cast<void *>(db.allocate(test)) << '\n';
  std::cout << "Inserted record 3 at: " << reinterpret_cast<void *>(db.allocate(test)) << '\n';
  std::cout << "Inserted record 4 at: " << reinterpret_cast<void *>(db.allocate(test)) << '\n';
  std::cout << "Inserted record 5 at: " << reinterpret_cast<void *>(db.allocate(test)) << '\n';
  std::cout << "Inserted record 6 at: " << reinterpret_cast<void *>(db.allocate(test)) << '\n';
  std::cout << "Inserted record 7 at: " << reinterpret_cast<void *>(db.allocate(test)) << '\n';
  std::cout << "Inserted record 8 at: " << reinterpret_cast<void *>(db.allocate(test)) << '\n';
  std::cout << "Inserted record 9 at: " << reinterpret_cast<void *>(db.allocate(test)) << '\n';
  std::cout << "Current blocks available: " << db.getAvailable() << " blocks" << '\n';
  std::cout << "Size used: " << db.getSizeUsed() << " bytes" << '\n';
  return 0;
}