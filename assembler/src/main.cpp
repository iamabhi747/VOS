#include <filesystem>
#include <fstream>
#include <iostream>

#include <ast.h>
#include <string>


extern FILE* yyin;
extern int yyparse();
extern Program astProgram;

int main()
{
    std::string inputFile = "sample.asm";

    FILE* iFile = fopen(inputFile.c_str(), "r");
    if (!iFile)
    {
        std::cerr << "[-] Could not open input file! (" << inputFile << ")" << std::endl;
        return 1;
    }
    yyin = iFile;

    if (yyparse() == 0)
    {
        std::cout << "Parsing completed successfully!" << std::endl;
        std::cout << "Data blocks: " << astProgram.dataSection.size() << std::endl;
        std::cout << "BSS blocks: " << astProgram.bssSection.size() << std::endl;
        std::cout << "Text blocks (Labels): " << astProgram.textSection.size() << std::endl;
    }
    return 0;
}