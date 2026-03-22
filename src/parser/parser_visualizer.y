%{
#include <stdio.h>
#include <stdlib.h>
int yylex();
void yyerror(const char *s);
extern int yylineno;
%}

%union{
        char *str;
}

%token <str> ID INT_NUM FLOAT_NUM STRING CHAR_LIT
%token INT FLOAT CHAR DOUBLE 
%token RETURN IF ELSE WHILE
%token AND OR EQ LEQ GEQ NEQ
%token INC DEC
%token ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
%token IO

%type <str> type

%left OR
%left AND
%left '|'
%left '^'
%left '&'
%left EQ NEQ
%left '<' '>' LEQ GEQ
%left '+' '-'
%left '*' '/' '%'
%right '!' '~'
%right INC DEC

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

program:
        statement_list
        ;

block:
        '{' statement_list '}'
        ;

statement_list: 
        statement_list statement
        | statement
        ;

statement: 
        declaration
        | return_stmt
        | if_stmt
        | while_stmt
        | assignment
        | io_stmt
        | expr ';'
        ;

io_stmt:
        IO '(' expr ')' ';'
        { printf("IO Statement with 1 argument (line %d)\n", yylineno); }          
        | IO '(' expr ',' expr ')' ';'
        { printf("IO Statement with 2 arguments (line %d)\n", yylineno); }        
        ;

declaration:
        type ID ';'
        { printf("Declaration: %s %s\n", $1, $2); }
        | type ID '=' expr ';'
        { printf("Declaration with init: %s %s (line %d)\n", $1, $2, yylineno); }
        ;
        

expr:
        expr '+' expr
        | expr '-' expr
        | expr '*' expr
        | expr '/' expr
        | expr '%' expr
        | '-' expr %prec '-'
        | expr EQ expr
        | expr NEQ expr
        | expr LEQ expr
        | expr GEQ expr
        | expr '<' expr
        | expr '>' expr
        | expr AND expr
        | expr OR expr
        | '!' expr
        | '~' expr
        | expr '^' expr
        | expr '&' expr
        | expr '|' expr
        | '&' expr
        | '(' expr ')'
        | ID            { printf("Identifier: %s\n", $1); }
        | INT_NUM       { printf("Integer: %s\n", $1); }
        | FLOAT_NUM     { printf("Float: %s\n", $1); }
        | INC expr
        | DEC expr
        | expr INC
        | expr DEC
        | STRING        { printf("String: %s\n", $1); }
        | CHAR_LIT      { printf("Char literal: %s\n", $1); }
        ;

return_stmt:
        RETURN expr ';'
        { printf("Return Statement: (line %d)\n", yylineno); }
        ;

if_stmt:
        IF '(' expr ')' block %prec LOWER_THAN_ELSE
           { printf("If Statement (line %d)\n", yylineno); }
        | IF '(' expr ')' block ELSE block
           { printf("If-Else Statement (line %d)\n", yylineno); }
        ;

while_stmt:
        WHILE '(' expr ')' block
        { printf("While Statement (line %d)\n", yylineno); }
        ;

assignment:
        ID '=' expr ';'         { printf("Assignment: %s (line %d)\n", $1, yylineno); }           
        | ID ADD_ASSIGN expr ';'{ printf("Compound Assignment: %s += (line %d)\n", $1, yylineno); } 
        | ID SUB_ASSIGN expr ';'{ printf("Compound Assignment: %s -= (line %d)\n", $1, yylineno); } 
        | ID MUL_ASSIGN expr ';'{ printf("Compound Assignment: %s *= (line %d)\n", $1, yylineno); } 
        | ID DIV_ASSIGN expr ';'{ printf("Compound Assignment: %s /= (line %d)\n", $1, yylineno); } 
        | ID MOD_ASSIGN expr ';'{ printf("Compound Assignment: %s %%= (line %d)\n", $1, yylineno); } 
        ;

type:
        INT     { $$ = "int"; }
        | FLOAT { $$ = "float"; }
        | CHAR  { $$ = "char"; }
        | DOUBLE{ $$ = "double"; }
        ;

%%

void yyerror(const char *s)
{
    printf("Syntax error at line %d: %s\n", yylineno, s);
}
