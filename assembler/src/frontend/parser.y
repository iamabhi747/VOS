%{
#include "ast.h"
#include <iostream>
#include <vector>

extern int yylex();
extern int yylineno;
extern char* yytext;

void yyerror(const char *s)
{
    std::cerr << "\n[-] Parse Error at line " << yylineno << ":\n";
    std::cerr << "   " << s;
    if (yytext && yytext[0] != '\0') {
        std::cerr << " (near token '" << yytext << "')";
    }
    std::cerr << "\n\n";
}


Program astProgram; 
%}

%code requires {
    #include "ast.h"
    #include <string>

    struct ParsedOperand {
        OperandType type;
        char* value;
    };
}

%union {
    char* strval;
    DataType dataType;
    BssType bssType;
    Opcode opcode;
    ParsedOperand parsedOp;
}

%token SEC_DATA SEC_BSS SEC_TEXT
%token T_COLON
%token <dataType> T_DATATYPE
%token <bssType> T_BSSTYPE
%token <opcode> T_OPCODE
%token <strval> T_ID T_STRING T_NUMBER

%type <strval> value
%type <parsedOp> operand

%%

program:
    sections
    ;

sections:
    section
    | sections section
    ;

section:
    data_section
    | bss_section
    | text_section
    ;

data_section:
    SEC_DATA data_stmts
    ;

data_stmts:
    /* empty */
    | data_stmts data_stmt
    ;

data_stmt:
    T_ID T_DATATYPE value
    {
        astProgram.addData($1, $2, $3);
        free($1); free($3);
    }
    ;

value:
    T_STRING   { $$ = $1; }
    | T_NUMBER { $$ = $1; }
    ;

bss_section:
    SEC_BSS bss_stmts
    ;

bss_stmts:
    /* empty */
    | bss_stmts bss_stmt
    ;

bss_stmt:
    T_ID T_BSSTYPE
    {
        astProgram.addBss($1, $2);
        free($1);
    }
    ;

text_section:
    SEC_TEXT text_stmts
    ;

text_stmts:
    /* empty */
    | text_stmts text_stmt
    ;

text_stmt:
    T_ID T_COLON
    {
        astProgram.addLabel($1);
        free($1);
    }
    | T_OPCODE operand
    {
        astProgram.addInstruction($1, $2.type, $2.value);
        free($2.value);
    }
    | T_OPCODE
    {
        astProgram.addInstruction($1);
    }
    ;

operand:
    T_ID
    { 
        $$.type = OperandType::LABEL; 
        $$.value = $1; 
    }
    | T_NUMBER
    { 
        $$.type = OperandType::NUMBER; 
        $$.value = $1; 
    }
    | '[' T_ID ']'
    { 
        $$.type = OperandType::REFERENCE; 
        $$.value = $2; 
    }
    ;

%%
