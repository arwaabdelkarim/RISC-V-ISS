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


void emitError(char *s)
{ 
	cout << s;
	exit(0);
}

void printPrefix(unsigned int instA, unsigned int instW){
	cout << "0x" << hex << setfill('0') << setw(8) << instA << "\t0x" << setw(8) << instW;
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
	

	// extracts the immdeiate used for the jump and link instruction
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
					cout << "\tUnkown I Instruction \n";
		}
	} 
	else if (opcode == 0x63)
	{
		switch(funct3){
			case 0: {
						cout << "\tBEQ\t" <<reg[rs1].name <<","<< reg[rs2].name << ","<< hex << "0x" << instPC+(int)B_imm << "\n";
						pc = (reg[rs1].value == reg[rs2].value) ? instPC+(int)B_imm : pc; 
					}
					break;
			case 1: {
						cout << "\tBNE\t" <<reg[rs1].name <<","<< reg[rs2].name <<","<< hex << "0x" << instPC+(int)B_imm <<"\n";
						pc = (reg[rs1].value != reg[rs2].value) ? instPC+(int)B_imm : pc; 
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
	else {
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
