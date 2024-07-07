# RISC-V-ISS

## Overview
This project was created for the Computer Organization & Assembly Language course, taught at the American University in Cairo in the Summer 2024 semester, under the instruction of Dr. Mohamed Shalan.  
  
This project implements a RISC-V ISA simulator and disassembler, challenging our understanding of how a CPU handles executing instructions and memory access, as well as the RISC-V ISA instructions and formats.


## How to build 
Building the program can be done on the command line using the following commands:
- `cd <cloned_program_directory>`
- `g++ rvsim.cpp -o rvsim.exe`

## How to use 
After building the program to be able to run the program you need to provide the binary file for the text section of the program you wish to disassemble 
and if your program has a data section , provide the binary file for it as well. 
That can be done using the following commands:
- for windows use `.\rvsim.exe <program_machine_code_filename> <data_in_binary_filename>[optional]`
- for MacOS use `./rvsim.exe <program_machine_code_filename> <data_in_binary_filename>[optional]`


## Program Design
This program is an ISS (ISA simulator) and disassembler for the RISC-V ISA, in particular **RV32IC** Base Integer Instruction Set with support for instruction compression.  
The program works by taking in an RV32IC machine code file and executes its instructions through the following steps:  
- Reading the machine code file and writing it to the text section of the memory, and if a data file exits it is also read into the data section.
- Going through the instructions one by one by a program counter, detecting whether they are compressed (16-bits) or not (32-bits).
- If the instructions are not compressed, they are immediately executed, otherwise they have to be decompressed first before becoming executable.
- The execution works by first determining the instruction through the opcode and other optional functions, then performing the indicated operations.
- Finally, the machine code is disassembled and outputted to the terminal screen, before the program moves onto the next instruction.


## Limitations in the program
- The ECALL instruction only supports the following commands:
   - printing an integer
   - printing a string
   - exiting the program
- The compressed functions were not thoroughly tested, as it was difficult to find a source to generate compressed instructions machine code from (however they passed the tests that were available).

 

## Challenges 
- extracting the immediates from the uncompressed instructions for most of the types proved to be a difficult task. It took us a while to test the way we extracted the immediates as we had to do it on paper and then test it on the program using the test files we had and had to check if correct immediates were used in the program. This caused issues such as an infinite loop caused by extracting incorrect immediate for the jal instruction.
- Extracting the immediate for the compressed instruction was even more difficult to do. The compressed CJ type's immediate is stored in the memory in a way that is hard to extract and arrange in the same order as the immediate in the instruction word of the uncompressed J type.
- Debugging the code took us a great deal of time as we were unable to make use of the debugger in the IDE we used and worked mostly on the command line. Because of that we had to use excute test files for every instruction in the program. Then we had to compare each instruction produced by our program to the asm file of the binary file used to make sure that our program disassembled correctly.


## Test cases 


## Collaborators
* [Yasmina Mahdy](https://github.com/Yasmina-Mahdy)
* [Rana Taher](https://github.com/rana5679)
  
## License
Copyright 2024 Arwa Abdelkarim  
Copyright 2024 Rana Taher  
Copyright 2024 Yasmina Mahdy  

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
