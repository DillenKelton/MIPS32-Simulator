#include <vector>
#include <unordered_map>

class MIPS32_Simulator
{
    public:

        MIPS32_Simulator(std::vector<std::string> instructions, std::vector<int> mainMemory, std::unordered_map<std::string, int> dataLabels, std::unordered_map<std::string, int> textLabels, bool debugMode);

        void executeInstructions();

        template <typename T>
        void printArrayContents(T& array, int arraySize);

        void printRegisterContents();

        void printMemoryContents();

        void printPipelineRegisterContents();

    private:

        typedef void (MIPS32_Simulator::*decodeFunction)(std::istringstream&);

        std::vector<std::string> instructions;
        std::vector<int> mainMemory;
        std::unordered_map<std::string, int> dataLabels;
        std::unordered_map<std::string, int> textLabels;
        bool debugMode;
        int pc; //Program counter
        int cycleCount;

        const std::unordered_map<std::string, int> REGISTER_NAMES
        {
            { "$zero", 0 }, { "$at", 1 }, { "$v0", 2 }, { "$v1", 3 },
            { "$a0", 4 }, { "$a1", 5 }, { "$a2", 6 }, { "$a3", 7 },
            { "$t0", 8 }, { "$t1", 9 }, { "$t2", 10 }, { "$t3", 11 },
            { "$t4", 12 }, { "$t5", 13 }, { "$t6", 14 }, { "$t7", 15 },
            { "$s0", 16 }, { "$s1", 17 }, { "$s2", 18 }, { "$s3", 19 },
            { "$s4", 20 }, { "$s5", 21 }, { "$s6", 22 }, { "$s7", 23 },
            { "$t8", 24 }, { "$t9", 25 }, { "$k0", 26 }, { "$k1", 27 },
            { "$gp", 28 }, { "$sp", 29 }, { "$fp", 30 }, { "$ra", 31 }
        };

        const std::unordered_map<std::string, decodeFunction> INSTRUCTION_NAMES
        {
            { "sw", &MIPS32_Simulator::decode_sw }, { "lw", &MIPS32_Simulator::decode_lw }, { "add", &MIPS32_Simulator::decode_add }, { "addi", &MIPS32_Simulator::decode_addi },
            { "sub", &MIPS32_Simulator::decode_sub }, { "mult", &MIPS32_Simulator::decode_mult }, { "and", &MIPS32_Simulator::decode_and }, { "or", &MIPS32_Simulator::decode_or },
            { "li", &MIPS32_Simulator::decode_li }, { "la", &MIPS32_Simulator::decode_la }, { "sll", &MIPS32_Simulator::decode_sll }, { "srl", &MIPS32_Simulator::decode_srl },
            { "beq", &MIPS32_Simulator::decode_beq }, { "j", &MIPS32_Simulator::decode_j }, { "nop", &MIPS32_Simulator::decode_nop }
        };

        int registerFile[32] = { };

        //Pipeline register files
        std::string if_id[1] = { };
        /*
           if_id[0] = Instruction
        */
        int id_ex[13] = { };
        /*
           id_ex[0] = ReadData1
           id_ex[1] = ReadData2
           id_ex[2] = RegWrite
           id_ex[3] = PcSrc
           id_ex[4] = MemToReg
           id_ex[5] = ALUSrc
           id_ex[6] = MemWrite
           id_ex[7] = MemRead
           id_ex[8] = RegDst
           id_ex[9] = WriteAddr1
           id_ex[10] = WriteAddr2
           id_ex[11] = Offset
           id_ex[12] = ALU Operation
        */
        int ex_mem[9] = { };
        /* 
           ex_mem[0] = PcSrc
           ex_mem[1] = Branch Addr
           ex_mem[2] = Mem Addr
           ex_mem[3] = Write Data
           ex_mem[4] = MemRead
           ex_mem[5] = MemWrite
           ex_mem[6] = MemToReg
           ex_mem[7] = RegWrite
           ex_mem[8] = Write Addr
         */
        int mem_wb[5] = { };
        /*
           mem_wb[0] = Memory Data
           mem_wb[1] = ALU Data
           mem_wb[2] = Write Addr
           mem_wb[3] = MemToReg
           mem_wb[4] = RegWrite
        */

        enum ALU_OP
        {
            ADD,
            SUB,
            MULT,
            SLL,
            SRL,
            AND,
            OR
        };

        enum PIPELINE_STAGE
        {
            FETCH,
            DECODE,
            EXECUTE,
            MEMORYACCESS,
            WRITEBACK
        };

        int getRegisterIndex(std::string name) const;

        decodeFunction getInstructionFunc(std::string identifier) const;

        void applyStoreOpCodes();

        void applyLoadOpCodes();

        void applyALUOpCodes();

        void applyBranchOpCodes();

        void applyRTypeCodes(std::istringstream& stringStream);

        void fetch();

        void decode();

        void execute();

        void memoryAccess();

        void writeBack();

        /* Instruction-specific decode functions */
        void decode_sw(std::istringstream& stringStream);

        void decode_lw(std::istringstream& stringStream);

        void decode_add(std::istringstream& stringStream);

        void decode_addi(std::istringstream& stringStream);

        void decode_sub(std::istringstream& stringStream);

        void decode_mult(std::istringstream& stringStream);

        void decode_and(std::istringstream& stringStream);

        void decode_or(std::istringstream& stringStream);

        void decode_sll(std::istringstream& stringStream);

        void decode_srl(std::istringstream& stringStream);

        void decode_li(std::istringstream& stringStream);

        void decode_la(std::istringstream& stringStream);

        void decode_beq(std::istringstream& stringStream);

        void decode_j(std::istringstream& stringStream);

        void decode_nop(std::istringstream& stringStream);
};
