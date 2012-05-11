#include "CommLink.h"

CommLink::CommLink(string ma, string sl,Core * o, Core * d,unsigned char Tstat, unsigned char Tdata, unsigned char Rstat, unsigned char Rdata, unsigned char Tdone, unsigned char Rdone)
{
     master = o;
     slave = d;
     TXstatusPort = Tstat;
     TXdataPort = Tdata;
     RXstatusPort = Rstat;
     RXdataPort = Rdata;
     TXdonePort = Tdone;
     RXdonePort = Rdone;
     m = ma;
     s = sl;
}

void CommLink::communicate()
{

     if(master->wroteToIO)
     {
          printf("%s wrote to an IO.. analyzing transmission: \n",m.c_str());
          masterToSlave();
          master->wroteToIO = false;
     }
     if(slave->wroteToIO)
     {
          slaveToMaster();
     }
     if(slave->readFromIO)
     {
          printf("%s read from IO.. analyzing transmission: \n",s.c_str());
          printf("%s Read %02X",s.c_str(),TXFIFO.back());
          TXFIFO.pop_back();
          slave->IO[0x00] = TXFIFO.back();
          slave->readFromIO = false;
     }
}

void CommLink::masterToSlave()
{
     //Transmitting from master -> slave
      if(master->IOADDR == TXstatusPort)
      {
           //master just set the IO status type
           TXSTATUS = master->IO[master->IOADDR];
           printf("%s setting status for transmission: %02X\n",m.c_str(),TXSTATUS);
      }
      if(master->IOADDR == TXdataPort)
      {
           //master just wrote some data into IO port
           TXFIFO.push_front(master->IO[master->IOADDR]);
           slave->IO[0x00] = TXFIFO.back();
           printf("%s trasmitting data: %02X on IO port %02X\n",m.c_str(),TXFIFO.front(),master->IOADDR);
      }
      if(master->IOADDR == TXdonePort)
      {
           if(master->IO[master->IOADDR] == 0x01)
           {
                //master finished writing to the buffer..
                //signal slave processor to start reading from the buffer
                interruptProc(slave, (unsigned char)0xFF, slave->RXISRADDR);
                cout<<"Fired interrupt on "<<s<<" core"<<endl;
           }
           else
           {
               //master isn't done writing to the buffer yet.. dont see why this code would ever hit
               cout<<m<<" isn't done writing to the buffer yet.. THIS SHOULD NEVER HIT"<<endl;
               //reason it should never hit is that the done port isn't written until the transmission end
           }
           
      }
      
}

void CommLink::slaveToMaster()
{
     //Transmitting from slave -> master
}

void CommLink::interruptProc(Core * C, unsigned char PMADDR, unsigned char ISRADDR)
{
     C->PM[PMADDR] = ISRADDR;
}
