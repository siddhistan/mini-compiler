%{
#include <stdio.h>
#include<string.h>
#include <stdlib.h>
int yylex();
void yyerror(const char *s);
extern int yylineno;
typedef struct declaration         
{
    char *type;
    char *name;
    int   line; 
    struct declaration *next;
}declaration;                       

declaration *head=NULL;

void add_declaration(char *type, char* name,int line)
{
    declaration *d=malloc(sizeof(declaration));
    d->type=strdup(type);
    d->name=strdup(name);
    d->line = line; 
    d->next=NULL;

    if(head==NULL)
    {
        head=d;     //head now will always point to first node since head is a global variable, changes are permanent
    }

     else
     {
           declaration *temp=head;
           while(temp->next!=NULL)
        {
            temp=temp->next;
        }                   
                             //after while loop,temp points to 2nd last node, temp->next points to null

            temp->next=d;     //now temp points to new attached node
     }

}

typedef struct ast
 {
    char *type;
    char *value;
    struct ast *left;
    struct ast *right;
} ast;

ast *create_node(char *type, char *value,ast *left, ast *right)
{
    ast *node=malloc(sizeof(ast));
    node->type= strdup(type);
    node->value = value? strdup(value): NULL;
    node->left=left;
    node->right=right;
    return node;
}

void run_semantic_analysis(ast *root, declaration *head);


%}

%union
{
        char *str;
         struct ast *node;
}

%token <str> ID INT_NUM FLOAT_NUM STRING CHAR_LIT
%token INT FLOAT CHAR DOUBLE 
%token RETURN IF ELSE WHILE
%token AND OR EQ LEQ GEQ NEQ
%token INC DEC
%token ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
%token IO

%type <str> type
%type <node> expr statement statement_list block
%type <node> declaration assignment if_stmt while_stmt return_stmt io_stmt

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
        {run_semantic_analysis($1,head);}   //root of tree strucure and head pointer of declaration list is passed to semantic phase
        ;

block:
        '{' statement_list '}'
        {$$=$2;}                //block returns whatever value the statement_list returns 
        ;

statement_list: 
        statement_list statement
         { $$ = create_node("sequence", NULL, $1, $2); }   //sequence has 2 children: left child will be left stmt and right will be right stmt
        | statement
          {$$=$1;}    
        ;

statement: 
        declaration     {$$=$1;}
        | return_stmt   {$$=$1;}       //they return the pointer to whatever the value their respective statements return
        | if_stmt       {$$=$1;}
        | while_stmt    {$$=$1;}
        | assignment     {$$=$1;}
        | io_stmt       {$$=$1;}
        | expr ';'      {$$=$1;}
        ;

io_stmt:
        IO '(' expr ')' ';'
        { $$ = create_node("io", NULL, $3, NULL); }    //if it has one expression, that will be its left child
        | IO '(' expr ',' expr ')' ';'
       { $$ = create_node("io", NULL, $3, $5); }   //if it has 2 expressions, one will be left child and one will be right child
        ;

declaration:
        type ID ';'
        {   add_declaration($1,$2,yylineno); 
            $$=create_node("declaration",$2,NULL,NULL); //ex: int x; it has no children
        }      
        | type ID '=' expr ';'
        {
           add_declaration($1, $2,yylineno);
           $$=create_node("declaration_init",$2,$4,NULL);
        }  
        ;

expr:
        expr '+' expr            { $$= create_node("op","+",$1,$3); }       //op: +, left child is expr, right child is expr
        | expr '-' expr          {$$= create_node("op","-",$1,$3) ;}
        | expr '*' expr          { $$= create_node("op","*",$1,$3); }   
        | expr '/' expr          {$$= create_node("op","/",$1,$3); } 
        | expr '%' expr          {$$= create_node("op","%",$1,$3); } 
        | '-' expr %prec UMINUS       {$$= create_node("unary_op","-",$2,NULL); }  //unary_op: -, left child is expr, right is NULL
        | expr EQ expr            {$$= create_node("multi_op","==",$1,$3); }
        | expr NEQ expr           {$$= create_node("multi_op","!=",$1,$3); }
        | expr LEQ expr            {$$= create_node("multi_op","<=",$1,$3); }
        | expr GEQ expr            {$$= create_node("multi_op",">=",$1,$3); }
        | expr '<' expr              {$$= create_node("comp_op","<",$1,$3); }
        | expr '>' expr               {$$= create_node("comp_op",">",$1,$3); } 
        | expr AND expr                {$$= create_node("logical_op","&&",$1,$3); }
        | expr OR expr                 {$$= create_node("logical_op","||",$1,$3); }
        | '!' expr                     {$$= create_node("log_not_op","!",$2,NULL); }
        | '~' expr                     {$$= create_node("bit_not_op","~",$2,NULL); }
        | expr '^' expr              {$$= create_node("bit_op","^",$1,$3); }
        | expr '&' expr              {$$= create_node("bit_op","&",$1,$3); }
        | expr '|' expr              {$$= create_node("bit_op","|",$1,$3); }
        | '&' expr                     {$$=create_node("address_of","&",$2,NULL);}
        | '(' expr ')'               {$$= $2;}
        | ID                          {$$=create_node("id",$1,NULL,NULL);}
        | INT_NUM                     {$$=create_node("integer",$1,NULL,NULL);}
        | FLOAT_NUM                   {$$=create_node("decimal",$1,NULL,NULL);}
        | INC expr                       { $$ = create_node("pre_increment", "++", $2, NULL); }                 
        | DEC expr                       { $$ = create_node("pre_decrement", "--", $2, NULL); }   
        | expr INC                       { $$ = create_node("post_increment", "++", $1, NULL); }  
        | expr DEC                       { $$ = create_node("post_decrement", "--", $1, NULL); }  
        | STRING                      {$$=create_node("string",$1,NULL,NULL);}
        | CHAR_LIT                    {$$=create_node("char_lit",$1,NULL,NULL);}     
        ;

return_stmt:
        RETURN expr ';'
       {$$=create_node("return",NULL,$2,NULL);}            //return will have one left child
        ;

if_stmt:
        IF '(' expr ')' block %prec LOWER_THAN_ELSE
       {$$=create_node("if",NULL,$3,$5);}           //if has condition as left child and body as right child
        | IF '(' expr ')' block ELSE block
        { $$ = create_node("if-else", NULL, $3, create_node("branches", NULL, $5, $7)); }

        //left child is condition, right side is branches, which will contain if body and else body on its left and right
        ;

while_stmt:
        WHILE '(' expr ')' block
      { $$ = create_node("while", NULL, $3, $5); }   //same as if
        ;

assignment:
        ID '=' expr ';'              { $$ = create_node("assign", $1, $3, NULL); }        //assign x, left child is expr, right is null
        | ID ADD_ASSIGN expr ';'    {$$ = create_node("assign","+=", create_node("id",$1,NULL,NULL), $3 ); }   //assign +=, left child is id: x, right is expr;
        | ID SUB_ASSIGN expr ';'     {$$ = create_node("assign","-=", create_node("id",$1,NULL,NULL), $3 ); }
        | ID MUL_ASSIGN expr ';'    {$$ = create_node("assign","*=", create_node("id",$1,NULL,NULL), $3 ); }
        | ID DIV_ASSIGN expr ';'    {$$ = create_node("assign","/=", create_node("id",$1,NULL,NULL), $3 ); }
        | ID MOD_ASSIGN expr ';'     {$$ = create_node("assign","%=", create_node("id",$1,NULL,NULL), $3 ); }
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
    printf("Syntax error at line: %d: %s\n",yylineno, s);
}
