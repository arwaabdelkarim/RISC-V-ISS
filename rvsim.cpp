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

unsigned int pc;
unsigned char memory[(64+64)*1024];
int reg[32];
string name[32] = { "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
					"s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
					"s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
					"t3", "t4", "t5", "t6" };

void emitError(char *s)
{
	cout << s;
	exit(0);
}

void printPrefix(unsigned int instA, unsigned int instW){
	cout << "0x" << hex << std::setfill('0') << std::setw(8) << instA << "\t0x" << std::setw(8) << instW;
}

int mem_to_reg(int ad, int i) { // i = n-1;
	int x = 0;

	while (i > 0) {
		x |= memory[ad + i];
		x <<= 8; // need '=' for that!
		i--;
	}
	x |= memory[ad];

	return x;
}


void instDecExec(unsigned int instWord)
{
	unsigned int rd, rs1, rs2, funct3, funct7, opcode;
	unsigned int I_imm, S_imm, B_imm, U_imm, J_imm;
	unsigned int address;
	unsigned int temp;

	unsigned int instPC = pc - 4;

	opcode = instWord & 0x0000007F;
	rd = (instWord >> 7) & 0x0000001F;
	funct3 = (instWord >> 12) & 0x00000007;
	rs1 = (instWord >> 15) & 0x0000001F;
	rs2 = (instWord >> 20) & 0x0000001F;

	// — inst[31] — inst[30:25] inst[24:21] inst[20]
	I_imm = ((instWord >> 20) & 0x7FF) | (((instWord >> 31) ? 0xFFFFF800 : 0x0));

	// - inst[31] - inst[30:25] - inst[11:7]
	//temp = (((instWord >> 7) & 0x0000001F) | ((instWord >> 25 & 0x0000003F) << 5));
	//S_imm = temp | (((instWord >> 31) ? 0xFFFFF800 : 0x0));

	S_imm = (((instWord >> 7) & 0x0000001F) |
			((instWord >> 20 & 0x00000FE0))) |
			((instWord >> 31) ? 0xFFFFF800 : 0x0);
	

	B_imm = ((instWord >> 7) & 0x1); 
	B_imm <<= 10;
	B_imm |= (((instWord >> 8) & 0x00F) | 
		((instWord >> 21) & 0x3F0) |
		((instWord >> 20) & 0x800));
	B_imm |= ((instWord >> 31) ? 0xFFFFF800 : 0x0);
	B_imm  <<=  1;

	printPrefix(instPC, instWord);

	if(opcode == 0x33){		// R Instructions
		switch(funct3){
			case 0: if(funct7 == 32) {
								cout << "\tSUB\tx" << rd << ", x" << rs1 << ", x" << rs2 << "\n";
							}
							else {
								cout << "\tADD\tx" << rd << ", x" << rs1 << ", x" << rs2 << "\n";
							}
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
		int ad = reg[rs1] + (int)I_imm; //stores the address of the LS byte we need
		switch (funct3) {
		case 0: cout << "\tLB\t" <<  name[rd] << ", " << dec << (int)I_imm << "(" << name[rs1] << ")\n";
			temp = memory[ad];
			reg[rd] = memory[ad] | ((temp >> 7) ? 0xFFFFFF00 : 0x0;
			break;
		case 1:	cout << "\tLH\tx" << name[rd] << ", " << dec << (int)I_imm << "(" << name[rs1] << ")\n";
			temp = mem_to_reg(ad, 1);
			reg[rd] = temp | ((temp >> 15)? (0xFFFF0000): 0x0);
			break;
		case 2:	cout << "\tLW\tx" << name[rd] << ", " << dec << (int)I_imm << "(" << name[rs1] << ")\n";
			reg[rd] = mem_to_reg(ad, 3);
			break;
		case 4:	cout << "\tLBU\tx" << name[rd] << ", " << dec << (int)I_imm << "(" << name[rs1] << ")\n";
			reg[rd] = memory[ad]; // automatically zero extended because memory is unsigned char
			break;
		case 5:	cout << "\tLHU\tx" << name[rd] << ", " << dec << (int)I_imm << "(" << name[rs1] << ")\n";
			reg[rd] = mem_to_reg(ad, 1);
			break;
		default: 
			cout<< "\tUnkown I-Load Instruction \n";
		}
	}
	else if (opcode == 0x23) {	// S Instructions
		int ad = reg[rs1] + (int)S_imm; //stores the address of the LS byte we need
		switch (funct3) {
		case 0:	cout << "\tSB\tx" <<name[rs2] << ", " << dec << (int)S_imm << "(" << name[rs1] << ")\n";
			memory[ad] = reg[rs2]; // the memory address gets the LS byte in reg[rs2]
			break;
		case 1:	cout << "\tSH\tx" << name[rs2] << ", " << dec << (int)S_imm << "(" << name[rs1] << ")\n";
			memory[ad] = reg[rs2];
			memory[ad + 1] = reg[rs2] >> 8;
			break;
		case 2: cout << "\tSW\tx" << name [rs2] << ", " << dec << (int)S_imm << "(" << name[rs1] << ")\n";
			memory[ad] = reg[rs2];
			memory[ad + 1] = reg[rs2] >> 8;
			memory[ad + 2] = reg[rs2] >> 16;
			memory[ad + 3] = reg[rs2] >> 24;
			break;
		default:
			cout << "\tUnkown S Instruction \n";
		}
	}
	else if (opcode == 0x63) {
		switch (funct3) {
		case 6: cout << "\tBLTU\tx" << name[rs1] << ", " << name[rs2] << " address\n";
			break;
		case 7: cout << "\tBGEU\tx" << name[rs1] << ", " << name[rs2] << " address\n";
			break;
		default:
			cout << "\tUnkown B Instruction \n";
		}
	}
	else if (opcode == 73) {
		if (reg[17] == 1) { // a7 = 1
			int x = 0; // will hold our int
			int i = 3; // offset to reach most significant byte (little endian)

			while (i > 0) {
				x |= memory[reg[10] + i]; // captures the i-th byte by 'or'ing
				x <<= 8; // shift to allow place for next byte
				i--;
			}
			x |= memory[reg[10]]; // add last byte without shifting

			cout << x << '\n';
		}

		else if (reg[17] == 4) {
			char c = memory[reg[10]; // c = char at the address stored in a0
			string output = "";
			while (c != 0) { // as long as we havent reached a null
				output += c;
			}
			cout << output;
		}
		else {
			cout << "\tUnkown Ecall Instruction \n";
		}
	}
	else {
		cout << "\tUnkown Instruction \n";
	}

}

int main(int argc, char *argv[]){

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
