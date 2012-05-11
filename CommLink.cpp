#include "CommLink.h"

CommLink::CommLink(string ma, string sl,Core * o, Core * d,unsigned char Tstat, unsigned char Tdata, unsigned char Tdone)
{
     master = o;
     slave = d;
     TXstatusPort = Tstat;
     TXdataPort = Tdata;
     TXdonePort = Tdone;
     m = ma;
     s = sl;
}

void CommLink::communicate()
{

     if(master->wroteToIO)
     {
          printf("%s wrote to IO[%02X].. analyzing transmission: \n",m.c_str(),master->IOADDR);
          masterToSlave();
     }
     if(slave->wroteToIO)
     {
          slaveToMaster();
     }
     if(slave->readFromIO)
     {
          printf("%s read from IO.. analyzing transmission: \n",s.c_str());
          printf("%s Read %02X\n",s.c_str(),TXFIFO.back());
          if(slave->IOADDR == 0x00)
          {
              TXFIFO.pop_back();
              slave->IO[0x00] = TXFIFO.back();
              slave->readFromIO = false;
          }
     }
}

void CommLink::masterToSlave()
{
     //Transmitting from master -> slave
      if(master->IOADDR == TXstatusPort)
      {
           //master just set the IO status type
           TXSTATUS = master->IO[master->IOADDR];
           slave->IO[0x01] = TXSTATUS;
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
           if(master->IO[TXdonePort] == 0x01)
           {
                //master finished writing to the buffer..
                //signal slave processor to start reading from the buffer
                interruptProc(slave, (unsigned char)0xFF, slave->RXISRADDR);
                cout<<"Fired interrupt on "<<s<<" core"<<endl;
                printFIFO();
           }
           else
           {
               //master isn't done writing to the buffer yet.. dont see why this code would ever hit
               cout<<m<<" isn't done writing to the buffer yet.. THIS SHOULD NEVER HIT"<<endl;
               //reason it should never hit is that the done port isn't written until the transmission end
           }
           
      }
      
}

void CommLink::print()
{
     cout<<"communication between "<<m<<" and "<<s;
     printf( " and TX done port = %02X\n",TXdonePort);  
}     

void CommLink::slaveToMaster()
{
     //Transmitting from slave -> master
}

void CommLink::interruptProc(Core * C, unsigned char PMADDR, unsigned char ISRADDR)
{
     C->PM[PMADDR] = ISRADDR;
}

void CommLink::printFIFO()
{
     cout<<"FIFO from "<<m<<" to "<<s<<" has data "<<endl;
     for(int i = 0;i<TXFIFO.size();i++)
     {
             printf("%02X ",TXFIFO[i]);
     }
     cout<<endl;
}
