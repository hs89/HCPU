/*
  Holden Sandlar
  Advanced Computer Architecture
  0301-810 w/ Dorin Patru
  Register File for HCPU
*/
#include "Registers.h"

Registers::Registers(string register_file)
{
     RF = register_file;
     R0 = 0x00;
     R1 = 0x00;
     R2 = 0x00;
     R3 = 0x00;
     R0_in_use = R1_in_use = R2_in_use = R3_in_use = 0;
}
void Registers::writeToDisk()
{
     ofstream outfile(RF.c_str(), ios::trunc);
     if(outfile.is_open())
     {
         outfile<<R0<<endl;
         outfile<<R1<<endl;
         outfile<<R2<<endl;
         outfile<<R3;
         outfile.close();
     }
     else
     {
         cout<<"Unable to open register file"<<endl;
     }
     
}
void Registers::readFromDisk()
{
     string line;
     ifstream infile(RF.c_str(), ios::in);
     int i = 0;
     if(infile.is_open())
     {
          while(infile.good())
          {
               getline(infile,line);
               setRegister(i, line.at(0));
               i++;
          }
          infile.close();
     }
     else
     {
         cout<<"Unable to open register file"<<endl;
     }
}
void Registers::setRegister(int rnum, char rval)
{
     switch(rnum)
     {
          case 0:
               R0 = rval;
               break;
          case 1:
               R1 = rval;
               break;
          case 2:
               R2 = rval;
               break;
          case 3:
               R3 = rval;
               break;
          default:
                  cout<<"Invalid register assignment number: "<<hex<<rnum<<endl;
                  break;
     }
}
void Registers::print()
{
     printf("R0 = %02X\n",R0);
     printf("R1 = %02X\n",R1);
     printf("R2 = %02X\n",R2);
     printf("R3 = %02X\n",R3);
}

void Registers::resetRegisters()
{
     setRegister(0,0x00);
     setRegister(1,0x00);
     setRegister(2,0x00);
     setRegister(3,0x00);  
     R0_in_use = R1_in_use = R2_in_use = R3_in_use = 0;   
}

char Registers::getRegister(int rnum)
{
     switch(rnum)
     {
          case 0:
               return R0;
               break;
          case 1:
               return R1;
               break;
          case 2:
               return R2;
               break;
          case 3:
               return R3;
               break;
          default:
                  cout<<"Trying to get a value from an invalid register number"<<endl;
                  return 0x00;
                  break;
     }
}
