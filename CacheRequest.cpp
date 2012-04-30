/*
  Holden Sandlar
  Advanced Computer Architecture
  0301-810 w/ Dorin Patru
  Cache Request
*/
#include "CacheRequest.h"
CacheRequest::CacheRequest(unsigned char r)
{
     request = r;
     groupid = (r & 0xC0)>>6;
     blockid = (r & 0x3C)>>2;
     byteid = (r&0x03);
     
}

void CacheRequest::print()
{
     cout<<dec<<"GID = "<<groupid<<"; BID = "<<blockid<<"; BYTEID = "<<byteid<<endl;
     printf("Byteread = %02X\n",byteread);
     printf("Stallfor = %d\n",stallfor);
     
}

