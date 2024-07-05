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

// remove initialization of registers, we will initialize them in the main function
registers reg[32]={{"zero",0},{"ra",0},{"sp",4194304},{"gp",0},{"tp",0},{"t0",0},{"t1",0},{"t2",0},{"s0",0},{"s1",0},{"a0",0},{"a1",0},
				   {"a2",0},{"a3",0},{"a4",0},{"a5",0},{"a6",0},{"a7",0},{"s2",0},{"s3",0},{"s4",0},{"s5",0},{"s6",0},{"s7",0},{"s8",0},
				   {"s9",0},{"s10",0},{"s11",0},{"t3",0},{"t4",0},{"t5",0},{"t6",0}};

unsigned int pc = 0;
// 64KB for instructions and 64KB for data
unsigned char memory[(64+64)*1024]; 

void printRegisterContents()
{
	for (int i = 0; i < 32; i++)
	{
		cout << "x" << dec << i << "\t" << reg[i].name << "\t0x" << hex << reg[i].value << "\n";
	}
}

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
	unsigned int I_imm = 0x0, S_imm = 0x0, B_imm = 0x0, U_imm = 0x0, J_imm = 0x0, temp;
	unsigned int address;
	unsigned int instPC;
    if (compressed == 0)
    {
        instPC = pc - 4;
    }

    else if (compressed == 1)
    {

        instPC = pc - 2;
    }

	if (!compressed)
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
	B_imm |= ((instWord >> 31) ? 0xFFFFF800 : 0x0);
	B_imm <<= 1; //multiplying by 2 


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
	J_imm = ((instWord >> 31 & 0x1 ) << 20) |
			((instWord >> 21 & 0x3FF) << 1) |
			((instWord >> 20 & 0x1) << 11) |
			((instWord >> 12 & 0xFF) << 12);
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
					if (!compressed)
						cout << "\tSUB\t" << reg[rd].name << "," << reg[rs1].name << "," << reg[rs2].name << "\n";
					if (rd == 0) return; // to keep reg zero unchanged
					reg[rd].value = reg[rs1].value - reg[rs2].value;
				}
				else
				{
					if (!compressed)
						cout << "\tADD\t" << reg[rd].name << "," << reg[rs1].name << "," << reg[rs2].name << "\n";
					if (rd == 0) return; // to keep reg zero unchanged
					reg[rd].value = reg[rs1].value + reg[rs2].value;
				}
			}
				break;

			case 1: 
			{
				if (!compressed)
					cout << "\tSLL\t" << reg[rd].name << "," << reg[rs1].name << "," << reg[rs2].name << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = reg[rs1].value << reg[rs2].value;
			}
				break;

			case 2: 
			{
				if (!compressed)
					cout << "\tSLT\t" << reg[rd].name << "," << reg[rs1].name << "," << reg[rs2].name << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = (reg[rs1].value < reg[rs2].value) ? 1 : 0;
			}
				break;
			case 3: 
			{
				if (!compressed)
					cout << "\tSLTU\t" << reg[rd].name << "," << reg[rs1].name << "," << reg[rs2].name << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = ((unsigned int)reg[rs1].value < (unsigned int)reg[rs2].value) ? 1 : 0;
			}
				break;

			case 4: 
			{
				if (!compressed)
					cout << "\tXOR\t" << reg[rd].name << "," << reg[rs1].name << "," << reg[rs2].name << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = reg[rs1].value ^ reg[rs2].value;
			}
				break;

			case 5: 
			{
				if (funct7 == 32)
				{
					if (!compressed)
						cout << "\tSRA\t" << reg[rd].name << "," << reg[rs1].name << "," << reg[rs2].name << "\n";
					if (rd == 0) return; // to keep reg zero unchanged
					reg[rd].value = reg[rs1].value >> reg[rs2].value;
				}
				else
				{
					if (!compressed)
						cout << "\tSRL\t" << reg[rd].name << "," << reg[rs1].name << "," << reg[rs2].name << "\n";
					if (rd == 0) return; // to keep reg zero unchanged
					reg[rd].value = (unsigned int)reg[rs1].value >> reg[rs2].value;
				}
			}
				break;
			case 6: 
			{
				if (!compressed)
					cout << "\tOR\t" << reg[rd].name << "," << reg[rs1].name << "," << reg[rs2].name << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = reg[rs1].value | reg[rs2].value;
			}
				break;
			case 7: 
			{
				if (!compressed)
					cout << "\tAND\t" << reg[rd].name << "," << reg[rs1].name << "," << reg[rs2].name << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = reg[rs1].value & reg[rs2].value;
			}
				break;
		
			default: if (!compressed) 
				cout << "\tUnkown R Instruction \n";
		}
	}

	// I instructions
	else if (opcode == 0x13)  
	{
		switch (funct3)
		{
			case 0:
			{
				if (!compressed)
					cout << "\tADDI\t" << reg[rd].name << ", " << reg[rs1].name << ", " << dec << (int)I_imm << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = reg[rs1].value + I_imm;
			}
				break;
			case 1:
			{
				if (!compressed)
					cout << "\tSLLI\t" << reg[rd].name << ", " << reg[rs1].name << ", " << dec << (int)I_imm << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				I_imm = I_imm & 0x1F;
				reg[rd].value = reg[rs1].value << (unsigned int)I_imm;
			}
				break;
			case 2:
			{
				if (!compressed)
					cout << "\tSLTI\t" << reg[rd].name << ", " << reg[rs1].name << ", " << dec << (int)I_imm << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = (reg[rs1].value < I_imm) ? 1 : 0;
			}
				break;
			case 3:
			{
				if (!compressed)
					cout << "\tSLTIU\t" << reg[rd].name << ", " << reg[rs1].name << ", " << dec << (int)I_imm << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = ((unsigned int)reg[rs1].value < I_imm) ? 1 : 0;
			}
				break;
			case 4:
			{
				if (!compressed)
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
					if (!compressed)
						cout << "\tSRAI\t" << reg[rd].name << ", " << reg[rs1].name << ", " << dec << (int)I_imm << "\n";
					if (rd == 0) return; // to keep reg zero unchanged
					reg[rd].value = reg[rs1].value >> (unsigned int)I_imm;
					// cout << dec << reg[rd] << "\n"; for debugging
				}
				else
				{
					I_imm = I_imm & 0x1F;
					if (!compressed)
						cout << "\tSRLI\t" << reg[rd].name << ", " << reg[rs1].name << ", " << dec << (int)I_imm << "\n";
					if (rd == 0) return; // to keep reg zero unchanged
					reg[rd].value = (unsigned int)reg[rs1].value >> (unsigned int)I_imm;
				}
			}
				break;
			case 6:
			{
				if (!compressed)
					cout << "\tORI\t" << reg[rd].name << ", " << reg[rs1].name << ", " << dec << (int)I_imm << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = reg[rs1].value | I_imm;
			}
				break;
			case 7:
			{
				if (!compressed)
					cout << "\tANDI\t" << reg[rd].name << ", " << reg[rs1].name << ", " << dec << (int)I_imm << "\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = reg[rs1].value & I_imm;
			}
				break;
			default:
				if (!compressed) 
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
				if (!compressed)
					cout << "\tLB\t" << reg[rd].name << ", " << dec << (int)I_imm << "(" << reg[rs1].name << ")\n";
				if (rd == 0) return; // to keep reg zero unchanged
				temp = memory[ad];
				// sign extending
				reg[rd].value = memory[ad] | ((temp >> 7) ? 0xFFFFFF00 : 0x0); 
			}
				break;

			case 1:	
			{
				if (!compressed)
					cout << "\tLH\tx" << reg[rd].name << ", " << dec << (int)I_imm << "(" << reg[rs1].name << ")\n";
				if (rd == 0) return; // to keep reg zero unchanged
				temp = mem_to_reg(ad, 1);
				// sign extending
				reg[rd].value = temp | ((temp >> 15) ? (0xFFFF0000) : 0x0); 
			}
				break;

			case 2:	
			{
				if (!compressed)
					cout << "\tLW\tx" << reg[rd].name << ", " << dec << (int)I_imm << "(" << reg[rs1].name << ")\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = mem_to_reg(ad, 3);
			}
				break;

			case 4:	
			{
				if (!compressed)
					cout << "\tLBU\tx" << reg[rd].name << ", " << dec << (int)I_imm << "(" << reg[rs1].name << ")\n";
				if (rd == 0) return; // to keep reg zero unchanged
				// automatically zero extended because memory is unsigned char
				reg[rd].value = memory[ad];
			}
				break;

			case 5:	
			{
				if (!compressed)
					cout << "\tLHU\tx" << reg[rd].name << ", " << dec << (int)I_imm << "(" << reg[rs1].name << ")\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = mem_to_reg(ad, 1);
			}
				break;

			default:
				if (!compressed) 
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
				if (!compressed)
					cout << "\tSB\t" << reg[rs2].name << ", " << dec << (int)S_imm << "(" << reg[rs1].name << ")\n";
				// the memory address gets the LS byte in reg[rs2]
				memory[ad] = reg[rs2].value; 
			}
				break;

			case 1:	
			{
				if (!compressed)
					cout << "\tSH\t" << reg[rs2].name << ", " << dec << (int)S_imm << "(" << reg[rs1].name << ")\n";
				memory[ad] = reg[rs2].value;
				memory[ad + 1] = reg[rs2].value >> 8;
			}
				break;

			case 2: 
			{
				if (!compressed)
					cout << "\tSW\t" << reg[rs2].name << ", " << dec << (int)S_imm << "(" << reg[rs1].name << ")\n";
				memory[ad] = reg[rs2].value;
				memory[ad + 1] = reg[rs2].value >> 8;
				memory[ad + 2] = reg[rs2].value >> 16;
				memory[ad + 3] = reg[rs2].value >> 24;
			}
				break;

			default:
				if (!compressed) 
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
				if (!compressed)
					cout << "\tBEQ\t" << reg[rs1].name << "," << reg[rs2].name << "," << hex << "0x" << instPC + (int)B_imm << "\n";
				pc = (reg[rs1].value == reg[rs2].value) ? instPC + (int)B_imm : pc;
			}
				break;

			case 1:
			{
				if (!compressed)
					cout << "\tBNE\t" << reg[rs1].name << "," << reg[rs2].name << "," << hex << "0x" << instPC + (int)B_imm << "\n";
				pc = (reg[rs1].value != reg[rs2].value) ? instPC + (int)B_imm : pc;
			}
				break;

			case 4:
			{
				if (!compressed)
					cout << "\tBLT\t" << reg[rs1].name << ", " << reg[rs2].name << ", " << hex << "0x" << instPC + (int)B_imm << "\n";
				if (reg[rs1].value < reg[rs2].value)
					pc = instPC + (int)B_imm;	
			}
				break;

			case 5:
			{
				if (!compressed)
					cout << "\tBGE\t" << reg[rs1].name << ", " << reg[rs2].name << ", " << hex << "0x" << instPC + (int)B_imm << "\n";
				if (reg[rs1].value >= reg[rs2].value)
					pc = instPC + (int)B_imm;
			}
				break;

			case 6:
			{
				if (!compressed)
					cout << "\tBLTU\t" << reg[rs1].name << ", " << reg[rs2].name << ", 0x" << hex << (instPC + B_imm) << "\n";
				if ((unsigned int)reg[rs1].value < (unsigned int)reg[rs2].value) pc = instPC + (int)B_imm;
			}
				break;

			case 7:
			{
				if (!compressed)
					cout << "\tBGEU\t" << reg[rs1].name << ", " << reg[rs2].name << ", 0x" << hex << (instPC + B_imm) << "\n";
				if ((unsigned int)reg[rs1].value >= (unsigned int)reg[rs2].value) pc = instPC + (int)B_imm;
			}
				break;

			default:
				if (!compressed)
					cout << "\tUnkown B Instruction \n";
		}
	}

	// U instructions 
	else if (opcode == 0x37) 
	{
		if (!compressed)
			cout << "\tLUI\t" << reg[rd].name << ",0x" << hex << ((int)U_imm) << "\n";
		if (rd == 0) return; // to keep reg zero unchanged
		U_imm <<= 12;  //shifting 12 bits to the right to load to upper 20 bits in rd
		reg[rd].value = (int)U_imm;
		// cout << reg[rd].value << endl; for debugging
	}
	else if (opcode == 0x17) 
	{

		if (!compressed)
			cout << "\tAUIPC\t" << reg[rd].name << ", 0x" << hex << ((int)U_imm) << "\n";
		if (rd == 0) return; // to keep reg zero unchanged
		U_imm <<= 12;
		reg[rd].value = instPC + (int)U_imm;
		// cout << reg[rd].value << endl; for debugging
	}

	// J instructions
	else if (opcode == 0x6F)
	{
		if (!compressed)
			cout << "\tJAL\t" << reg[rd].name << "," << hex << "0x" << instPC + (int)J_imm << "\n";
		pc = instPC + J_imm;
		if (rd == 0) return; // to keep reg zero unchanged
		reg[rd].value = pc;
	}
	else if (opcode == 0x67)
	{
		if (!compressed)
			cout << "\tJALR\t" << reg[rd].name << "," << reg[rs1].name << "," << hex << "0x" << reg[rs1].value + (int)I_imm << "\n";
		pc = reg[rs1].value + (int)I_imm;
		if (rd == 0) return; // to keep reg zero unchanged
		reg[rd].value = pc;
	}
  
	// Ecall instruction
	else if (opcode == 0x73)
	{
		if (!compressed)
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
		if (!compressed)
			cout << "\tUnkown Instruction \n";
	}
}

