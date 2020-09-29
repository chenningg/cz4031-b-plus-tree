#include "db.h"

#include <iostream>
#include <malloc.h>

DB::DB()
{
  store = ::operator new(sizeof(Record) * 1000000);
  std::cout << _msize(store) << std::endl;
}

DB::~DB()
{
  ::operator delete(store);
}