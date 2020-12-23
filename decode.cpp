#include <sstream>
#include <string>
#include <unordered_map>
#include "simulator.h"

MIPS32_Simulator::decodeFunction MIPS32_Simulator::getInstructionFunc(std::string identifier) const
{
    if(INSTRUCTION_NAMES.find(identifier) == INSTRUCTION_NAMES.end()) //identifier not in map
    {
        return INSTRUCTION_NAMES.at("nop"); //default to nop
    }

    return INSTRUCTION_NAMES.at(identifier);
}

void MIPS32_Simulator::applyStoreOpCodes()
{
    //Store instruction
    id_ex[2] = 0; //RegWrite
    id_ex[3] = 0; //PCSrc
    id_ex[4] = 0; //MemToReg
    id_ex[5] = 1; //ALUSrc
    id_ex[6] = 1; //MemWrite
    id_ex[7] = 0; //MemRead
    id_ex[8] = 1; //RegDst
}

void MIPS32_Simulator::applyLoadOpCodes()
{
    //Load instruction
    id_ex[2] = 1; //RegWrite
    id_ex[3] = 0; //PCSrc
    id_ex[4] = 0; //MemToReg
    id_ex[5] = 1; //ALUSrc
    id_ex[6] = 0; //MemWrite
    id_ex[7] = 1; //MemRead
    id_ex[8] = 0; //RegDst
}

void MIPS32_Simulator::applyALUOpCodes()
{
    //ALU instruction
    id_ex[2] = 1; //RegWrite
    id_ex[3] = 0; //PCSrc
    id_ex[4] = 1; //MemToReg
    id_ex[5] = 0; //ALUSrc
    id_ex[6] = 0; //MemWrite
    id_ex[7] = 0; //MemRead
    id_ex[8] = 1; //RegDst
}

void MIPS32_Simulator::applyBranchOpCodes()
{
    //Branch instruction
    id_ex[2] = 0; //RegWrite
    id_ex[3] = 1; //PCSrc
    id_ex[4] = 0; //MemToReg
    id_ex[5] = 0; //ALUSrc
    id_ex[6] = 0; //MemWrite
    id_ex[7] = 0; //MemRead
    id_ex[8] = 0; //RegDst
}

void MIPS32_Simulator::applyRTypeCodes(std::istringstream& stringStream)
{
    //R-type
    std::string dest, src, targ;
    stringStream >> dest >> src >> targ;
    dest = dest.substr(0, dest.length() - 1); //remove comma
    src = src.substr(0, src.length() - 1); //remove comma

    int rd, rs, rt;
    rd = getRegisterIndex(dest);
    rs = getRegisterIndex(src);
    if(targ[0] == '$') //register name
    {
        rt = getRegisterIndex(targ);
        id_ex[1] = registerFile[rt]; //ReadData2 = register file at rt
    }
    else
    {
        rt = std::stoi(targ); //sll or srl
        id_ex[1] = rt; //ReadData2 = rt
    }

    id_ex[0] = registerFile[rs]; //ReadData1 = register file at rs
    id_ex[9] = rt; //WriteAddr1 = rt
    id_ex[10] = rd; //WriteAddr2 = rd
    id_ex[11] = 0; //Offset = 0
}

/* Instruction-specific decode functions */

void MIPS32_Simulator::decode_sw(std::istringstream& stringStream)
{
    applyStoreOpCodes();

    std::string storeSource, storeOffset, storeTarget;
    stringStream >> storeTarget;
    storeTarget = storeTarget.substr(0, storeTarget.length() - 1); //remove comma
    stringStream >> storeOffset;

    //extract storeSource
    int startLoc, endLoc;
    startLoc = storeOffset.find('(');
    endLoc = storeOffset.find(')');
    storeSource = storeOffset.substr(startLoc + 1, endLoc - startLoc - 1); //get register source
    storeOffset = storeOffset.substr(0, startLoc); //leave only the offset here

    id_ex[0] = registerFile[getRegisterIndex(storeSource)]; //ReadData1 = register contents at source (addr)
    id_ex[1] = registerFile[getRegisterIndex(storeTarget)]; //ReadData2 = register contents at target (data)
    id_ex[9] = 0; //WriteAddr1 = 0;
    id_ex[10] = 0; //WriteAddr2 = 0
    id_ex[11] = std::stoi(storeOffset); //Offset = storeOffset
    id_ex[12] = ADD;
}

void MIPS32_Simulator::decode_lw(std::istringstream& stringStream)
{
    applyLoadOpCodes();

    std::string loadSource, loadOffset, loadTarget;
    stringStream >> loadTarget;
    loadTarget = loadTarget.substr(0, loadTarget.length() - 1); //remove comma
    stringStream >> loadOffset;

    //extract loadSource
    int startLoc, endLoc;
    startLoc = loadOffset.find('(');
    endLoc = loadOffset.find(')');
    loadSource = loadOffset.substr(startLoc + 1, endLoc - startLoc - 1); //get register source
    loadOffset = loadOffset.substr(0, startLoc); //leave only the offset here

    id_ex[0] = registerFile[getRegisterIndex(loadSource)]; //ReadData1 = register contents at source (addr)
    id_ex[1] = 0; //ReadData2 = 0
    id_ex[9] = getRegisterIndex(loadTarget); //WriteAddr1 = loadTarget
    id_ex[10] = 0; //WriteAddr2 = 0
    id_ex[11] = std::stoi(loadOffset);
    id_ex[12] = ADD;
}

