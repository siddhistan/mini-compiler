%{
    #include<stdio.h>
    int yylex(void);
    int token=0;
%}

%x COMMENT  

%%

"//".*                      ;

"/*"        { BEGIN(COMMENT); }

<COMMENT>"*/"  { BEGIN(INITIAL); }

<COMMENT>.|\n  ;   /* just ignore everything that user has written inside these comment */

     
int|float|char|return|double|if|else|while  {printf("Token %-4d Keywords:\t %s\n",++token,yytext);}

printf|scanf                    {printf("Token %-4d I/O Identifiers:\t %s\n",++token,yytext);}

[a-zA-Z][a-zA-Z0-9]*    {printf("Token %-4d Identifiers:\t %s\n",++token,yytext);}

[();,{}]                       {printf("Token %-4d Separators:\t %s\n",++token,yytext);}

[0-9]+\.[0-9]*|\.[0-9]+     {printf("Token %-4d Floating point numbers:\t %s\n",++token,yytext);}

[0-9]+                {printf("Token %-4d Integer numbers:\t %s\n",++token,yytext);}

"&&"|"||"    { printf("Token %-4d Logical operators:\t %s\n",++token, yytext); }

"=="|"<="|">="|"!="      {printf("Token %-4d multi character Operators:\t %s\n",++token,yytext);}

"&"|"|"      { printf("Token %-4d Bitwise Operators:\t %s\n",++token,yytext); }

[=><!]                  {printf("Token %-4d single character Operators:\t %s\n",++token,yytext);}

[+\-*/%]                {printf("Token %-4d Arithmetic Operators:\t %s\n",++token,yytext);}

\'([^\\']|\\.)\'      { printf("Token %-4d Character literal:\t %s\n",++token,yytext); }

\"([^\\"]|\\.)*\"     { printf("Token %-4d String literal:\t %s\n",++token,yytext); }


[0-9]+[0-9a-zA-Z]+     { printf("Invalid Identifier:\t %s\n",yytext); }

[ \n\t\r]+               ;    

.                     { printf("Invalid character:\t %s\n",yytext); }


%%

int main(int argc, char *argv[])
{
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