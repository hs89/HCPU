/*
  Holden Sandlar
  Advanced Computer Architecture
  0301-810 w/ Dorin Patru
  Cache Controller
*/
#include "CacheController.h"

CacheController::CacheController(unsigned char * memory, int s)
{
     DM = memory;
     size = s;
     
}

CacheRequest CacheRequest::write(CacheRequest& address, unsigned char data)
{
             
}
CacheRequest CacheRequest::read(CacheRequest& address)
{
             if(Cache[address.groupid][0] == DM[address.request])
             {
                  //cache hit
             }
             else if(Cache[address.groupid][1] == DM[address.request])
             {
                  //cache hit
             }
             else
             {
                 //cache miss
             }
}


void CacheController::PrintDataMemory()
{
    for(int i = 0; i<size; i++)
    {
            printf("DM %02X = %02X\n",i,DM[i]);
    }
}

