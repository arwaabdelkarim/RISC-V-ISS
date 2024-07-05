#include <iostream>
#include <fstream>
#include "stdlib.h"
#include <iomanip>
#include <string>
#include <bitset>
using namespace std;

struct registers
{
	string name;
	int value;
};


registers reg[32]={{"zero",0},{"ra"},{"sp"},{"gp"},{"tp"},{"t0"},{"t1"},{"t2"},{"s0"},{"s1"},{"a0"},{"a1"},
				   {"a2"},{"a3"},{"a4"},{"a5"},{"a6"},{"a7"},{"s2"},{"s3"},{"s4"},{"s5"},{"s6"},{"s7"},{"s8"},
				   {"s9"},{"s10"},{"s11"},{"t3"},{"t4"},{"t5"},{"t6"}};

unsigned int pc = 0;
unsigned char memory[(64+64)*1024]; 



void emitError(char *s)
{ 
	cout << s;
	exit(0);
}


void printPrefix(unsigned int instA, unsigned int instW)
{
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



void instDecExec(unsigned int instWord, bool compressed)
{
	unsigned int rd, rs1, rs2, funct3, funct7, opcode;
	unsigned int I_imm = 0x0, S_imm = 0x0, B_imm = 0x0, U_imm = 0x0, J_imm = 0x0, J_temp = 0x0, temp;
	unsigned int address;
	unsigned int instPC = pc - 4;
	unsigned int instPC_C = pc - 2;

	printPrefix(instPC, instWord);

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
	B_imm = (B_imm << 10 );
	B_imm = (B_imm | ((instWord >> 8) & 0x00F) |
			((instWord >> 21) & 0x3F0) |
			((instWord >> 20) & 0x800));
	B_imm = B_imm | ((instWord >> 31) ? 0xFFFFF800 : 0x0);
	B_imm = B_imm << 1; //multiplying by 2 

	//Extraction of U immediate 
	U_imm = ((instWord >> 12) & 0xFFFFF) | (((instWord >> 31) ? 0xFFF00000 : 0x0));

	// - inst[31] - inst[30:25] - inst[11:7]
	//temp = (((instWord >> 7) & 0x0000001F) | ((instWord >> 25 & 0x0000003F) << 5));
	//S_imm = temp | (((instWord >> 31) ? 0xFFFFF800 : 0x0));

	//Extraction of S immediate 
	S_imm = (((instWord >> 7) & 0x0000001F) |
		((instWord >> 20 & 0x00000FE0))) |
		((instWord >> 31) ? 0xFFFFF800 : 0x0);

	//Extraction of J immediate 
	// J_temp = (instWord >> 20) & 0x000003FF;
	// J_temp = J_temp << 1;
	// J_imm = J_imm | J_temp;
	// J_temp = (instWord >> 19) & 0x00000001;
	// J_temp = J_temp << 11;
	// J_imm = J_imm | J_temp;
	// J_temp = (instWord >> 12) & 0x000000FF;
	// J_temp = J_temp << 12;
	// J_imm = J_imm | J_temp;
	// J_temp = (instWord >> 31) & 0x00000001;
	// J_temp = J_temp << 20;
	// J_imm = J_imm | J_temp;

	J_imm = ((instWord >> 31 & 0x1 ) << 20) |
			((instWord >> 21 & 0x3FF) << 1) |
			((instWord >> 20 & 0x1) << 11) |
			((instWord >> 12 & 0xFF) << 12);
	J_imm |= (((instWord >> 31) ? 0xFFF00000 : 0x0));

	J_imm |= (((instWord >> 31) ? 0xFFF00000 : 0x0));

	// R Instructions
	if (opcode == 0x33) 
	{
		switch (funct3) 
		{
			case 0: 
			{
				if (funct7 == 32)
				{
					cout << "\tSUB\t" << reg[rd].name << "," << reg[rs1].name << "," << reg[rs2].name << "\n";
					if (rd == 0) return; // to keep reg zero unchanged
					reg[rd].value = reg[rs1].value - reg[rs2].value;
				}
				else
				{
					cout << "\tADD\t" << reg[rd].name << "," << reg[rs1].name << "," << reg[rs2].name << "\n";
					if (rd == 0) return; // to keep reg zero unchanged
					reg[rd].value = reg[rs1].value + reg[rs2].value;
				}
			}
				break;

			case 1: 
			{
				cout << "\tSLL\t" << reg[rd].name << "," << reg[rs1].name << "," << reg[rs2].name << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = reg[rs1].value << reg[rs2].value;
			}
				break;

			case 2: 
			{
				cout << "\tSLT\t" << reg[rd].name << "," << reg[rs1].name << "," << reg[rs2].name << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = (reg[rs1].value < reg[rs2].value) ? 1 : 0;
			}
				break;
			case 3: 
			{
				cout << "\tSLTU\t" << reg[rd].name << "," << reg[rs1].name << "," << reg[rs2].name << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = ((unsigned int)reg[rs1].value < (unsigned int)reg[rs2].value) ? 1 : 0;
			}
				break;

			case 4: 
			{
				cout << "\tXOR\t" << reg[rd].name << "," << reg[rs1].name << "," << reg[rs2].name << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = reg[rs1].value ^ reg[rs2].value;
			}
				break;

			case 5: 
			{
				if (funct7 == 32)
				{
					cout << "\tSRA\t" << reg[rd].name << "," << reg[rs1].name << "," << reg[rs2].name << "\n";
					if (rd == 0) return; // to keep reg zero unchanged
					reg[rd].value = reg[rs1].value >> reg[rs2].value;
				}
				else
				{
					cout << "\tSRL\t" << reg[rd].name << "," << reg[rs1].name << "," << reg[rs2].name << "\n";
					if (rd == 0) return; // to keep reg zero unchanged
					reg[rd].value = (unsigned int)reg[rs1].value >> reg[rs2].value;
				}
			}
				break;
			case 6: 
			{
				cout << "\tOR\t" << reg[rd].name << "," << reg[rs1].name << "," << reg[rs2].name << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = reg[rs1].value | reg[rs2].value;
			}
				break;
			case 7: 
			{
				cout << "\tAND\t" << reg[rd].name << "," << reg[rs1].name << "," << reg[rs2].name << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = reg[rs1].value & reg[rs2].value;
			}
				break;
		
			default: cout << "\tUnkown R Instruction \n";
		}
	}

	// I instructions
	else if (opcode == 0x13)  
	{
		switch (funct3)
		{
			case 0:
			{
				cout << "\tADDI\t" << reg[rd].name << ", " << reg[rs1].name << ", " << dec << (int)I_imm << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = reg[rs1].value + I_imm;
			}
				break;
			case 1:
			{
				cout << "\tSLLI\t" << reg[rd].name << ", " << reg[rs1].name << ", " << dec << (int)I_imm << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				I_imm = I_imm & 0x1F;
				reg[rd].value = reg[rs1].value << (unsigned int)I_imm;
			}
				break;
			case 2:
			{
				// not tested yet
				cout << "\tSLTI\t" << reg[rd].name << ", " << reg[rs1].name << ", " << dec << (int)I_imm << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = (reg[rs1].value < I_imm) ? 1 : 0;
			}
				break;
			case 3:
			{
				cout << "\tSLTIU\t" << reg[rd].name << ", " << reg[rs1].name << ", " << dec << (int)I_imm << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = ((unsigned int)reg[rs1].value < I_imm) ? 1 : 0;
			}
				break;
			case 4:
			{
				cout << "\tXORI\t" << reg[rd].name << ", " << reg[rs1].name << ", " << dec << (int)I_imm << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = reg[rs1].value ^ I_imm;
			}
				break;
			case 5:
			{
				if (funct7 == 0x20)
				{
					I_imm = I_imm & 0x1F;
					cout << "\tSRAI\t" << reg[rd].name << ", " << reg[rs1].name << ", " << dec << (int)I_imm << "\n";
					if (rd == 0) return; // to keep reg zero unchanged
					reg[rd].value = reg[rs1].value >> (unsigned int)I_imm;
					// cout << dec << reg[rd] << "\n"; for debugging
				}
				else
				{
					I_imm = I_imm & 0x1F;
					cout << "\tSRLI\t" << reg[rd].name << ", " << reg[rs1].name << ", " << dec << (int)I_imm << "\n";
					if (rd == 0) return; // to keep reg zero unchanged
					reg[rd].value = (unsigned int)reg[rs1].value >> (unsigned int)I_imm;
				}
			}
				break;
			case 6:
			{
				cout << "\tORI\t" << reg[rd].name << ", " << reg[rs1].name << ", " << dec << (int)I_imm << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = reg[rs1].value | I_imm;
			}
				break;
			case 7:
			{
				cout << "\tANDI\t" << reg[rd].name << ", " << reg[rs1].name << ", " << dec << (int)I_imm << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = reg[rs1].value & I_imm;
			}
				break;
			default:
				cout << "\tUnkown I Instruction \n";
		}
	}

	// I - Load Instructions
	else if (opcode == 0x03) 
	{ 
		//stores the address of the LS byte we need
		int ad = reg[rs1].value + (int)I_imm; 
		switch (funct3) 
		{
			case 0: 
			{
				cout << "\tLB\t" << reg[rd].name << ", " << dec << (int)I_imm << "(" << reg[rs1].name << ")\n";
				if (rd == 0) return; // to keep reg zero unchanged
				temp = memory[ad];
				// sign extending
				reg[rd].value = memory[ad] | ((temp >> 7) ? 0xFFFFFF00 : 0x0); 
			}
				break;

			case 1:	
			{
				cout << "\tLH\tx" << reg[rd].name << ", " << dec << (int)I_imm << "(" << reg[rs1].name << ")\n";
				if (rd == 0) return; // to keep reg zero unchanged
				temp = mem_to_reg(ad, 1);
				// sign extending
				reg[rd].value = temp | ((temp >> 15) ? (0xFFFF0000) : 0x0); 
			}
				break;

			case 2:	
			{
				cout << "\tLW\tx" << reg[rd].name << ", " << dec << (int)I_imm << "(" << reg[rs1].name << ")\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = mem_to_reg(ad, 3);
			}
				break;

			case 4:	
			{
				cout << "\tLBU\tx" << reg[rd].name << ", " << dec << (int)I_imm << "(" << reg[rs1].name << ")\n";
				if (rd == 0) return; // to keep reg zero unchanged
				// automatically zero extended because memory is unsigned char
				reg[rd].value = memory[ad];
			}
				break;

			case 5:	
			{
				cout << "\tLHU\tx" << reg[rd].name << ", " << dec << (int)I_imm << "(" << reg[rs1].name << ")\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = mem_to_reg(ad, 1);
			}
				break;

			default:
				cout << "\tUnkown I-Load Instruction \n";
		}
	}
  
	// S Instructions
	else if (opcode == 0x23) 	
	{	
		//stores the address of the LS byte we need
		int ad = reg[rs1].value + (int)S_imm; 
		switch (funct3)
		{
			case 0:	
			{
				cout << "\tSB\t" << reg[rs2].name << ", " << dec << (int)S_imm << "(" << reg[rs1].name << ")\n";
				// the memory address gets the LS byte in reg[rs2]
				memory[ad] = reg[rs2].value; 
			}
				break;

			case 1:	
			{
				cout << "\tSH\t" << reg[rs2].name << ", " << dec << (int)S_imm << "(" << reg[rs1].name << ")\n";
				memory[ad] = reg[rs2].value;
				memory[ad + 1] = reg[rs2].value >> 8;
			}
				break;

			case 2: 
			{
				cout << "\tSW\t" << reg[rs2].name << ", " << dec << (int)S_imm << "(" << reg[rs1].name << ")\n";
				memory[ad] = reg[rs2].value;
				memory[ad + 1] = reg[rs2].value >> 8;
				memory[ad + 2] = reg[rs2].value >> 16;
				memory[ad + 3] = reg[rs2].value >> 24;
			}
				break;

			default:
				cout << "\tUnkown S Instruction \n";
		}

	}

	// B instructions
	else if (opcode == 0x63) 
	{
		switch (funct3) 
		{
			case 0:
			{
				cout << "\tBEQ\t" << reg[rs1].name << "," << reg[rs2].name << "," << hex << "0x" << instPC + (int)B_imm << "\n";
				pc = (reg[rs1].value == reg[rs2].value) ? instPC + (int)B_imm : pc;
			}
				break;

			case 1:
			{
				cout << "\tBNE\t" << reg[rs1].name << "," << reg[rs2].name << "," << hex << "0x" << instPC + (int)B_imm << "\n";
				pc = (reg[rs1].value != reg[rs2].value) ? instPC + (int)B_imm : pc;
			}
				break;

			case 4:
			{
				cout << "\tBLT\t" << reg[rs1].name << ", " << reg[rs2].name << ", " << hex << "0x" << instPC + (int)B_imm << "\n";
				if (reg[rs1].value < reg[rs2].value)
					pc = instPC + (int)B_imm;	
			}
				break;

			case 5:
			{
				cout << "\tBGE\t" << reg[rs1].name << ", " << reg[rs2].name << ", " << hex << "0x" << instPC + (int)B_imm << "\n";
				if (reg[rs1].value >= reg[rs2].value)
					pc = instPC + (int)B_imm;
			}
				break;

			case 6:
			{
				cout << "\tBLTU\t" << reg[rs1].name << ", " << reg[rs2].name << ", 0x" << hex << (instPC + B_imm) << "\n";
				if ((unsigned int)reg[rs1].value < (unsigned int)reg[rs2].value) pc = instPC + (int)B_imm;
			}
				break;

			case 7:
			{
				cout << "\tBGEU\t" << reg[rs1].name << ", " << reg[rs2].name << ", 0x" << hex << (instPC + B_imm) << "\n";
				if ((unsigned int)reg[rs1].value >= (unsigned int)reg[rs2].value) pc = instPC + (int)B_imm;
			}
				break;

			default:
				cout << "\tUnkown B Instruction \n";
		}
	}

	// U instructions 
	else if (opcode == 0x37) 
	{
		cout << "\tLUI\t" << reg[rd].name << ", " << dec << ((int)U_imm) << "\n";
		if (rd == 0) return; // to keep reg zero unchanged
		U_imm <<= 12;  //shifting 12 bits to the right to load to upper 20 bits in rd
		reg[rd].value = (int)U_imm;
		// cout << reg[rd].value << endl; for debugging
	}
	else if (opcode == 0x17) 
	{

		cout << "\tAUIPC\t" << reg[rd].name << ", " << dec << ((int)U_imm) << "\n";
		if (rd == 0) return; // to keep reg zero unchanged


		U_imm <<= 12;
		reg[rd].value = instPC + (int)U_imm;
		// cout << reg[rd].value << endl; for debugging
	}

	// J instructions
	else if (opcode == 0x6F)
	{
		cout << "\tJAL\t" << reg[rd].name << "," << hex << "0x" << instPC + (int)J_imm << "\n";
		if (rd != 0) reg[rd].value = pc; // to keep reg zero unchanged
		pc = (compressed)? instPC_C+J_imm : instPC + J_imm;
	}
	else if (opcode == 0x67)
	{
		cout << "\tJALR\t" << reg[rd].name << "," << reg[rs1].name << "," << hex << "0x" << reg[rs1].value + (int)I_imm << "\n";
		if (rd != 0) reg[rd].value = pc; // to keep reg zero unchanged
		pc =reg[rs1].value + (int)I_imm;
	}
  
	// Ecall instruction
	else if (opcode == 0x73)
	{
		cout << "\tECALL\n";
		if (reg[17].value == 1)  // a7 = 1
			cout << dec << reg[10].value << '\n';

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
			cout << output <<'\n';
		}
		else if (reg[17].value == 10)
		{
			exit(0);
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

void Decompress(unsigned int instHalf)
{
	unsigned int opcode, rs2, rs1, rd, func4, rd_dash, rs1_dash, rs2_dash, offset, func3, func, func6, instWord = 0x0;
	unsigned int CJ_imm = 0x0, CI_imm = 0x0, CSS_imm = 0x0, CIW_imm = 0x0, CL_imm = 0x0, CS_imm = 0x0, CB_imm = 0x0;
	
	opcode = instHalf & 0x00000003;
	rs2 = (instHalf >> 2) & 0x0000001F;
	rd = (instHalf>> 7) & 0x0000001F;
	rs1 = (instHalf>> 7) & 0x0000001F;
	func4 = (instHalf>> 12) & 0x0000000F;
	func3 = (instHalf>> 13) & 0x00000007;
	rd_dash = (instHalf>> 2) & 0x00000007;
	rs2_dash = (instHalf>> 2) & 0x00000007;
	rs1_dash = (instHalf>> 7) & 0x00000007;
	func = (instHalf>> 5) & 0x00000003;
	func6 = (instHalf>> 10) & 0x003F;
	rd_dash = rd_dash | 0x08;
	rs2_dash = rs2_dash | 0x08;
	rs1_dash = rs1_dash | 0x08;

	unsigned int instPC = pc - 2;

	printPrefix(instPC, instHalf);

	// CSS immediate
	CSS_imm = (instHalf >> 9) & 0x00F;
	CSS_imm = CSS_imm | (((instHalf >> 7) & 0x0003) << 4);
	CSS_imm = CSS_imm << 2;
	CSS_imm = CSS_imm | (0x0 << 8);

	// CL immediate
	CL_imm = (instHalf >> 6) & 0x0001;
	CL_imm = CL_imm | (((instHalf >> 10) & 0x0007) << 1);
	CL_imm = CL_imm | (((instHalf >> 5) & 0x0001) << 4);
	CL_imm = CL_imm << 2;
	CL_imm = CL_imm | (0x0000 << 7);

	// CS immediate 
	CS_imm = (instHalf >> 6) & 0x0001;
	CS_imm = CS_imm | (((instHalf >> 10) & 0x0007) << 1);
	CS_imm = CS_imm | (((instHalf >> 5) & 0x0001) << 4);
	CS_imm = CS_imm << 2;
	CS_imm = CS_imm | (0x0000 << 7);



	if (opcode == 0x1)
	{
		switch (func)
		{
			case 0:
			{
				// c.sub
				if(func6 == 0x23)
				{
					// opcode
					instWord = 0x33;
					// rd
					instWord = instWord | (rs1_dash << 7);
					// fucntion 3
					instWord = instWord | (0x0 << 12);
					// rs1
					instWord = instWord | (rs1_dash << 15);
					// rs2
					instWord = instWord | (rs2_dash << 20);
					// function 7
					instWord = instWord | (0x20 << 25);

					cout<<"\tC.SUB\t"<<reg[rs1_dash].name<<","<< reg[rs2_dash].name << endl;
				}

					
			}
				break;	

			case 1:
			{
				// c.xor
				if(func6 == 0x23)
				{
					// opcode
					instWord = 0x33;
					// rd
					instWord = instWord | (rs1_dash << 7);
					// function 3
					instWord = instWord | (0x4 << 12);
					// rs1
					instWord = instWord | (rs1_dash << 15);
					// rs2
					instWord = instWord | (rs2_dash << 20);
					// function 7
					instWord = instWord | (0x00 << 25);

					cout<<"\tC.XOR\t"<<reg[rs1_dash].name<<","<< reg[rs2_dash].name << endl;
				}
			}
				break;

			case 2:
			{
				// c.or
				if(func6 == 0x23)
				{
					// opcode
					instWord = 0x33;
					// rd
					instWord = instWord | (rs1_dash << 7);
					// function 3
					instWord = instWord | (0x6 << 12);
					// rs1
					instWord = instWord | (rs1_dash << 15);
					// rs2
					instWord = instWord | (rs2_dash << 20);
					// function 7
					instWord = instWord | (0x00 << 25);

					cout<<"\tC.OR\t"<<reg[rs1_dash].name<<","<< reg[rs2_dash].name << endl;
				}
			}
				break;

			case 3:
			{
				// c.and
				if(func6 == 0x23)
				{
					// opcode
					instWord = 0x33;
					// rd
					instWord = instWord | (rs1_dash << 7);
					// function 3
					instWord = instWord | (0x7 << 12);
					// rs1
					instWord = instWord | (rs1_dash << 15);
					// rs2
					instWord = instWord | (rs2_dash << 20);
					// function 7
					instWord = instWord | (0x00 << 25);

					cout<<"\tC.AND\t"<<reg[rs1_dash].name<<","<< reg[rs2_dash].name << endl;
				}
			}	
				break;	

			default:
			        cout<<"\tunknown compressed instruction\t"<<endl;	

		}

		switch (func3)
		{
			case 3:
			{
				 //addisp16
				if (rd == 2)
				{
					// adding the base (opcode -- rd -- funct3 -- rs1 )
					instWord = 0x00010113;
					// extracting the immediate

					// adding the immediate
					instWord |= ((CI_imm << 20) & 0xFFF00000);

				}

				// c.lui
				else if(rd != 0)
				{
					// CI immediate
					CI_imm = (instHalf >> 2) & 0x001F;
					CI_imm = CI_imm |(((instHalf >> 12) & 0x0001) << 5);
					CI_imm = CI_imm << 12;
					CI_imm |= (CI_imm & 0x020000) ? 0xFFFFFFC0000 : 0x0;

					// opcode
					instWord = 0x37;
					// rd
					instWord = instWord | (rd << 7);
					// imm[31:12]
					instWord = instWord | (CI_imm << 12);

					cout<<"\tC.LUI\t"<<reg[rs1].name<<","<< CI_imm << endl;
				}
			}
				break;

			case 0:
			{
				if(rs1 == 0) // c.nop
				{}
				// c.addi
				else
				{
					CI_imm = (instHalf >> 2) & 0x001F;
					CI_imm = CI_imm |(((instHalf >> 12) & 0x0001) << 5);
					CI_imm |= (CI_imm & 0x0020) ? 0xFFFFFFC0 : 0x0;
					cout<<"\tC.ADDI\t"<<reg[rs1].name<<","<< CI_imm << endl;

					CI_imm = CI_imm << 20;

					instWord = 0x13;
					instWord = instWord | (rs1 << 7);
					instWord = instWord | (0x0 <<12);
					instWord = instWord | (rs1 <<15);
					instWord = instWord | CI_imm;

					
				}
			}

			default: cout<<"\tunknown compressed instruction\t"<<endl;	
		}	
	}

	else if (opcode == 0x2)
	{
		switch (func3)
		{
			// c.swsp
			case 6:
			{
				// opcode
				instWord = 0x23;
				// imm [4:0]
				instWord = instWord | ((CSS_imm & 0x01F) << 7);
				// function 3
				instWord = instWord | (0x3 << 12);
				// stack pointer
				instWord = instWord | (0x02 << 15);
				// rs2
				instWord = instWord | (rs2 << 20);
				// imm[11:5]
				instWord = instWord | ((CSS_imm >> 5) & 0x07F);

				cout<<"\tC.SWSP\t"<<reg[rs2].name<<","<< CSS_imm << endl;
			}
				break;


			default: cout<<"\tunknown compressed instruction\t"<<endl;	

		}
	}

	else if (opcode == 0x0)
	{
		switch (func3)
		{
			// c.lw
			case 2:
			{
				// opcode
				instWord = 0x02;
				// rd
				instWord = instWord | (rd_dash << 7);
				// function 3
				instWord = instWord | (func3 << 12);
				// rs1
				instWord = instWord | (rs1_dash << 15);
				// imm[11:0]
				instWord = instWord | (CL_imm << 20);

				cout<<"\tC.LW\t"<<reg[rd_dash].name<<","<< CL_imm <<"("<<reg[rs1_dash].name<<")"<<endl;
			}
				break;

			// c.sw
			case 6:
			{
				// opcode
				instWord = 0x23;
				// imm[4:0]
				instWord = instWord | ((CS_imm & 0x001F) << 7);
				// function 3
				instWord = instWord | (0x2 << 12);
				// rs1
				instWord = instWord | (rs1_dash << 15);
				// rs2
				instWord = instWord | (rs2_dash << 20);
				// imm[11:5]
				instWord = instWord | (((CS_imm >> 5) & 0x007F) << 25);

				cout<<"\tC.SW\t"<<reg[rs2_dash].name<<","<< CS_imm <<"("<<reg[rs1_dash].name<<")"<<endl;
			}
				break;

			default: cout<<"\tunknown compressed instruction\t"<<endl;	
		}
	}

	// after decompressing the instruction halfword we will call the other function to execute the instruction
	instDecExec(instWord,1);
}

int main(int argc, char *argv[])
{
	unsigned int instWord = 0;
	ifstream inFile;
	ifstream dataFile;
	ofstream outFile;

	// for sp the initisl value is (64*64*1024) 
	// char *a = new char; for debugging purposes

	if(argc<2) 
		emitError("use: rvcdiss <machine_code_file_name>\n");


	inFile.open(argv[1], ios::in | ios::binary | ios::ate);

	// checking if we had a third argument input in the terminal; thus, we have a data file
	if (argc == 3)
		dataFile.open(argv[2], ios::in | ios::binary | ios::ate);

	if (dataFile.is_open())
	{	
		// current pos of file pointer, dictates size (last element?)
		int curr = dataFile.tellg(); 
		// move the file pointer to the beginning, 0 is the offset
		dataFile.seekg (0, dataFile.beg); 
		// read the data file into the memory array, curr shows the number of bits 
		if(!dataFile.read(/*a*/(char *)(memory + 0x00010000), curr)) 
			emitError("Cannot read from data file\n");
	}


	if(inFile.is_open())
	{
		int fsize = inFile.tellg();
		inFile.seekg (0, inFile.beg);
		if(!inFile.read((char *)memory, fsize)) 
			emitError("Cannot read from input file\n");
		
		while(true)
		{
			instWord = 	(unsigned char)memory[pc] |
						(((unsigned char)memory[pc+1])<<8);
						
			pc += 2;

			if (instWord == 0)
                break;
		    else if((instWord & 0x0003) == 0x3) // if decompressed
			{
				instWord = instWord | (((unsigned char)memory[pc])<<16) |
			               (((unsigned char)memory[pc+1])<<24);
				pc += 2;
				instDecExec(instWord,0);
			}	
			else
			    Decompress(instWord);
		}
	} else emitError("Cannot access input file\n");
}

