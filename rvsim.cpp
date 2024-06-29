/*
	This is just a skeleton. It DOES NOT implement all the requirements.
	It only recognizes the RV32I "ADD", "SUB" and "ADDI" instructions only. 
	It prints "Unkown Instruction" for all other instructions!

	References:
	(1) The risc-v ISA Manual ver. 2.1 @ https://riscv.org/specifications/
	(2) https://github.com/michaeljclark/riscv-meta/blob/master/meta/opcodes
*/

#include <iostream>
#include <fstream>
#include "stdlib.h"
#include <iomanip>
#include <string>

using namespace std;

struct registers
{
	string name;
	int value;
};

unsigned int pc;
unsigned char memory[(64+64)*1024];
registers reg[32]={{"zero",0},{"ra",0},{"sp",4194304},{"gp",0},{"tp",0},{"t0",0},{"t1",0},{"t2",0},{"s0",0},{"s1",0},{"a0",0},{"a1",0},{"a2",0},{"a3",0},{"a4",0},{"a5",0},{"a6",0},{"a7",0},{"s2",0},{"s3",0},{"s4",0},{"s5",0},{"s6",0},{"s7",0},{"s8",0},{"s9",0},{"s10",0},{"s11",0},{"t3",0},{"t4",0},{"t5",0},{"t6",0}};


struct registers
{
	string name;
	int value;
};

registers reg[32] = { {"zero",0},{"ra",0},{"sp",4194304},{"gp",0},{"tp",0},
					{"t0",0},{"t1",0},{"t2",0},{"s0",0},{"s1",0},
					{"a0",0},{"a1",0},{"a2",0},{"a3",0},{"a4",0},{"a5",0},{"a6",0},{"a7",0},
					{"s2",0},{"s3",0},{"s4",0},{"s5",0},{"s6",0},{"s7",0},{"s8",0},{"s9",0},
					{"s10",0},{"s11",0},{"t3",0},{"t4",0},{"t5",0},{"t6",0} };



void emitError(char *s)
{ 
	cout << s;
	exit(0);
}

void printPrefix(unsigned int instA, unsigned int instW){
	cout << "0x" << hex << setfill('0') << setw(8) << instA << "\t0x" << setw(8) << instW;
}

// this is a function to load i+1 bytes to a reg, i is offset to reach most significant byte (little endian)
int mem_to_reg(int ad, int i) { 
	int x = 0; // will hold our int

	while (i > 0) {
		x |= memory[ad + i]; // captures the i-th byte by 'or'ing
		x <<= 8; // shift to allow place for next byte
		i--;
	}
	x |= memory[ad]; // add last byte without shifting

	return x;
}


