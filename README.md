# RISC-V-ISS

## Overview


## How to build 
Building the program can be done on the command line using the following commands:
- `cd <cloned program directory>`
- `g++ rvsim.cpp -o rvsim.exe`

## How to use 
After building the program to be able to run the program you need to provide the binary file for the text section of the program you wish to disassemble 
and if your program has a data section , provide the binary file for it as well. 
That can be done using the foolowing command:
- for windows use `.\rvsim.exe <program_machine_code_filename> <data_in_binary_filename>[optional]`
- for MacOS use `./rvsim.exe <program_machine_code_filename> <data_in_binary_filename>[optional]`


## Program Design

## Limitations in the program
- The ECALL instruction only supports the following commands:
   - printing an integer
   - printing a string
   - exiting the program
 

## Challenges 



## Test cases 


## Collaborators
* [Yasmina Mahdy](https://github.com/Yasmina-Mahdy)
* [Rana Taher](https://github.com/rana5679)
  
## License
Copyright 2024 Yasmina Mahdy  
Copyright 2024 Arwa Abdelkarim  
Copyright 2024 Rana Taher  

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
