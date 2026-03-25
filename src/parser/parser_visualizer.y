%{
#include <stdio.h>
#include <stdlib.h>
int yylex();
void yyerror(const char *s);
extern int yylineno;
int construct=0;
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
%right INC DEC UMINUS

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
        { printf("%-20d %-40s %-20d\n", ++construct, "IO Statement (1 arg)", yylineno); }   
        | IO '(' expr ',' expr ')' ';'
       { printf("%-20d %-40s %-20d\n", ++construct, "IO Statement (2 arg)", yylineno); }       
        ;

declaration:
        type ID ';'
      { printf("%-20d %-40s %s %s (line %d)\n", ++construct, "Declaration", $1, $2, yylineno); }
        | type ID '=' expr ';'
      { printf("%-20d %-40s %s %s (line %d)\n", ++construct, "Declaration with Init", $1, $2, yylineno); }
        ;
        

expr:
        expr '+' expr
        | expr '-' expr
        | expr '*' expr
        | expr '/' expr
        | expr '%' expr
        | '-' expr %prec UMINUS
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
        | ID            
        | INT_NUM       
        | FLOAT_NUM     
        | INC expr
        | DEC expr
        | expr INC
        | expr DEC
        | STRING       
        | CHAR_LIT      
        ;

return_stmt:
        RETURN expr ';'
        { printf("%-20d %-40s %-20d\n", ++construct, "Return Statement", yylineno); }
        ;

if_stmt:
        IF '(' expr ')' block %prec LOWER_THAN_ELSE
          { printf("%-20d %-40s %-20d\n", ++construct, "If Statement", yylineno); }
        | IF '(' expr ')' block ELSE block
          { printf("%-20d %-40s %-20d\n", ++construct, "If-Else Statement", yylineno); }
        ;

while_stmt:
        WHILE '(' expr ')' block
       { printf("%-20d %-40s %-20d\n", ++construct, "While Statement", yylineno); }
        ;

assignment:
        ID '=' expr ';'           { printf("%-20d %-40s %s (line %d)\n", ++construct, "Assignment", $1, yylineno); }         
        | ID ADD_ASSIGN expr ';'  { printf("%-20d %-40s %s += (line %d)\n", ++construct, "Compound Assignment", $1, yylineno); }
        | ID SUB_ASSIGN expr ';'  { printf("%-20d %-40s %s -= (line %d)\n", ++construct, "Compound Assignment", $1, yylineno); }
        | ID MUL_ASSIGN expr ';'  { printf("%-20d %-40s %s *= (line %d)\n", ++construct, "Compound Assignment", $1, yylineno); }
        | ID DIV_ASSIGN expr ';'  { printf("%-20d %-40s %s /= (line %d)\n", ++construct, "Compound Assignment", $1, yylineno); }
        | ID MOD_ASSIGN expr ';'  { printf("%-20d %-40s %s %%= (line %d)\n", ++construct, "Compound Assignment", $1, yylineno); }
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
