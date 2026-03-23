%{
    #include<stdio.h>
    #include<stdlib.h>
    #include<string.h>
    #include "parser_visualizer.tab.h"
%}

%option yylineno
%x COMMENT  

%%

"//".*                      ;

"/*"        { BEGIN(COMMENT); }

<COMMENT>"*/"  { BEGIN(INITIAL); }

<COMMENT>.|\n  ;   /* ignore everything inside comment */

     
int  {return INT;}
float {return FLOAT;}
char {return CHAR;}
return {return RETURN;}
double {return DOUBLE;}
if {return IF;}
else {return ELSE;}
while {return WHILE;}

printf|scanf            {return IO;}

[a-zA-Z][a-zA-Z0-9]*    { yylval.str = strdup(yytext); return ID; }

[{},;()]                {return yytext[0];}

[0-9]+\.[0-9]*|\.[0-9]+    { yylval.str = strdup(yytext); return FLOAT_NUM; }
[0-9]+                      { yylval.str = strdup(yytext); return INT_NUM; }

"&&"                        { return AND; }
"||"                        { return OR; }

[&|~^]                    { return yytext[0]; }

"=="                    {return EQ;}
"<="                    {return LEQ;}
">="                    {return GEQ;}
"!="                    {return NEQ;}

"++"                   { return INC; }
"--"                   { return DEC; }
"+="                   { return ADD_ASSIGN; }
"-="                   { return SUB_ASSIGN; }
"*="                   { return MUL_ASSIGN; }
"/="                   { return DIV_ASSIGN; }
"%="                   { return MOD_ASSIGN; }

[=><!]                      {return yytext[0];}
[+/*\-%]                {return yytext[0];}

\'([^\\']|\\.)\'      { yylval.str = strdup(yytext); return CHAR_LIT; }

\"([^\\"]|\\.)*\"     { yylval.str = strdup(yytext); return STRING; }
   
[0-9]+[0-9a-zA-Z]+   { printf("Invalid identifier: %s\n", yytext); }

[ \n\t\r]+               ;    

.                     { printf("Lexical error: Invalid character:\t %s\n",yytext); }

%%



int main(int argc, char *argv[])
{
    printf("\n=================== Parser Output =====================\n\n");
    printf("%-20s %-40s %-20s\n", "Construct No", "Construct Type", "Details");
    printf("----------------------------------------------------------------\n");
    
   extern FILE *yyin;

   if(argc<2)
   {
      printf("Usage: %s <input_file>\n",argv[0]);
      return 1;
   }

   yyin=fopen(argv[1],"r");

   if(!yyin)
   {
     printf("File can't be opened %s\n",argv[1]);
     return 1;
   }

   yyparse();

   return 0;
}

int yywrap()
{
    return 1;
}