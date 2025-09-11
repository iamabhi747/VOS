%{
#include "ast.h"
#include "parser.tab.hpp"
#include <string.h>
%}

%option yylineno
%option noyywrap
%option case-insensitive

ID      [a-zA-Z_][a-zA-Z0-9_]*
STRING  \"[^\"]*\"
NUMBER  -?[0-9]+

%%

[ \t\r\n]+          { /* Ignore whitespace */ }
";".*               { /* Ignore comments */ }

".data"             { return SEC_DATA; }
".bss"              { return SEC_BSS; }
".text"             { return SEC_TEXT; }

"dw"                { yylval.dataType = DataType::DW; return T_DATATYPE; }
"dd"                { yylval.dataType = DataType::DD; return T_DATATYPE; }
"dq"                { yylval.dataType = DataType::DQ; return T_DATATYPE; }
"dl"                { yylval.dataType = DataType::DL; return T_DATATYPE; }

"resw"              { yylval.bssType = BssType::RESW; return T_BSSTYPE; }
"resd"              { yylval.bssType = BssType::RESD; return T_BSSTYPE; }
"resq"              { yylval.bssType = BssType::RESQ; return T_BSSTYPE; }
"resl"              { yylval.bssType = BssType::RESL; return T_BSSTYPE; }

"GD"                { yylval.opcode = Opcode::GD; return T_OPCODE; }
"PD"                { yylval.opcode = Opcode::PD; return T_OPCODE; }
"H"                 { yylval.opcode = Opcode::H;  return T_OPCODE; }
"LR"                { yylval.opcode = Opcode::LR; return T_OPCODE; }
"SR"                { yylval.opcode = Opcode::SR; return T_OPCODE; }
"CR"                { yylval.opcode = Opcode::CR; return T_OPCODE; }
"BT"                { yylval.opcode = Opcode::BT; return T_OPCODE; }
"JT"                { yylval.opcode = Opcode::JT; return T_OPCODE; }
"AD" | "ADD"        { yylval.opcode = Opcode::AD; return T_OPCODE; }
"SB" | "SUB"        { yylval.opcode = Opcode::SB; return T_OPCODE; }
"ML" | "MUL"        { yylval.opcode = Opcode::ML; return T_OPCODE; }
"DV" | "DIV"        { yylval.opcode = Opcode::DV; return T_OPCODE; }
"IM" | "INC"        { yylval.opcode = Opcode::IM; return T_OPCODE; }
"DM" | "DEC"        { yylval.opcode = Opcode::DM; return T_OPCODE; }
"BC"                { yylval.opcode = Opcode::BC; return T_OPCODE; }
"JC"                { yylval.opcode = Opcode::JC; return T_OPCODE; }
"AN" | "AND"        { yylval.opcode = Opcode::AN; return T_OPCODE; }
"OR"                { yylval.opcode = Opcode::OR; return T_OPCODE; }
"NT" | "NOT"        { yylval.opcode = Opcode::NT; return T_OPCODE; }
"LS"                { yylval.opcode = Opcode::LS; return T_OPCODE; }
"RS"                { yylval.opcode = Opcode::RS; return T_OPCODE; }

":"                 { return T_COLON; }

"["                 { return '['; }
"]"                 { return ']'; }

{STRING}            { yylval.strval = strdup(yytext); return T_STRING; }
{NUMBER}            { yylval.strval = strdup(yytext); return T_NUMBER; }
{ID}                { yylval.strval = strdup(yytext); return T_ID; }

.                   { printf("Lexical Error: Unrecognized character %s\n", yytext); }

%%