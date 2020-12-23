# MIPS32-Simulator
 CS3339 Project (Spring 2020) - Basic simulation of a MIPS32 processor with assembly input  
 Compiled with g++ 8.1.0 (wingw-w64)
 
 Compile with the provided makefile
 
 Program input is one assembly/text file executed as such: "simulator.exe input.asm"
 
 Use debug mode for register file, memory, and pipeline register data at each cycle  
 Debug mode is activated with the -d flag: "simulator.exe -d input.asm"

 NOTE: Behavior of simulator is undefined when data/control hazards are present in input file. Use nop instruction to prevent hazards.
