#include "CommLink.h"

CommLink::CommLink(Core& o, Core& d,unsigned char Tstat, unsigned char Tdata, unsigned char Rstat, unsigned char Rdata, unsigned char Tdone, unsigned char Rdone)
{
     *master = o;
     *slave = d;
     TXstatusPort = Tstat;
     TXdataPort = Tdata;
     RXstatusPort = Rstat;
     RXdataPort = Rdata;
     TXdonePort = Tdone;
     RXdonePort = Rdone;
}

void CommLink::communicate()
{
     if(master->wroteToIO)
     {
          TXComms();
          master->wroteToIO = false;
     }
     if(slave->wroteToIO)
     {
          RXComms();
     }
}

void CommLink::TXComms()
{
     //Transmitting from master -> slave
      if(master->IOADDR == TXstatusPort)
      {
           //master just set the IO status type
           TXSTATUS = master->IO[master->IOADDR];
      }
      if(master->IOADDR == TXdataPort)
      {
           //master just wrote some data into IO port
           TXFIFO.push_front(master->IO[master->IOADDR]);
      }
      if(master->IOADDR == TXdonePort)
      {
           if(master->IO[master->IOADDR] == 0x01)
           {
                //master finished writing to the buffer..
                //signal slave processor to start reading from the buffer
                interruptProc(slave, (unsigned char)0xFF, slave->RXISRADDR);
           }
           else
           {
               //master isn't done writing to the buffer yet.. dont see why this code would ever hit
               cout<<"Master isn't done writing to the buffer yet.. THIS SHOULD NEVER HIT"<<endl;
           }
      }
}

void CommLink::RXComms()
{
     //Transmitting from slave -> master
}

void CommLink::interruptProc(Core * C, unsigned char PMADDR, unsigned char ISRADDR)
{
     C->PM[PMADDR] = ISRADDR;
}
