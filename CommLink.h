#ifndef COMMLINK_H
#define COMMLINK_H
#include <iostream>
#include <deque>
#include "Core.h"
using namespace std;


class CommLink
{
      public:
             deque <unsigned char> TXFIFO;
             int TXSTATUS;
             int TXDONE;
             CommLink(Core&,Core&,unsigned char,unsigned char,unsigned char, unsigned char, unsigned char, unsigned char);
             void communicate();
             
      private:
              Core *master,*slave;
              void TXComms();
              void RXComms();
              void interruptProc(Core * C, unsigned char, unsigned char);
              unsigned char TXstatusPort,TXdataPort,RXstatusPort,RXdataPort, TXdonePort, RXdonePort;
};

#endif
