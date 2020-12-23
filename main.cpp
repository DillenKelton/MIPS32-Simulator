#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include "simulator.h"

bool readInputFile(std::vector<std::string>& contents, std::string fileName);
void processDataSeg(std::vector<std::string>& instructions, std::vector<int>& memory, std::unordered_map<std::string, int>& dataLabels);
void processTextSeg(std::vector<std::string>& instructions, std::unordered_map<std::string, int>& textLabels);

int main(int argc, char** argv)
{
    std::vector<std::string> fileContents;
    std::unordered_map<std::string, int> dataLabels;
    std::unordered_map<std::string, int> textLabels;
    std::vector<int> mainMemory;
    bool debugMode = false;

    if(argc > 1)
    {
        if(argc > 2)
        {
            if(std::string(argv[1]) == "-d")
            {
                std::cout << "Debug Mode enabled" << std::endl;
                debugMode = true;
                argv[1] = argv[2];
            }
            else
            {
                std::cout << "commands not recognized, please check the readme" << std::endl;
                return 0;
            }
        }
        if(readInputFile(fileContents, argv[1]))
        {
            processDataSeg(fileContents, mainMemory, dataLabels);
            processTextSeg(fileContents, textLabels);

            MIPS32_Simulator simulator(fileContents, mainMemory, dataLabels, textLabels, debugMode);

            simulator.executeInstructions();
            simulator.printRegisterContents();
            simulator.printMemoryContents();
        }
    }

    return 0;
}

bool readInputFile(std::vector<std::string>& contents, std::string fileName)
{
    std::ifstream inFile(fileName);

    if(inFile.is_open())
    {
        std::string s;
        while(std::getline(inFile, s))
        {
            contents.push_back(s);
        }
    }
    else
    {
        std::cout << "Input file \"" << fileName << "\" could not be opened" << std::endl;
        return false;
    }

    inFile.close();
    return true;
}

void processDataSeg(std::vector<std::string>& instructions, std::vector<int>& memory, std::unordered_map<std::string, int>& dataLabels)
{
    //Read data segment from instructions, allocate memory as needed and add label/index mapping to dataLabels
    //Removes data segment from instructions when finished

    std::vector<std::string> dataSeg;
    bool inDataSeg = false;
    std::vector<std::string>::iterator i;

    for(i = instructions.begin(); i != instructions.end(); i++)
    {
        if(inDataSeg)
        {
            if(*i == ".text")
            {
                break; //data segment finished
            }
            if(i->length() > 0)
            {
                dataSeg.push_back(*i);
            }
        }
        else
        {
            if(*i == ".data")
            {
                inDataSeg = true; //data segment started
            }
        }
    }

    instructions.erase(instructions.begin(), i + 1); //only keep text segment here

    for(std::string line : dataSeg)
    {
        //assuming only words in data segment for now
        std::istringstream stringStream(line);
        std::string tag, instr, value;
        stringStream >> tag >> instr >> value;

        tag = tag.substr(0, tag.length() - 1); //remove colon
        int init_val = std::stoi(value);

        memory.push_back(init_val); //allocate word of memory with initial value
        dataLabels.emplace(tag, memory.size() - 1); //add label and index mapping to dataLabels
    }
}

void processTextSeg(std::vector<std::string>& instructions, std::unordered_map<std::string, int>& textLabels)
{
    //Look for labels in instructions and map them with line numbers in textLabels
    //Cleans up text segment for easier processing (remove labels, blank lines, etc)

    std::vector<std::string> newInstructions;

    //first pass to remove empty lines
    for(std::string s : instructions)
    {
        if(s.length() > 0 && s[0] != '#') //not blank line or comments
        {
            newInstructions.push_back(s);
        }
    }

    //second pass to handle and remove labels
    for(int i = 0; i < newInstructions.size(); i++)
    {
        int labelEnd = newInstructions[i].find(':');

        if(labelEnd != -1) //line has a label
        {
            std::string label;
            label = newInstructions[i].substr(0, labelEnd);

            textLabels.emplace(label, i); //save index of label

            newInstructions[i] = newInstructions[i].substr(labelEnd + 1); //remove label from instruction
        }
    }

    instructions.clear();
    for(auto i = newInstructions.begin(); i != newInstructions.end(); i++)
    {
        instructions.push_back(*i);
    }
}