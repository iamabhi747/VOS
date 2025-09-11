#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <iostream>

enum class Opcode
{ 
    GD, PD, H, LR, SR, CR, BT, JT, AD, SB, ML, DV, IM, DM, BC, JC, AN, OR, NT, LS, RS 
};

enum class DataType    { DW, DD, DQ, DL };
enum class BssType     { RESW, RESD, RESQ, RESL };
enum class OperandType { NONE, NUMBER, LABEL, REFERENCE };

struct DataEntry
{
    std::string label;
    DataType type;
    std::string value; 
};

struct BssEntry
{
    std::string label;
    BssType type;
};

struct Instruction
{
    Opcode opcode;

    OperandType opType;
    std::string operand;
};

struct Block
{
    std::string labelName;
    std::vector<Instruction> instructions;
};

class Program
{
public:
    std::vector<DataEntry> dataSection;
    std::vector<BssEntry>  bssSection;
    std::vector<Block>     textSection;

    void addData(const std::string& label, DataType type, const std::string& val);
    void addBss(const std::string& label, BssType type);
    void addLabel(const std::string& label);
    void addInstruction(Opcode op);
    void addInstruction(Opcode op, OperandType type, const std::string& operand);
};

extern Program astProgram;

#endif