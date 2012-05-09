/*
  Holden Sandlar
  Advanced Computer Architecture
  0301-810 w/ Dorin Patru
  Assembler for HCPU
*/

#include "Assembler.h"

Assembler::Assembler(string opcode_file)
{
     //Need to load in the instruction mapping
     ASSEMBLER_DEBUG = true;
     ifstream input(opcode_file.c_str());
     string text;
     string hex;
     string blank;
     written_count = linenumber = 0;
     for(int i = 0;i<256;i++)
     {
             programMemory[i] = 0x00;
     }
     if(input.is_open()) 
     {
         while(input.good())
         { 
              getline(input,text,'\n');
              getline(input,hex,'\n');
              instructionTable[text] = strtol(hex.c_str(), NULL, 16);
         }
         input.close();
     }
     else
     {
         printError("Unable to open opcode file",0);
     }
     reset();
}

Assembler::~Assembler(){}

void Assembler::Assemble(string input_file, string output_file)
{
     //printInstructionTable();
     if(ASSEMBLER_DEBUG) {cout<<"Assembling "<<input_file<<" into "<<output_file<<endl;}
     ifstream input(input_file.c_str());
     clearOutputFile(output_file);
     string line;
     char comment_char;
     if(input.is_open())
     {
         while(input.good())
         {
              getline(input, line, ';');
              linenumber++;
              for(;;)
              {
                  //Ignore the comment
                  input.get(comment_char);
                  if(comment_char != '\n')
                  {
                      //Ignore the comment
                      continue;
                  }
                  else
                  {
                      //end of the line (including comments)
                      break;
                  }
              }
              line = rtrim(line); //trim all trailing whitespace
              line = ltrim(line); //trim all leading  whitespace
              if(ASSEMBLER_DEBUG) cout<<line<<endl;
              if(line == "END")
              {
                      endofprog = written_count;
                      written_count++;
                      reset();
                      continue;
              }
              parseInstruction(line);
              if(is_label != 1)
              {
                    createInstructionWords();
                    outputInstruction();
              }
              else
              {
                  //need to save this label and the corresponding starting address of its instruction sequence
                  if(line == ":RXISR")
                  {
                      RISRA = written_count;
                  }
                  if(line == ":TXISR")
                  {
                      TISRA = written_count;
                  }
                  if(ASSEMBLER_DEBUG) cout<<"Starting address of label labelStart["<<line<<"] = "<<dec<<written_count<<endl;
                  labelStart[line] = written_count;
              }
              reset();
         }
         input.close();
     }
     else
     {
         printError("Unable to open code files",0);
     }
     fillInLabelAddresses();
     writeOutProgramMemory(output_file);
}

char Assembler::convertInstruction(string instruction)
{
     char temp = (char)instructionTable[instruction];
     //cout<<"Lookup of "<<instruction<<" = "<<hex<<(int)temp<<endl;
     return temp;
}

