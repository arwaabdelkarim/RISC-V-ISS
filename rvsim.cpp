#include <iostream>
#include <fstream>
#include "stdlib.h"
#include <iomanip>
using namespace std;

string name[32] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"}; 
int reg[32];
unsigned int pc = 0;
unsigned char memory[(64+64)*1024]; 


void emitError(char *s)
{
	cout << s;
	exit(0);
}

void printPrefix(unsigned int instA, unsigned int instW)
{
	cout << "0x" << hex << std::setfill('0') << std::setw(8) << instA << "\t0x" << std::setw(8) << instW;
}

void instDecExec(unsigned int instWord)
{
	unsigned int rd, rs1, rs2, funct3, funct7, opcode;
	unsigned int I_imm, S_imm, B_imm, U_imm, J_imm;
	unsigned int address;
	unsigned int instPC = pc - 4;
	
	// initializing immediates
	I_imm = 0;
	B_imm = 0;
	U_imm = 0;

	opcode = instWord & 0x0000007F;
	rd = (instWord >> 7) & 0x0000001F;
	funct3 = (instWord >> 12) & 0x00000007;
	rs1 = (instWord >> 15) & 0x0000001F;
	rs2 = (instWord >> 20) & 0x0000001F;
	funct7 = (instWord >> 25) & 0x0000007F;

	// Extraction of I immediate
	I_imm = ((instWord >> 20) & 0x7FF) | (((instWord >> 31) ? 0xFFFFF800 : 0x0));
	
	//Extraction of B immediate 
	B_imm = ((instWord >> 7) & 0x1); 
    B_imm = (B_imm << 10);
	B_imm = (B_imm | ((instWord >> 8) & 0x00F) | 
		    ((instWord >> 21) & 0x3F0) |
		    ((instWord >> 20) & 0x800));
	B_imm = B_imm | ((instWord >> 31) ? 0xFFFFF800 : 0x0);
	B_imm = B_imm << 1;

	//Extraction of U immediate 
	U_imm = ((instWord >> 12) & 0xFFFFF) | (((instWord >> 31) ? 0xFFF00000 : 0x0));

	printPrefix(instPC, instWord);
 
	if(opcode == 0x33)  // R instructions
	{		
		
	} 
	else if(opcode == 0x13)  // I instructions
	{	
		switch(funct3)
		{
			case 0:	
			{
				//tested
				cout << "\tADDI\t" << name[rd] << ", " << name[rs1] << ", " << dec << (int)I_imm << "\n";
				reg[rd] = reg[rs1] + I_imm;
				break;
			}
			case 1:
			{
				//tested
				cout << "\tSLLI\t" << name[rd] << ", " << name[rs1] << ", " << dec << (int)I_imm << "\n";
				I_imm = I_imm & 0x1F;
				reg[rd] = reg[rs1] << I_imm;
				break;
				
			}
			case 2:
			{ 
				// not tested yet
				cout << "\tSLTI\t" << name[rd] << ", " << name[rs1] << ", " << dec << (int)I_imm << "\n";
				reg[rd] = (reg[rs1] < I_imm) ? 1 : 0;
				break;
			}
			case 3:
			{
				// not tested yet
				cout << "\tSLTIU\tx" << name[rd] << ", " << name[rs1] << ", " << dec << (int)I_imm << "\n";
				reg[rd] = ((unsigned int)reg[rs1] < I_imm) ? 1 : 0;
                break;
			}
			case 4:
			{	//tested
				cout << "\tXORI\tx" << name[rd] << ", " << name[rs1] << ", " << dec << (int)I_imm << "\n";
				reg[rd] = reg[rs1] ^ I_imm;
				break;
			}
			case 5:
			{
				if (funct7 == 0x20)
				{
					I_imm = I_imm & 0x1F;
					cout << "\tSRAI\t" << name[rd] << ", " << name[rs1] << ", " << dec << (int)I_imm << "\n";
					reg[rd] = reg[rs1] >> I_imm;
					// cout << dec << reg[rd] << "\n"; for debugging
				} 
				else
				{
					I_imm = I_imm & 0x1F;
					cout << "\tSRLI\t" << name[rd] << ", " << name[rs1] << ", " << dec << (int)I_imm << "\n";
					reg[rd] = (unsigned int)reg[rs1] >> I_imm;
				} 
				break;
			}
			case 6:
			{
				cout << "\tORI\t" << name[rd] << ", " << name[rs1] << ", " << dec << (int)I_imm << "\n";
				reg[rd] = reg[rs1] | I_imm;
				break;
			}
			case 7:
			{
				cout << "\tANDI\t" << name[rd] << ", " << name[rs1] << ", " << dec << (int)I_imm << "\n";
				reg[rd] = reg[rs1] & I_imm;
				break;
			}
			default:
				cout << "\tUnkown I Instruction \n";
		}
	} 
	else if(opcode == 0x63)  // B instructions
	{	
		switch(funct3)
		{
			case 0:	
			{
				break;
			}
			case 1:
			{
				break;
				
			}
			case 4:
			{ 
				cout << "\tBLT\t" << name[rs1] << ", " << name[rs2] << ", " << hex << "0x" << instPC + (int)B_imm << "\n";
				if (reg[rs1] < reg[rs2])
					pc = instPC + (int)B_imm;
				break;
			}
			case 5:
			{
				cout << "\tBGE\t" << name[rs1] << ", " << name[rs2] << ", " <<  hex << "0x" << instPC + (int)B_imm << "\n";
                if (reg[rs1] >= reg[rs2])
					pc = instPC + (int)B_imm;
				break;
			}
			case 6:
			{
				break;
			}
			case 7:
			{
				break;
			}
			default:
				cout << "\tUnkown B Instruction \n";
		}
	}
	else if (opcode == 0x37) //not done yet
	{
		
	}
	else if (opcode == 0x17) //not done yet 
	{
		
	}
	else 
	{
		cout << "\tUnkown Instruction \n";
	}

}

int main(int argc, char *argv[])
{

	unsigned int instWord=0;
	ifstream inFile;
	ofstream outFile;

	if(argc<1) emitError("use: rvcdiss <machine_code_file_name>\n");

	inFile.open(argv[1], ios::in | ios::binary | ios::ate);

	if(inFile.is_open())
	{
		int fsize = inFile.tellg();

		inFile.seekg (0, inFile.beg);
		if(!inFile.read((char *)memory, fsize)) emitError("Cannot read from input file\n");

		while(true)
		{
			instWord = 	(unsigned char)memory[pc] |
						(((unsigned char)memory[pc+1])<<8) |
						(((unsigned char)memory[pc+2])<<16) |
						(((unsigned char)memory[pc+3])<<24);
			pc += 4;
			// remove the following line once you have a complete simulator
			//if(pc==32) break;			// stop when PC reached address 32
			instDecExec(instWord);
			if (instWord == 0)
            {
				break;
            }
		}
	} else emitError("Cannot access input file\n");
}