void MIPS32_Simulator::decode_add(std::istringstream& stringStream)
{
    applyALUOpCodes();
    applyRTypeCodes(stringStream);
    id_ex[12] = ADD;
}

void MIPS32_Simulator::decode_addi(std::istringstream& stringStream)
{
    applyALUOpCodes();
        
    std::string dest, src;
    int imm;

    stringStream >> dest >> src >> imm;
    dest = dest.substr(0, dest.length() - 1); //remove comma
    src = src.substr(0, src.length() - 1); //remove comma

    id_ex[0] = registerFile[getRegisterIndex(src)]; //ReadData1 = contents of register file at src
    id_ex[1] = imm; //ReadData2 = imm
    id_ex[9] = 0;
    id_ex[10] = getRegisterIndex(dest); //WriteAddr2 = index at dest
    id_ex[11] = 0;
    id_ex[12] = ADD;
}

void MIPS32_Simulator::decode_sub(std::istringstream& stringStream)
{
    applyALUOpCodes();
    applyRTypeCodes(stringStream);
    id_ex[12] = SUB;
}

void MIPS32_Simulator::decode_mult(std::istringstream& stringStream)
{
    applyALUOpCodes();
    applyRTypeCodes(stringStream);
    id_ex[12] = MULT;
}

void MIPS32_Simulator::decode_and(std::istringstream& stringStream)
{
    applyALUOpCodes();
    applyRTypeCodes(stringStream);
    id_ex[12] = AND;
}

void MIPS32_Simulator::decode_or(std::istringstream& stringStream)
{
    applyALUOpCodes();
    applyRTypeCodes(stringStream);
    id_ex[12] = OR;
}

void MIPS32_Simulator::decode_sll(std::istringstream& stringStream)
{
    applyALUOpCodes();
    applyRTypeCodes(stringStream);
    id_ex[12] = SLL;
}

void MIPS32_Simulator::decode_srl(std::istringstream& stringStream)
{
    applyALUOpCodes();
    applyRTypeCodes(stringStream);
    id_ex[12] = SRL;
}

void MIPS32_Simulator::decode_li(std::istringstream& stringStream)
{
    applyALUOpCodes();

    std::string li_src;
    int imm;
    stringStream >> li_src >> imm;
    li_src = li_src.substr(0, li_src.length() - 1); //remove comma

    id_ex[0] = imm; //ReadData1 = imm
    id_ex[1] = 0;
    id_ex[9] = 0;
    id_ex[10] = getRegisterIndex(li_src); //WriteAddr2 = register number at li_src
    id_ex[11] = 0;
    id_ex[12] = ADD;
}

void MIPS32_Simulator::decode_la(std::istringstream& stringStream)
{
    applyALUOpCodes();

    std::string la_dst, la_tag;
    int addr;
    stringStream >> la_dst >> la_tag;
    la_dst = la_dst.substr(0, la_dst.length() - 1); //remove comma
    addr = dataLabels[la_tag];

    id_ex[0] = addr; //ReadData1 = addr
    id_ex[1] = 0;
    id_ex[9] = 0;
    id_ex[10] = getRegisterIndex(la_dst); //WriteAddr2 = register number at la_dst
    id_ex[11] = 0;
    id_ex[12] = ADD;
}

void MIPS32_Simulator::decode_beq(std::istringstream& stringStream)
{
    applyBranchOpCodes();

    std::string src, targ, label;
    int branchIndex;
    stringStream >> src >> targ >> label;
    src = src.substr(0, src.length() - 1);
    targ = targ.substr(0, targ.length() - 1);
    branchIndex = textLabels[label];

    id_ex[0] = registerFile[getRegisterIndex(src)];
    id_ex[1] = registerFile[getRegisterIndex(targ)];
    id_ex[9] = 0;
    id_ex[10] = 0;
    id_ex[11] = branchIndex;
    id_ex[12] = SUB;
}

void MIPS32_Simulator::decode_j(std::istringstream& stringStream)
{
    applyBranchOpCodes();

    std::string label;
    int jumpIndex;
    stringStream >> label;
    jumpIndex = textLabels[label]; //jumpindex obtained from label mapping

    id_ex[0] = 0; //ReadData1 = 0
    id_ex[1] = 0; //ReadData2 = 0
    id_ex[9] = 0; //WriteAddr1 = 0
    id_ex[10] = 0; //WriteAddr2 = 0
    id_ex[11] = jumpIndex; //Offset = jumpIndex
    id_ex[12] = 0; //ALU OP = 0
} 

void MIPS32_Simulator::decode_nop(std::istringstream& stringStream)
{
    //NOP
    id_ex[0] = 0; //ReadData1
    id_ex[1] = 0; //ReadData2
    id_ex[2] = 0; //RegWrite
    id_ex[3] = 0; //PcSrc
    id_ex[4] = 0; //MemToReg
    id_ex[5] = 0; //ALUSrc
    id_ex[6] = 0; //MemWrite
    id_ex[7] = 0; //MemRead
    id_ex[8] = 0; //RegDst
    id_ex[9] = 0; //WriteAddr1
    id_ex[10] = 0; //WriteAddr2
    id_ex[11] = 0; //Offset
    id_ex[12] = 0; //ALU Operation
} 