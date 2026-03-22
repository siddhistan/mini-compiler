%{
    #include<stdio.h>
    int yylex(void);
    int token=0;
%}

%option yylineno

%x COMMENT  

%%

"//".*                      ;

"/*"        { BEGIN(COMMENT); }

<COMMENT>"*/"  { BEGIN(INITIAL); }

<COMMENT>.|\n  ;   /* just ignore everything that user has written inside these comment */

     
int|float|char|return|double|if|else|while  { printf("%-20d %-40s %-20s\n", ++token, "Keyword", yytext); }

printf|scanf                   { printf("%-20d %-40s %-20s\n", ++token, "I/O Identifiers", yytext); }

[a-zA-Z][a-zA-Z0-9]*    { printf("%-20d %-40s %-20s\n", ++token, "Identifiers", yytext); }

[();,{}]                      { printf("%-20d %-40s %-20s\n", ++token, "Separators", yytext); }

[0-9]+\.[0-9]*|\.[0-9]+    { printf("%-20d %-40s %-20s\n", ++token, "Floating point numbers", yytext); }

[0-9]+                { printf("%-20d %-40s %-20s\n", ++token, "Integers", yytext); }

"++"|"--"|"+="|"-="|"*="|"/="|"%="|"=="|"<="|">="|"!="   { printf("%-20d %-40s %-20s\n", ++token, "Multi-character Operators", yytext); }

"&&"|"||"    { printf("%-20d %-40s %-20s\n", ++token, "Logical Operators", yytext); }

"&"|"|"      { printf("%-20d %-40s %-20s\n", ++token, "Bitwise Operators", yytext); }

[=><!]                 { printf("%-20d %-40s %-20s\n", ++token, "Single Character Operators", yytext); }

[+\-*/%]               { printf("%-20d %-40s %-20s\n", ++token, "Arithmetic Operators", yytext); }

\'([^\\']|\\.)\'      { printf("%-20d %-40s %-20s\n", ++token, "Character literals", yytext); }

\"([^\\"]|\\.)*\"     { printf("%-20d %-40s %-20s\n", ++token, "String literals", yytext); }


[0-9]+[0-9a-zA-Z]+     { printf("Invalid Identifier at line %d:\t %s\n",yylineno,yytext); }

[ \n\t\r]+               ;    

.                     { printf("Invalid character at line %d:\t %s\n",yylineno, yytext); }


%%

int main(int argc, char *argv[])
{
    printf("\n=== Lexer Output (Token Stream) ===\n\n");
    printf("%-20s %-40s %-20s\n", "Token No", "Token Type", "Lexeme");
    printf("-------------------------------------------------------------------------\n");

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

   yylex();

   return 0;
}

int yywrap()
{
    return 1;
}
