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


registers reg[32]={{"zero",0},{"ra"},{"sp"},{"gp"},{"tp"},{"t0"},{"t1"},{"t2"},{"s0"},{"s1"},{"a0"},{"a1"},
				   {"a2"},{"a3"},{"a4"},{"a5"},{"a6"},{"a7"},{"s2"},{"s3"},{"s4"},{"s5"},{"s6"},{"s7"},{"s8"},
				   {"s9"},{"s10"},{"s11"},{"t3"},{"t4"},{"t5"},{"t6"}};

unsigned int pc = 0;
// 64KB for instructions and 64KB for data
unsigned char memory[(64+64)*1024]; 

void RegisterContents()
{
	for (int i = 0; i < 32; i++)
	{
		cout << "x" << dec << i << "\t" << reg[i].name << "\t0x" << hex << reg[i].value << "\n";
	}
}

void emitError(const char *s)
{ 
	cout << s;
	exit(0);
}


void printPrefix(unsigned int instA, unsigned int instW)
{
	cout << "0x" << hex << setfill('0') << setw(8) << instA << "\t0x" << setw(8) << instW;
}


// this is a function to load i+1 bytes to a reg, i is offset to reach most significant byte (little endian)
int mem_to_reg(int ad, int i) 
{ 
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

    if (!compressed)
        instPC = pc - 4;

    else
        instPC = pc - 2;

	if(!compressed) printPrefix(instPC, instWord);

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
	
	//Extraction of S immediate 
	S_imm = (((instWord >> 7) & 0x0000001F) |
			((instWord >> 20 & 0x00000FE0))) |
			((instWord >> 31) ? 0xFFFFF800 : 0x0);

	//Extraction of J immediate
	J_imm = ((instWord >> 31 & 0x1) << 20) |
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
				reg[rd].value = reg[rs1].value + (int)I_imm;
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
					cout << "\tLH\t" << reg[rd].name << ", " << dec << (int)I_imm << "(" << reg[rs1].name << ")\n";
				if (rd == 0) return; // to keep reg zero unchanged
				temp = mem_to_reg(ad, 1);
				// sign extending
				reg[rd].value = temp | ((temp >> 15) ? (0xFFFF0000) : 0x0); 
			}
				break;

			case 2:	
			{
				if (!compressed)
					cout << "\tLW\t" << reg[rd].name << ", " << dec << (int)I_imm << "(" << reg[rs1].name << ")\n";
				if (rd == 0) return; // to keep reg zero unchanged
				reg[rd].value = mem_to_reg(ad, 3);
			}
				break;

			case 4:	
			{
				if (!compressed)
					cout << "\tLBU\t" << reg[rd].name << ", " << dec << (int)I_imm << "(" << reg[rs1].name << ")\n";
				if (rd == 0) return; // to keep reg zero unchanged
				// automatically zero extended because memory is unsigned char
				reg[rd].value = memory[ad];
			}
				break;

			case 5:	
			{
				if (!compressed)
					cout << "\tLHU\t" << reg[rd].name << ", " << dec << (int)I_imm << "(" << reg[rs1].name << ")\n";
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
			cout << "\tLUI\t" << reg[rd].name << ", 0x" << hex << ((int)U_imm) << "\n";
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
		if (!compressed) cout << "\tJAL\t" << reg[rd].name << "," << hex << "0x" << instPC + (int)J_imm << "\n";
		if (rd != 0) reg[rd].value = pc; // to keep reg zero unchanged
		pc = instPC + J_imm;
		
	}
	else if (opcode == 0x67)
	{
		if (!compressed) cout << "\tJALR\t" << reg[rd].name << "," << reg[rs1].name << "," << hex << "0x" << (int)I_imm << "\n";
		if (rd != 0) reg[rd].value = pc;
		pc = reg[rs1].value + (int)I_imm;
		 // to keep reg zero unchanged
	}
  
	// Ecall instruction
	else if (opcode == 0x73)
	{
		if (reg[17].value == 1)  // a7 = 1
			cout << "\tECALL (INT)\n" << dec << reg[10].value << '\n';

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
			cout << "\tECALL (STR)\n" << output <<'\n';
		}
		else if (reg[17].value == 10)
		{
			cout << "\tECALL (Exit)\n";
			exit(0);
		}
		else
		{
			cout << "\tUnsupported Ecall Functionality\n";
		}
	}
	else
	{

		if (!compressed)cout << "\tUnkown Instruction \n";
	}
}

