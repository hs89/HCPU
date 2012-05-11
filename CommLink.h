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
             string m,s;
             CommLink(string,string,Core*,Core*,unsigned char,unsigned char,unsigned char);
             void communicate();
             void printFIFO();
             void print();
             
      private:
              Core *master,*slave;
              void masterToSlave();
              void slaveToMaster();
              void interruptProc(Core * C, unsigned char, unsigned char);
              unsigned char TXstatusPort,TXdataPort,RXstatusPort,RXdataPort, TXdonePort, RXdonePort;
};

#endif
