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

CacheRequest CacheController::write(CacheRequest& address, unsigned char data)
{
             
}
CacheRequest CacheController::read(CacheRequest& address)
{
             unsigned char a = address.request;
             cout<<"address = "<<hex<<(int)a<<endl;
             if(CacheIndex[address.groupid][address.blockid][0] == (a&0xFC))
             {
                 //hit
                 LastUsed[address.groupid][0] = 1;
                 LastUsed[address.groupid][1] = 0;
                 address.stallfor = 0;
             }
             else if(CacheIndex[address.groupid][address.blockid][1] == (a&0xFC))
             {
                  //hit
                  LastUsed[address.groupid][1] = 1;
                  LastUsed[address.groupid][0] = 0;
                  address.stallfor = 0;
             }
             else
             {
                 //miss
                 int replaceselect = getNotLastUsed(address.groupid,address.blockid);
                 putBlockInCache(address.groupid, address.blockid, replaceselect , a);
                 if(replaceselect == 0)
                 {
                     LastUsed[address.groupid][0] = 1;
                     LastUsed[address.groupid][1] = 0;
                 }
                 else
                 {
                     LastUsed[address.groupid][1] = 1;
                     LastUsed[address.groupid][0] = 0;
                 }
                 address.stallfor = 4;
                 
             }    
             
             address.byteread = Cache[address.groupid][address.blockid][address.byteid];         
             
             return address;
}

int CacheController::getNotLastUsed(int gid, int bid)
{
     if(CacheIndex[gid][bid][0] == 1)
     {
         return 0;
     }
     else
     {
         return 1;
     }
}

void CacheController::putBlockInCache(int gid, int blid, int blsel, unsigned char request)
{
     CacheIndex[gid][blid][blsel] = (request&0xFC);
     Cache[gid][blid][0] = DM[(request&0xFC) | 0x00];
     Cache[gid][blid][1] = DM[(request&0xFC) | 0x01];
     Cache[gid][blid][2] = DM[(request&0xFC) | 0x02];
     Cache[gid][blid][3] = DM[(request&0xFC) | 0x03];
}


void CacheController::PrintDataMemory()
{
    for(int i = 0; i<size; i++)
    {
            printf("DM %02X = %02X\n",i,DM[i]);
    }
}

