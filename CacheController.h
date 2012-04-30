/*
  Holden Sandlar
  Advanced Computer Architecture
  0301-810 w/ Dorin Patru
  Cache Controller
*/

#ifndef CACHE_CONTROLLER_H
#define CACHE_CONTROLLER_H
#define CACHE_SIZE 32
#include <fstream>
#include "CacheRequest.h"
using namespace std;
class CacheController
{
      friend class CacheRequest;
      public:
             CacheController(unsigned char*,int);
             CacheRequest write(CacheRequest&, unsigned char);
             CacheRequest read(CacheRequest&);
             void putBlockInCache(int, int, int, unsigned char);
             int getNotLastUsed(int, int);
             void PrintDataMemory();
             
      private:
             unsigned char Cache[4][2][4];
             unsigned char CacheIndex[4][2][2];
             unsigned int LastUsed[4][2];
             unsigned char * DM;
             int size;
};
#endif