void Decompress(unsigned int instHalf)
{

	unsigned int opcode, rs2, rs1, rd, func4, rd_dash, rs1_dash, rs2_dash, func3, func2, shamt, func, func6, instWord = 0x0;
	unsigned int CJ_imm = 0x0, CI_imm = 0x0, CSS_imm = 0x0, CIW_imm = 0x0, CL_imm = 0x0, CS_imm = 0x0, CB_imm = 0x0, B_imm = 0x0, CJ_imm_D = 0x0;

	unsigned int instPC = pc - 2;

	printPrefix(instPC, instHalf);

	opcode = instHalf & 0x00000003;
	rs2 = (instHalf >> 2) & 0x0000001F;
	rd = (instHalf >> 7) & 0x0000001F;
	rs1 = (instHalf >> 7) & 0x0000001F;
	func4 = (instHalf >> 12) & 0x0000000F;
	func3 = (instHalf >> 13) & 0x00000007;
	func2 = (instHalf >> 10) & 0x03;
	func = (instHalf>> 5) & 0x00000003;
	func6 = (instHalf>> 10) & 0x003F;
	rd_dash = (instHalf >> 2) & 0x00000007;
	rs2_dash = (instHalf >> 2) & 0x00000007;
	rs1_dash = (instHalf >> 7) & 0x00000007;
	rd_dash |= 0x08;
	rs2_dash |= 0x08;
	rs1_dash |= 0x08;

	// CJ offset
	CJ_imm = ((instHalf >> 2) & 0x1);
	CJ_imm <<= 4; 
	CJ_imm |= ((instHalf >> 3) & 0x007);  
	CJ_imm |= (instHalf & 0x040);  // 7th bit
	CJ_imm |= ((instHalf >> 2) & 0x020); // 6th bit
	CJ_imm |= ((instHalf >> 1) & 0x200); // 10th bit
	CJ_imm |= ((instHalf >> 2) & 0x180); // 8th & 9th bits
	CJ_imm |= ((instHalf >> 8) & 0x008); // 4th
	CJ_imm |= ((instHalf >> 2) & 0x400);
	CJ_imm |= ((instHalf >> 12 & 0x01) ? 0xFFFFF800 : 0X0);
	// Rearranging immediate to J-Format 
	CJ_imm_D = ((CJ_imm & 0x3FF) << 9);
	CJ_imm_D |= (CJ_imm  >> 11) & 0x0FF; 
	CJ_imm_D |= ((CJ_imm >> 2) & 0x100); 
	CJ_imm_D |= CJ_imm & 0x80000;

	// CB offset
	CB_imm = ((instHalf << 3) & 0x020) | ((instHalf >> 2) & 0x06) | ((instHalf << 1) & 0x0c0) | ((instHalf >> 7) & 0x018);
	CB_imm |= ((instHalf) & 0x01000) ? 0xFFFFFF00 : 0x0;

	// Shift amount, shamt[5] is 0 for rv32C
	shamt = ((instHalf >> 2) & (0x01F));
        
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
      

	if (opcode == 0x0)
	{
		switch (func3)
		{
			case 0: // c.addi4spn
			{
				// setting the base (opcode -- fn3 -- x2)
				instWord = 0x00010413;
				// adding rd'
				instWord |= ((instHalf << 5) & 0x0380);
				// extract the immediate and make it start at position 0 
				// non-zero unsigned immediate scaled by 4
				CI_imm = 0x0;
				CI_imm |= ((instHalf >> 2) & 0x08) |
					((instHalf >> 4) & 0x04) |
					((instHalf >> 1) & 0x03C0) |
					((instHalf >> 7) & 0x030);
				// adding the immediate
				instWord |= ((CI_imm << 20) & 0xFFF00000);
				cout << "\tC.ADDI4SPN " << reg[rd_dash].name << hex << ", 0x" << (int)CI_imm << "\n";
			}
				break;
        
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
			default: cout << "\tUnknown C0 Instruction\n";
        
		}

	}
	else if (opcode == 0x1)
	{
        
		switch (func3)
		{
			// c.nop
			case 0:
			{
				if(rs1 == 0) // c.nop
				{	
					cout << "\tC.NOP\t" << "\n";
					// set 32-bit instruction
					instWord = 0x00000013;
				}
				// c.addi
				else
				{
					CI_imm = (instHalf >> 2) & 0x001F;
					CI_imm = CI_imm |(((instHalf >> 12) & 0x0001) << 5);
					CI_imm |= (CI_imm & 0x0020) ? 0xFFFFFFC0 : 0x0;
					cout<<"\tC.ADDI\t"<<reg[rs1].name<<"," << dec << (int)CI_imm << endl;

					CI_imm = CI_imm << 20;

					instWord = 0x13;
					instWord = instWord | (rs1 << 7);
					instWord = instWord | (0x0 <<12);
					instWord = instWord | (rs1 <<15);
					instWord = instWord | CI_imm;					
				}
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
				cout << "\tC.LI\t" << reg[rd].name << ", " << dec  << (int)CI_imm << "\n";
				// setting rd
				instWord = rd << 7;
				// setting immediate
				instWord |= CI_imm << 20;
				// setting base (rs1 - func3 - op)
				instWord |= 0x00000013;
			}
				break;
        
			case 3:
			{
				if (rd == 2) //addi16sp

				{
					// adding the base (opcode -- rd -- funct3 -- rs1 )
					instWord = 0x00010113;
					// extracting the immediate
						CI_imm = 0x0;
					CI_imm |= ((instHalf << 3) & 0x020) |
						((instHalf << 4) & 0x0180) |
						((instHalf << 1) & 0x020) |
						((instHalf >> 2) & 0x010) |
						((instHalf & 0x01000) ? 0xFFFFFE00 : 0x0);
					// adding the immediate
					instWord |= ((CI_imm << 20) & 0xFFF00000);
					cout << "\tC.ADDI16SP " << hex << "0x" << int(CI_imm) << '\n';
			     }
				// c.lui
				else if(rd != 0)
				{
					// CI immediate
					CI_imm = (instHalf >> 2) & 0x001F;
					CI_imm = CI_imm |(((instHalf >> 12) & 0x0001) << 5);
					cout<<"\tC.LUI\t"<<reg[rs1].name<< ", 0x"<< CI_imm << endl;
					CI_imm = CI_imm << 12;
					CI_imm |= (CI_imm & 0x020000) ? 0xFFFFFFC0000 : 0x0;
					// opcode
					instWord = 0x37;
					// rd
					instWord = instWord | (rd << 7);
					// imm[31:12]
					instWord |= (CI_imm);
				}
			}
				break;

			case 4:
			{
				if(func6 == 0x23)
				{
					switch (func)
				    {
						// c.sub
						case 0:
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
							break;	

						// c.xor
						case 1:
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
							break;

						// c.or
						case 2:
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
							break;

						// c.and
						case 3:
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
							break;	
				    }
			    }
				else 
				{
					if (func2 == 0) // c.srli
					{
						// setting the base (opcode -- fn3 -- fn2)
						instWord = 0x00045413;
						// adding rd'
						instWord |= instHalf & 0x0380;
						// adding rs1'
						instWord |= ((instHalf << 8) & 0x038000);
						// adding shamt
						instWord |= ((shamt << 20) & 0x01F00000);

						cout << "\tC.SRLI\t" << reg[rs1_dash].name << hex << ", 0x" << int(shamt) << "\n";
					}
					else if (func2 == 1) //c.srai
					{
						// setting the base (opcode -- fn3 -- fn2)
						instWord = 0x40045413;
						// adding rd'
						instWord |= instHalf & 0x0380;
						// adding rs1'
						instWord |= ((instHalf << 8) & 0x038000);
						// adding shamt
						instWord |= (shamt << 20) & 0x01F00000;
						cout << "\tC.SRAI\t" << reg[rs1_dash].name << hex << ", 0x" << int(shamt) << "\n";
					}
					else // c.andi
					{
						// setting the base (opcode -- fn3 -- fn2)
						instWord = 0x00087813;
						// adding rd'
						instWord |= instHalf & 0x380;
						// adding rs1'
						instWord |= ((instHalf << 8) & 0x038000);
						// special immediate for andi
						B_imm = shamt | (((instHalf >> 7) & 0x020) ? 0xFFFFFFE0 : 0x0);
						// adding the immediate
						instWord |= (B_imm << 20) & 0xFFF00000;
						cout << "\tC.ANDI\t" << reg[rs1_dash].name << hex << ", 0x" << int(B_imm) << "\n";
					}
				}
				
		    }
				break;
        
			case 5:
			{	CJ_imm <<= 1;
				cout << "\tC.J\t" << "0x" << hex << instPC + (int)CJ_imm << "\n";
				instWord = CJ_imm_D << 12;
				instWord |= 0x000000EF;
			}
				break;
			case 6: // c.beqz
			{
				// setting the base (opcode -- rs2 -- funct3)
				instWord = 0x00040063;
				// adding rs1'
				instWord |= ((instHalf << 8) & 0x038000);
				// adding the immediate
				instWord |= ((CB_imm >> 4) & 0x080) |
					((CB_imm << 7) & 0x0F00) |
					((CB_imm << 20) & 0x7e000000) |
					((CB_imm << 19) & 0x80000000);

				cout << "\tC.BEQZ\t" << reg[rs1_dash].name << hex << ", 0x" << int(instPC + CB_imm) << "\n";
			}
				break;
			case 7: // c.bnez
			{
				// setting the base (opcode -- rs2 -- funct3)
				instWord = 0x00041063;
				// adding rs1'
				instWord |= ((instHalf << 8) & 0x038000);
				// adding the immediate
				instWord |= ((CB_imm >> 4) & 0x080) |
					((CB_imm << 7) & 0x0F00) |
					((CB_imm << 20) & 0x7e000000) |
					((CB_imm << 19) & 0x80000000);
				cout << "\tC.BNEZ\t" << reg[rs1_dash].name << hex << ", 0x" << int(instPC + CB_imm) << "\n";
			}
				break;
		
			default: cout << "\tunknown C1 instruction\n";
		}

	}
	else if (opcode == 0x2)
	{
		switch (func3)
		{

			case 0: // c.slli
			{
				// setting the base (opcode -- fn3 -- fn2)
				instWord = 0x00001013;
				// adding rd
				instWord |= instHalf & 0xF80;
				// adding rs1
				instWord |= ((instHalf << 8) & 0x0F8000);
				// adding shamt
				instWord |= (shamt << 20) & 0x01F00000;

				cout << "\tC.SLLI\t" << reg[rs1].name << hex << ", 0x" << int(shamt) << "\n";
			}
				break;
			case 2: // c.lwsp
			{
				// adding rd
				instWord = (rd << 7);
				// setting the base (opcode -- rd -- fn3)
				instWord |= 0x00012003;
			 	// extracting the immediate
				CI_imm = 0x0;
				CI_imm |= ((instHalf << 4) & 0x0C0) |
					     ((instHalf >> 2) & 0x01C) |
						 ((instHalf >> 7) & 0x020);
				// adding the immediate
				instWord |= ((CI_imm << 20) & 0xFFF00000);
				cout << "\tC.LWSP\t" << reg[rd].name << ", " << dec << (int)CI_imm << "\n"; 
			}
				break;
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

			// c.swsp
			case 6:
			{
				// opcode
				instWord = 0x23;
				// imm [4:0]
				instWord = instWord | ((CSS_imm & 0x01F) << 7);
				// function 3
				instWord = instWord | (0x2 << 12);
				// stack pointer
				instWord = instWord | (0x02 << 15);
				// rs2
				instWord = instWord | (rs2 << 20);
				// imm[11:5]
				instWord = instWord | ((CSS_imm >> 5) & 0x07F);
				cout<<"\tC.SWSP\t"<<reg[rs2].name<< ", " << dec << CSS_imm << endl;
			}
				break;
			
			default: cout << "\tunknown C2 instruction\n"; 
	    }
    }	
	else cout << "\tUnsupported Compressed Function\n";
	// after decompressing the instruction halfword we will call the other function to execute the instruction

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
		emitError("use: .\rvsim.exe <machine_code_file_name>\n");

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
			    Decompress(instWord);
		}

	} else emitError("Cannot access input file\n");
}