void Decompress(unsigned int instHalf)
{
	unsigned int opcode, rs2, rs1, rd, func4, rd_dash, rs1_dash, rs2_dash, func3;
	unsigned int CJ_imm = 0x0, CI_imm = 0x0, CSS_imm = 0x0, CIW_imm = 0x0, CL_imm = 0x0, CS_imm = 0x0, CB_imm = 0x0, CJ_imm_D = 0x0;
	unsigned int instPC = pc - 2;
	unsigned int instWord;

	printPrefix(instPC, instHalf);
	
	opcode = instHalf & 0x00000003;
	rs2 = (instHalf >> 2) & 0x0000001F;
	rd = (instHalf>> 7) & 0x0000001F;
	rs1 = (instHalf>> 7) & 0x0000001F;
	func4 = (instHalf>> 12) & 0x0000000F;
	func3 = (instHalf>> 13) & 0x00000007;
	rd_dash = (instHalf>> 2) & 0x00000007;
	rs2_dash = (instHalf>> 2) & 0x00000007;
	rs1_dash = (instHalf>> 7) & 0x00000007;
	rd_dash |= 0x08;
	rs2_dash |= 0x08;
	rs1_dash |= 0x08;

	// CJ offset
	CJ_imm = ((instHalf >> 2) & 0x1);
	CJ_imm <<= 4; 
	CJ_imm = CJ_imm | ((instHalf >> 3) & 0x007);
	CJ_imm = CJ_imm | (instHalf & 0x040);
	CJ_imm = CJ_imm | ((instHalf >> 2) & 0x020);
	CJ_imm = CJ_imm | (instHalf & 0x200);
	CJ_imm = CJ_imm | ((instHalf >> 2) & 0x180);
	CJ_imm = CJ_imm | ((instHalf >> 8) & 0x008);
	CJ_imm = CJ_imm | ((instHalf >> 2) & 0x400);
	CJ_imm = CJ_imm | ((instHalf>> 12) ? 0xFFFFF800 : 0X0);
	CJ_imm_D = ((CJ_imm & 0x3FF) << 9);
	CJ_imm_D |= (CJ_imm  >> 11) & 0x0FF; 
	CJ_imm_D |= ((CJ_imm >> 2) & 0x100); 
	CJ_imm_D |= CJ_imm & 0x80000; 

	if (opcode == 0x1)
	{
		switch(func3)
		{
			// c.nop
			case 0:
			{
				cout << "\tC.Nop\t" << "\n";
				// set 32-bit instruction
				instWord = 0x00000013;
			}
				break;
			// c.jal
			case 1:
			{
				CJ_imm <<= 1;
				cout << "\tC.JAL\t" << "0x" << hex << instPC + (int)CJ_imm << "\n";
				//set immediate
				instWord = CJ_imm_D << 12;
				instWord |= 0x000000EF;	
			}
				break;
			// c.li
			case 2:
			{
				CI_imm = (instHalf >> 2) & 0x1F;
				CI_imm |= ((instHalf >> 12) & 0x01);
				CI_imm |= (((instHalf >> 12) & 0x01) ? 0xFFFFFFC0 : 0x0);
				cout << "\tC.LI\t" << reg[rd].name << ", " << "0x" << hex  << (int)CI_imm << "\n";
				// setting rd
				instWord = rd << 7;
				// setting immediate
				instWord |= CI_imm << 20;
				// setting base (rs1 - func3 - op)
				instWord |= 0x00000013;
			}
				break;
			// c.j
			case 5:
			{	CJ_imm <<= 1;
				cout << "\tC.J\t" << "0x" << hex << instPC + (int)CJ_imm << "\n";
				instWord = CJ_imm_D << 12;
				instWord |= 0x000000EF;
			}
				break;
			
		}

	}
	else if (opcode == 0x2)
	{
		switch(func3)
		{
			case 4:
			{
				// c.jr
				if (func4 == 0x8 && rs2 == 0x0)
				{
				    cout << "\tC.JR\t" << reg[rs1].name << "\n";
					// adding rs1
					instWord = rs1 << 15;
					// setting base (fn - op - rd - imm)
					instWord |= 0x00000067;
				}
				// c.mv
				else if (func4 == 0x8 && rs2 != 0x0)
				{
					cout << "\tC.MV\t" << reg[rd].name << ", " << reg[rs2].name << "\n";
					// setting rs2
					instWord = rs2 << 20;
					// setting rd
					instWord |= ((instHalf >> 7) & 0x1F) << 7;
					// setting base (func7 - func3 - op)
					instWord |= 0x00000033;
				}
				// c.jalr
				if (func4 == 0x9 && rs2 == 0x0)
				{
					cout << "\tC.JALR\t" << reg[rs1].name << "\n";
					// setting rs1
					instWord = rs1 << 15;
					// setting base (fn - op - rd - imm)
					instWord |= 0x000000E7;
				}
				// c.add
				else if (func4 == 0x9 && rs2 != 0x0)
				{
					cout << "\tC.ADD\t" << reg[rd].name << ", " << reg[rs2].name << "\n";
					// setting rs2
					instWord = rs2 << 20;
					// setting rs1
					instWord |= rs1 << 15;
					// setting rd
					instWord |= rd << 7;
					// setting base (func7 - func3 - op)
					instWord |= 0x00000033;	
				}
			}
				break;
		}		
	}
	else 
	{
		cout << "\tUnsupported Compressed Function\n";
	}
	instDecExec(instWord, 1);
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
				instDecExec(instWord, 0);
			}	
			else
			    // Decompress(instWord);
				cout << "Compressed Instructions are not supported\n";
		}
		
		printRegisterContents();
	} else emitError("Cannot access input file\n");
}

