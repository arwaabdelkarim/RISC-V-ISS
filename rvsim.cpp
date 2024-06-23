#include <iostream>
#include <fstream>
#include "stdlib.h"
#include <iomanip>
#include <cstdint>

using namespace std;

unsigned reg[32] = {0};
unsigned int pc;
unsigned char memory[(16+64)*1024] = {0}; 

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

	// initializing I immediate
	I_imm = 0;

	opcode = instWord & 0x0000007F;
	rd = (instWord >> 7) & 0x0000001F;
	funct3 = (instWord >> 12) & 0x00000007;
	rs1 = (instWord >> 15) & 0x0000001F;
	rs2 = (instWord >> 20) & 0x0000001F;
	funct7 = (instWord >> 25) & 0x0000007F;

	// Extraction of I immediate
	I_imm = ((instWord >> 20) & 0x7FF) | (((instWord >> 31) ? 0xFFFFF800 : 0x0));

	printPrefix(instPC, instWord);

	if(opcode == 0x33)  // R instructions
	{		
		switch(funct3)
		{
			case 0: 
				if(funct7 == 32)
				{
					cout << "\tSUB\tx" << rd << ", x" << rs1 << ", x" << rs2 << "\n";
				}
				else 
				{
					cout << "\tADD\tx" << rd << ", x" << rs1 << ", x" << rs2 << "\n";
				}
			    break;
			default:
				cout << "\tUnkown R Instruction \n";
		}
	} 
	else if(opcode == 0x13)  // I instructions
	{	
		switch(funct3)
		{
			case 0:	
			{
				cout << "\tADDI\tx" << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
				reg[rd] = reg[rs1] + I_imm;
				break;
			}
			case 1:
			{
				cout << "\tSLLI\tx" << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
				I_imm = I_imm & 0x1F;
				reg[rd] = reg[rs1] << I_imm;
				break;
				
			}
			case 2:
			{ 
				cout << "\tSLTI\tx" << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
				reg[rd] = reg[rs1] < I_imm;
				break;
			}
			case 3:
			{
				cout << "\tSLTIU\tx" << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
				reg[rd] = (static_cast<uint32_t>(reg[rs1]) < static_cast<uint32_t>(I_imm)) ? 1 : 0;
                break;
			}
			case 4:
			{
				cout << "\tXORI\tx" << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
				reg[rd] = reg[rs1] ^ I_imm;
				break;
			}
			case 5:
			{
				if (funct7 == 0x20)
				{
					cout << "\tSRAI\tx" << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
					I_imm = I_imm & 0x1F;
					reg[rd] = reg[rs1] >> I_imm;
				}
				else
				{
					cout << "\tSRLI\tx" << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
					I_imm = I_imm & 0x1F;
					reg[rd] = reg[rs1] >> I_imm;
				}
				break;
			}
			case 6:
			{
				cout << "\tORI\tx" << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
				reg[rd] = reg[rs1] | I_imm;
				break;
			}
			case 7:
			{
				cout << "\tANDI\tx" << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
				reg[rd] = reg[rs1] & I_imm;
				break;
			}
			default:
				cout << "\tUnkown I Instruction \n";
		}
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
			if(pc==32) break;			// stop when PC reached address 32
			instDecExec(instWord);
		}
	} else emitError("Cannot access input file\n");
}