void instDecExec(unsigned int instWord)
{
	unsigned int rd, rs1, rs2, funct3, funct7, opcode;
	unsigned int I_imm, S_imm, B_imm, U_imm, J_imm, J_temp;
	unsigned int address;

	unsigned int instPC = pc - 4;
	B_imm=0x0;
	J_imm=0x0;
	// — inst[31] — inst[30:25] inst[24:21] inst[20]
	

	printPrefix(instPC, instWord);

	opcode = instWord & 0x0000007F;
	rd = (instWord >> 7) & 0x0000001F;
	funct3 = (instWord >> 12) & 0x00000007;
	rs1 = (instWord >> 15) & 0x0000001F;
	rs2 = (instWord >> 20) & 0x0000001F;

	I_imm = ((instWord >> 20) & 0x7FF) | (((instWord >> 31) ? 0xFFFFF800 : 0x0));


	B_imm = ((instWord >> 7) & 0x1); 
  B_imm = (B_imm << 10);
	B_imm = (B_imm | ((instWord >> 8) & 0x00F) |  ((instWord >> 21) & 0x3F0) | ((instWord >> 20) & 0x800));
	B_imm = B_imm | ((instWord >> 31) ? 0xFFFFF800 : 0x0);
	B_imm = B_imm << 1;
	
  	// - inst[31] - inst[30:25] - inst[11:7]
	//temp = (((instWord >> 7) & 0x0000001F) | ((instWord >> 25 & 0x0000003F) << 5));
	//S_imm = temp | (((instWord >> 31) ? 0xFFFFF800 : 0x0));

	S_imm = (((instWord >> 7) & 0x0000001F) |
			((instWord >> 20 & 0x00000FE0))) |
			((instWord >> 31) ? 0xFFFFF800 : 0x0);

	// extracts the immediate used for the jump and link instruction
	J_temp = (instWord >> 20) & 0x000003FF; 
	J_temp = J_temp << 1;
	J_imm = J_imm | J_temp ;
	J_temp = (instWord >> 19) & 0x00000001; 
	J_temp = J_temp << 11;
	J_imm = J_imm | J_temp;
	J_temp = (instWord >> 12) & 0x000000FF;
	J_temp = J_temp << 12;
	J_imm = J_imm | J_temp;
	J_temp = (instWord >> 31) & 0x00000001;
	J_temp = J_temp << 20;
	J_imm = J_imm | J_temp;

	J_imm |= (((instWord >> 31) ? 0xFFF00000 : 0x0));

	// R Instructions
	if(opcode == 0x33){	
		funct7 = (instWord >> 25) & 0x0000007F;	
		switch(funct3){
			case 0: if(funct7 == 32)
					{
					   cout << "\tSUB\t" << reg[rd].name <<","<<reg[rs1].name <<","<< reg[rs2].name << "\n";
					   reg[rd].value = reg[rs1].value - reg[rs2].value;
					}
					else 
					{
						cout << "\tADD\t" << reg[rd].name <<","<<reg[rs1].name <<","<< reg[rs2].name << "\n";
						reg[rd].value = reg[rs1].value + reg[rs2].value;
					}
					break;

			case 4: {
						cout << "\tXOR\t" << reg[rd].name <<","<<reg[rs1].name <<","<< reg[rs2].name << "\n";
						reg[rd].value = reg[rs1].value ^ reg[rs2].value;
			        } 
			         break;
			case 6:  {
						cout << "\tOR\t" << reg[rd].name <<","<<reg[rs1].name <<","<< reg[rs2].name << "\n";
						reg[rd].value = reg[rs1].value | reg[rs2].value;
			         }	
			         break;
			case 7:  {
						cout << "\tAND\t" << reg[rd].name <<","<<reg[rs1].name <<","<< reg[rs2].name << "\n";
						reg[rd].value = reg[rs1].value & reg[rs2].value;
					 }
			         break;
			case 1: {
						 cout << "\tSLL\t" << reg[rd].name <<","<<reg[rs1].name <<","<< reg[rs2].name << "\n";
						 reg[rd].value = reg[rs1].value << reg[rs2].value;
					}
                     break;
			case 5: if(funct7 == 32)
					{
						cout << "\tSRA\t" << reg[rd].name <<","<<reg[rs1].name <<","<< reg[rs2].name << "\n";
						reg[rd].value = reg[rs1].value >> reg[rs2].value;
					} 
                    else
					{
						cout << "\tSRL\t" << reg[rd].name <<","<<reg[rs1].name <<","<< reg[rs2].name << "\n";
						 reg[rd].value = (unsigned int)reg[rs1].value >> reg[rs2].value;
					}
					 break;

			case 2: {
						 cout << "\tSLT\t" << reg[rd].name <<","<<reg[rs1].name <<","<< reg[rs2].name << "\n";
						 reg[rd].value = (reg[rs1].value < reg[rs2].value) ? 1 : 0;
					}	 
				     break;
			case 3:  {
						 cout << "\tSLTU\t" << reg[rd].name <<","<<reg[rs1].name <<","<< reg[rs2].name << "\n";
						 reg[rd].value = ((unsigned int)reg[rs1].value < (unsigned int)reg[rs2].value) ? 1 : 0;	 
					 }
			         break;
			       				
			default: cout << "\tUnkown R Instruction \n";
		}

	} 
	else if(opcode == 0x13){	// I instructions
		switch(funct3){
			case 0:	cout << "\tADDI\tx" << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
					break;

			default:
							cout << "\tUnkown R Instruction \n";
		}

	} 
	else if (opcode == 0x13) {	// I instructions
		switch (funct3) {
		case 0:	cout << "\tADDI\tx" << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
			break;
		default:
			cout << "\tUnkown I Instruction \n";
		}
	}
	else if (opcode == 0x03) { // I - Load Instructions
		int ad = reg[rs1].value + (int)I_imm; //stores the address of the LS byte we need
		switch (funct3) {
		case 0: cout << "\tLB\t" <<  reg[rd].name << ", " << dec << (int)I_imm << "(" << reg[rs1].name << ")\n";
			temp = memory[ad];
			reg[rd].value = memory[ad] | ((temp >> 7) ? 0xFFFFFF00 : 0x0); // sign extending
			break;
		case 1:	cout << "\tLH\tx" << reg[rd].name << ", " << dec << (int)I_imm << "(" << reg[rs1].name << ")\n";
			temp = mem_to_reg(ad, 1);
			reg[rd].value = temp | ((temp >> 15)? (0xFFFF0000): 0x0); // sign extending
			break;
		case 2:	cout << "\tLW\tx" << reg[rd].name << ", " << dec << (int)I_imm << "(" << reg[rs1].name << ")\n";
			reg[rd].value = mem_to_reg(ad, 3);
			break;
		case 4:	cout << "\tLBU\tx" << reg[rd].name << ", " << dec << (int)I_imm << "(" << reg[rs1].name << ")\n";
			reg[rd].value = memory[ad]; // automatically zero extended because memory is unsigned char
			break;
		case 5:	cout << "\tLHU\tx" << reg[rd].name << ", " << dec << (int)I_imm << "(" << reg[rs1].name << ")\n";
			reg[rd].value = mem_to_reg(ad, 1);
			break;
		default: 
			cout<< "\tUnkown I-Load Instruction \n";
		}
	}
	else if (opcode == 0x23) {	// S Instructions
		int ad = reg[rs1].value + (int)S_imm; //stores the address of the LS byte we need
		switch (funct3) {
		case 0:	cout << "\tSB\tx" << reg[rs2].name << ", " << dec << (int)S_imm << "(" << reg[rs1].name << ")\n";
			memory[ad] = reg[rs2].value; // the memory address gets the LS byte in reg[rs2]
			break;
		case 1:	cout << "\tSH\tx" << reg[rs2].name << ", " << dec << (int)S_imm << "(" << reg[rs1].name << ")\n";
			memory[ad] = reg[rs2].value;
			memory[ad + 1] = reg[rs2].value >> 8;
			break;
		case 2: cout << "\tSW\tx" << reg[rs2].name << ", " << dec << (int)S_imm << "(" << reg[rs1].name << ")\n";
			memory[ad] = reg[rs2].value;
			memory[ad + 1] = reg[rs2].value >> 8;
			memory[ad + 2] = reg[rs2].value >> 16;
			memory[ad + 3] = reg[rs2].value >> 24;
			break;
		default:
			cout << "\tUnkown S Instruction \n";
		}

	} 
	else if (opcode == 0x63)
	{
		switch(funct3){
			case 0: 
         {
						cout << "\tBEQ\t" <<reg[rs1].name <<","<< reg[rs2].name << ","<< hex << "0x" << instPC+(int)B_imm << "\n";
						pc = (reg[rs1].value == reg[rs2].value) ? instPC+(int)B_imm : pc; 
				 }
					break;
			case 1: 
        {
						cout << "\tBNE\t" <<reg[rs1].name <<","<< reg[rs2].name <<","<< hex << "0x" << instPC+(int)B_imm <<"\n";
						pc = (reg[rs1].value != reg[rs2].value) ? instPC+(int)B_imm : pc; 
				}
					break;
       case 6: 
        {
          cout << "\tBLTU\tx" << reg[rs1].name << ", " << reg[rs2].name <<", 0x" << hex << (instPC + B_imm)<< "\n";
          if ((unsigned int)reg[rs1].value < (unsigned int)reg[rs2].value) pc = instPC + (int)B_imm;
        }
			break;
		  case 7: 
        {
          cout << "\tBGEU\tx" << reg[rs1].name << ", " << reg[rs2].name <<", 0x" << hex << (instPC + B_imm) << "\n";
			    if ((unsigned int)reg[rs1].value >= (unsigned int)reg[rs2].value) pc = instPC + (int)B_imm;
        }
			break;
			default:
						cout << "\tUnkown B Instruction \n";	
		}

	}
	else if(opcode == 0x6F)
	{

		cout << "\tJAL\t" <<reg[rd].name <<","<< hex << "0x" << instPC+(int)J_imm <<"\n";
		reg[rd].value = pc;
		pc = instPC + J_imm;
	}
	else if(opcode == 0x67)
	{
		cout << "\tJALR\t" <<reg[rd].name <<","<< reg[rs1].name <<","<< hex << "0x" << reg[rs1].value + (int)I_imm <<"\n";
		reg[rd].value = pc;
		pc = reg[rs1].value + (int)I_imm;
	}
	else if (opcode == 73) 
  {
		if (reg[17].value == 1)  // a7 = 1
			 cout << reg[10].value;	
		else if (reg[17].value == 4)
    {
			int ad = reg[10].value;
			char c = memory[ad]; // c = char at the address stored in a0
			string output = "";

			while (c != 0) 
      { // as long as we havent reached a null
				output += c;
				ad++;
				c = memory[ad];
		  }
			cout << output;
		}
		else 
    {
			cout << "\tUnsupported Ecall Functionality\n";
		}
	}
	else 
  {
      cout << "\tUnkown Instruction \n";
	} 
}


int main(int argc, char *argv[]){

	unsigned int instWord=0;
	ifstream inFile;
	ofstream outFile;
	// for sp the initail value is (64*64*1024) 
	
	if(argc<1) emitError("use: rvcdiss <machine_code_file_name>\n");

	inFile.open(argv[1], ios::in | ios::binary | ios::ate);

	if(inFile.is_open())
	{
		int fsize = inFile.tellg();

		inFile.seekg (0, inFile.beg);
		if(!inFile.read((char *)memory, fsize)) emitError("Cannot read from input file\n");

		while(true){
				instWord = 	(unsigned char)memory[pc] |
							(((unsigned char)memory[pc+1])<<8) |
							(((unsigned char)memory[pc+2])<<16) |
							(((unsigned char)memory[pc+3])<<24);
				pc += 4;
				// remove the following line once you have a complete simulator
				if(pc==32) break;			// stop when PC reached address 32
				instDecExec(instWord);
		}
	} else emitError("Cannot access input file\n");
}