void Assembler::parseInstruction(string instruction_line)
{
     int space_at;
     int comma_at;
     if(instruction_line.at(0) == ':')
     {
          is_label = 1;
          return;
     }                         
     space_at = instruction_line.find(" ");
     opcode = instruction_line.substr(0,space_at);
     comma_at = instruction_line.find(",",space_at);
     if(comma_at == -1)
     {
          //no comma -- 0 or 1 operand instruction
          if(space_at == -1)
          {
              //zero operand instruction
              num_of_operands = 0;
          }
          else
          {
              //one operand instruction
              //be careful here.. we need to get instruction_line.length() characters  in the case 
              //there is a label being held in operand1
              operand1 = instruction_line.substr(space_at+1, instruction_line.length());
              num_of_operands = 1;
          }
     }
     else
     {
         //multiple operand instruction..
         //split it into parts
         operand1 = instruction_line.substr(space_at+1,2);
         operand2 = instruction_line.substr(comma_at+1,2);
         if(comma_at+3 != instruction_line.length())
         {
             //3 operand instruction.. must be a displacement addressing mode instruction or IO
             operand3 = instruction_line.substr(comma_at+4,2);
             num_of_operands = 3;
         }
         else
         {
             num_of_operands = 2;
         }
     }
     //print();
}
void Assembler::outputInstruction()
{
         programMemory[written_count] = (iw1 & 0xFF);
         written_count++;
         if(iw2 != 0x3B)
         {
                if(iw2 == ':')
                {
                    labelOperand[written_count] = operand1;
                    written_count++;
                }
                else
                {
                    programMemory[written_count] = (iw2 & 0xFF);
                    written_count++;
                }
         }
}
void Assembler::createInstructionWords()
{
     char tempchar = convertInstruction(opcode);
     opcode.empty();
     opcode[0] = tempchar;
     //cout<<"Op = "<<hex<<(opcode[0] & 0xFF)<<endl;
     switch(opcode[0] & 0xF0)
     {
          case 0x00: //CPY
          case 0x10: //SWAP
          case 0x80: //ADD
          case 0x90: //SUB
          case 0xA0: //AND
          case 0xB0: //CEQ
          case 0xC0: //CLT
          case 0xD0: //MUL
          case 0xE0: //DIV
               //Two operand instructions
               {
                     iw1 = (opcode[0]&0xF0) | (registerNumber(operand1)<<2);
                     iw1 = iw1 | (registerNumber(operand2));
               }
               break;
          case 0x20: //LD/ST
               iw1 = (opcode[0] & 0xFF) | (registerNumber(operand1)<<2);
               if((opcode[0] & 0xFF) == 0x22)
               {
                    //LD Displacement
                    //cout<<"operand3= "<<operand3<<endl;
                    iw2 = (registerNumber(operand2)<<6) | (strtol(operand3.c_str(),NULL,16) & 0x3F);
                    
               }
               if((opcode[0] & 0xFF) == 0x20)
               {
                    //LD Immediate
                    iw2 = (strtol(operand2.c_str(), NULL, 16) & 0xFF);
                    //cout<<"LD IMMEDIATE: iw2 = "<<hex<<(int)iw2<<endl;
               }
               if((opcode[0] & 0xFF) == 0x23)
               {
                    //ST Displacement
                    iw2 = (registerNumber(operand2)<<6) | (strtol(operand3.c_str(),NULL,16) & 0x3F);
               }
               if((opcode[0] & 0xFF) == 0x21)
               {
                    //ST Immediate -- this translates to writing to program memory
                    iw2 = (strtol(operand2.c_str(),NULL,16)&0xFF);
               }
               break;
          case 0x30: //IN/OUT
                 iw1 = (opcode[0] & 0xFF) | (registerNumber(operand1)<<2);
                 iw2 = strtol(operand2.c_str(),NULL,16);
                 //cout<<"IW2 = "<<hex<<(int)iw2<<endl;
               break;
          case 0x40: //SHIFT
          case 0xF0: //NOT
               {
                     iw1 = (opcode[0] & 0xFF) | (registerNumber(operand1)<<2);
               }
               break;
          case 0x50: //JMP/BR/CALL
                     if(operand1.size() != 0)
                     {
                         if(operand1.at(0) == ':' && operand1.size() != 0)
                         {
                             iw1 = (opcode[0] & 0xFF);
                             iw2 = ':'; //this corresponds to 0x3A which is a code that will never occur
                         }
                         else
                         {
                             //must be a direct address, not a label
                             iw2 = strtol(operand1.c_str(),NULL,16);
                         }
                     }
                     else
                     {
                         printError("ILLEGAL JMP/BR/CALL INSTRUCTION",linenumber);
                     }
               break;
          case 0x60: //RET/RETI
                     iw1 = (opcode[0] & 0xFF);
               break;
          case 0x70: //SIMD
               break;
          
     }
}
void Assembler::clearOutputFile(string output_file_name)
{
     ofstream output(output_file_name.c_str(), ios::trunc);
     output.close();
}
//Credit for the trim functions: http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
string & Assembler::rtrim(string &s)
{
      s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
      return s;
}
// trim from start
string & Assembler::ltrim(string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}
char Assembler::registerNumber(string regnum)
{
     if(regnum == "R0") { return 0x00; }
     if(regnum == "R1") { return 0x01; }
     if(regnum == "R2") { return 0x02; }
     if(regnum == "R3") { return 0x03; }
     else
     {
         printError("Trying to convert an invalid register number in the assembler",linenumber);
     }
}
void Assembler::fillInLabelAddresses()
{
     map<int,string>::iterator it;
     for(it = labelOperand.begin(); it != labelOperand.end(); it++)
     {
            //we need to go to line number (*it).first and insert the starting address for label (*it).second
            //starting address can be obtained from labelStart[(*it).second]
            if(ASSEMBLER_DEBUG) cout<<"Filling labels.. programMemory["<<(*it).first<<"] = labelStart["<<(*it).second<<"] = "<<(labelStart[(*it).second])<<endl;
            programMemory[(*it).first] = (labelStart[(*it).second]);
     }
}
void Assembler::writeOutProgramMemory(string output_file)
{
     FILE* output = fopen(output_file.c_str(),"w");
     for(int i = 0; i<255; i++)
     {
             if(i == endofprog) 
             {
                  fprintf(output,"END\n");
                  continue;
             }
             fprintf(output, "%02X\n", (programMemory[i] & 0xFF));
     }
     if(endofprog == 255) fprintf(output,"END");
     else fprintf(output, "%02X", (programMemory[255] & 0xFF));
     fclose(output);
     
}
void Assembler::reset()
{
     int numofoperands = -1;
     is_label = 0;
     opcode.clear();
     operand1.clear();
     operand2.clear();
     operand3.clear();
     iw1 = 0x00;
     iw2 = 0x3B; // this is a code that can never occur
}
void Assembler::print()
{
     cout<<"Number of operands = "<<num_of_operands<<endl;
     cout<<"Opcode = "<<hex<<(opcode[0]&0xFF)<<endl;
     cout<<"Operand1 = "<<operand1<<endl;
     cout<<"Operand2 = "<<operand2<<endl;
     cout<<"Operand3 = "<<operand3<<endl;
}
void Assembler::printInstructionTable()
{    
     map<string,char>::iterator it;
     cout << "Instruction Table Contains:\n";
     for ( it=instructionTable.begin() ; it != instructionTable.end(); it++ )
      cout << (*it).first << " => " << hex<<(int)((*it).second) << endl;
}
void Assembler::printError(string error, int line)
{
     cout<<"ERROR: "<<error<<" at line "<<line<<endl;
     system("pause");
     exit(0);
}
