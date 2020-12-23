#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <iomanip>
#include <queue>
#include "simulator.h"

MIPS32_Simulator::MIPS32_Simulator(std::vector<std::string> instructions, std::vector<int> mainMemory, std::unordered_map<std::string, int> dataLabels, std::unordered_map<std::string, int> textLabels, bool debugMode = false)
{
    this->instructions = instructions;
    this->mainMemory = mainMemory;
    this->dataLabels = dataLabels;
    this->textLabels = textLabels;
    this->debugMode = debugMode;

    pc = -1;
    cycleCount = 0;
}

void MIPS32_Simulator::executeInstructions()
{
    std::queue<int> pipeline;

    do
    {
        if(pc < (int)instructions.size() - 1)
        {
            pipeline.push(FETCH); //put next instruction into queue
        }

        int pipelineSize = pipeline.size();
        for(int i = 0; i < pipelineSize; i++) //for each instruction in the queue:
        {
            int current = pipeline.front(); //get top instruction in queue
            pipeline.pop();

            switch(current) //execute top instruction's stage
            {
                case FETCH:
                    fetch();
                    break;
                case DECODE:
                    decode();
                    break;
                case EXECUTE:
                    execute();
                    break;
                case MEMORYACCESS:
                    memoryAccess();
                    break;
                case WRITEBACK:
                    writeBack();
                    break;
            }

            if(current != WRITEBACK)
            {
                //if instruction isn't finished, advance its stage and push to the back of the queue
                current++;
                pipeline.push(current);
            }
        }

        if(debugMode)
        {
            std::cout << "-----CYCLE " << cycleCount << "-----" << std::endl;
            printPipelineRegisterContents();
            printRegisterContents();
            printMemoryContents();
        }

        cycleCount++;

    } while (!pipeline.empty());
}

template <typename T>
void MIPS32_Simulator::printArrayContents(T& array, int arraySize)
{
    for(int i = 0; i < arraySize; i += 4)
    {
        for(int t = i; t < i + 4 && t < arraySize; t++)
        {
            std::string stringT, stringArray;
            stringT = std::to_string(t);
            stringArray = std::to_string(array[t]);

            std::string s("[" + stringT + "]: " + stringArray);
            std::cout << std::left << std::setw(20) << s;
        }
        std::cout << std::endl;
    }
}

void MIPS32_Simulator::printRegisterContents()
{
    int registerFileSize = sizeof(registerFile) / sizeof(int);

    std::cout << "-------------------------Register File-------------------------" << std::endl;
    printArrayContents(registerFile, registerFileSize);
}

void MIPS32_Simulator::printMemoryContents()
{
    std::cout << "-------------------------Main Memory-------------------------" << std::endl;
    printArrayContents(mainMemory, mainMemory.size());
}

void MIPS32_Simulator::printPipelineRegisterContents()
{
    int id_ex_size = sizeof(id_ex) / sizeof(int);
    int ex_mem_size = sizeof(ex_mem) / sizeof(int);
    int mem_wb_size = sizeof(mem_wb) / sizeof(int);

    std::cout << "-------------------------IF/ID Registers-------------------------" << std::endl;
    //special case since IF/ID register is a string
    std::cout << "[0]: " << if_id[0] << std::endl;
    std::cout << "-------------------------ID/EX Registers-------------------------" << std::endl;
    printArrayContents(id_ex, id_ex_size);
    std::cout << "-------------------------EX/MEM Registers-------------------------" << std::endl;
    printArrayContents(ex_mem, ex_mem_size);
    std::cout << "-------------------------MEM/WB Registers-------------------------" << std::endl;
    printArrayContents(mem_wb, mem_wb_size);
}

int MIPS32_Simulator::getRegisterIndex(std::string name) const
{
    return REGISTER_NAMES.at(name);
}

void MIPS32_Simulator::fetch()
{
    //Instruction Fetch
    
    if(ex_mem[0] == 0) //PcSrc = 0
    {
        pc++;
    }
    else //PcSrc = 1
    {
        pc = ex_mem[1];
    }

    if_id[0] = instructions[pc];
}

void MIPS32_Simulator::decode()
{
    //Instruction Decode

    //parse instruction
    std::istringstream stringStream(if_id[0]);
    std::string str;

    //obtain operation
    stringStream >> str;
    
    //call instruction-specific decode function
    (this->*getInstructionFunc(str))(stringStream);
}

void MIPS32_Simulator::execute()
{
    //Execute

    //ALU
    int operand1 = id_ex[0]; //operand1 = ReadData1
    int operand2 = (id_ex[5] == 0) ? id_ex[1] : id_ex[11]; //operand2 = ReadData2 or Offset depending on ALUSrc
    int result;

    switch(id_ex[12])
    {
        case ADD:
            result = operand1 + operand2;
            break;
        case SUB:
            result = operand1 - operand2;
            //branch
            if(result != 0 && id_ex[3] == 1)
            {
                //beq resulted negatively, stop branching
                id_ex[3] = 0;
            }
            break;
        case MULT:
            result = operand1 * operand2;
            break;
        case SLL:
            result = operand1 << operand2;
            break;
        case SRL:
            result = operand1 >> operand2;
            break;
        case AND:
            result = operand1 & operand2;
            break;
        case OR:
            result = operand1 | operand2;
            break;
    }

    ex_mem[0] = id_ex[3]; //PcSrc = PcSrc
    ex_mem[1] = id_ex[11]; //Branch Addr = Offset
    ex_mem[2] = result; //Mem Addr = result
    ex_mem[3] = id_ex[1]; //Write Data = ReadData2
    ex_mem[4] = id_ex[7]; //MemRead = MemRead
    ex_mem[5] = id_ex[6]; //MemWrite = MemWrite
    ex_mem[6] = id_ex[4]; //MemToReg = MemToReg
    ex_mem[7] = id_ex[2]; //RegWrite = RegWrite
    ex_mem[8] = (id_ex[8] == 0) ? id_ex[9] : id_ex[10]; //Write Addr = WriteAddr1 or WriteAddr2 depending on RegDst
}

void MIPS32_Simulator::memoryAccess()
{
    //Memory Access

    if(ex_mem[5] == 1) //MemWrite = 1
    {
        mainMemory[ex_mem[2]] = ex_mem[3]; //memory at Mem Addr = Write Data
    }

    if(ex_mem[4] == 1) //MemRead = 1
    {
        mem_wb[0] = mainMemory[ex_mem[2]]; //Memory Data = memory at Mem Addr
    }

    mem_wb[1] = ex_mem[2]; //ALU data = Mem Addr
    mem_wb[2] = ex_mem[8]; //Write Addr = Write Addr
    mem_wb[3] = ex_mem[6]; //MemToReg = MemToReg
    mem_wb[4] = ex_mem[7]; //RegWrite = RegWrite
}

void MIPS32_Simulator::writeBack()
{
    //Write Back

    if(mem_wb[4] == 1) //RegWrite == 1
    {
        if(mem_wb[3] == 0) //MemToReg = 0
        {
            registerFile[mem_wb[2]] = mem_wb[0]; //register at Write Addr = Memory data
        }
        else //MemToReg == 1
        {
            registerFile[mem_wb[2]] = mem_wb[1]; //register at Write Addr = ALU data
        }
    }
}