/*
  Holden Sandlar
  Advanced Computer Architecture
  0301-810 w/ Dorin Patru
  Cache Request
*/

#ifndef CACHE_REQUEST
#define CACHE_REQUEST
#include <iostream>
using namespace std;

class CacheRequest
{
      public:
             unsigned char request;
             int groupid, blockid, byteid;
             int stallfor;
             unsigned char byteread;
             CacheRequest();
             CacheRequest(unsigned char);
             void print();
      private:
              
};

#endif

