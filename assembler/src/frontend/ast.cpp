#include <ast.h>

void Program::addData(const std::string& label, DataType type, const std::string& val)
{
    dataSection.push_back({label, type, val});
}

void Program::addBss(const std::string& label, BssType type)
{
    bssSection.push_back({label, type});
}

void Program::addLabel(const std::string& label)
{
    textSection.push_back({label, {}});
}

void Program::addInstruction(Opcode op)
{
    if (textSection.empty())
    {
        std::cerr << "Error: Instruction parsed before any label definition!" << std::endl;
        exit(1);
    }
    textSection.back().instructions.push_back({op, OperandType::NONE, ""});
}

void Program::addInstruction(Opcode op, OperandType type, const std::string& operand)
{
    if (textSection.empty())
    {
        std::cerr << "Error: Instruction parsed before any label definition!" << std::endl;
        exit(1);
    }
    textSection.back().instructions.push_back({op, type, operand});
}