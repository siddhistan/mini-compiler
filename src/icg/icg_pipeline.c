#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct declaration {
    char *type;
    char *name;
    int   line;
    struct declaration *next;
} declaration;

typedef struct ast {
    char *type;
    char *value;
    struct ast *left;
    struct ast *right;
} ast;

typedef struct tac_instr {
    char *instr;  // stores one TAC line: "t1 = a + b"
    struct tac_instr *next;   // pointer to next instruction
} tac_instr;

tac_instr *tac_head = NULL;   // points to FIRST instruction in list
tac_instr *tac_tail = NULL;   // points to LAST instruction in list
int tac_count = 0;

void add_tac(const char *instr) {
    tac_instr *node = malloc(sizeof(tac_instr));
    node->instr = strdup(instr); //Makes a copy of the instruction string and stores it. strdup = string duplicate.
    node->next  = NULL; //new empty
    if (!tac_head) {
        tac_head = node;
        tac_tail = node;  //If list is empty — this node becomes both head and tail.
    } else {
        tac_tail->next = node;
        tac_tail = node;  //If list has nodes — attach new node at the end, update tail pointer.
    }
    tac_count++;
} 
 /*add_tac("t1 = b * 2")
add_tac("t2 = a + t1")
add_tac("x = t2")
add_tac("x=9")

List becomes:
tac_head → ["t1 = b * 2"] → ["t2 = a + t1"] → ["x = t2"] → NULL
                                                              ↑
                                                           tac_tail*/


static int temp_count  = 0;
static int label_count = 0;

char *new_temp() {
    char *buf = malloc(16); // allocate 16 bytes for string
    sprintf(buf, "t%d", ++temp_count);  // generate temp name like t1, t2, etc.
    return buf;
}

char *new_label() {
    char *buf = malloc(16);
    sprintf(buf, "L%d", ++label_count);
    return buf;
}

char *eval_expr(ast *node) { //Takes an AST node, generates TAC for it, returns the name of the variable/temp holding the result.
    if (!node) return NULL;

    if (strcmp(node->type, "integer")  == 0 || 
        strcmp(node->type, "decimal")  == 0 ||
        strcmp(node->type, "char_lit") == 0 ||
        strcmp(node->type, "string")   == 0)
        return strdup(node->value);//If node has value like 5, 3.14, 'a', "hello" — just return the value directly. No TAC needed. strdup makes a copy of the string.
c

    if (strcmp(node->type, "id") == 0)
        return strdup(node->value); //If node is a variable like x, y — just return the variable name directly. No TAC needed.

    if (strcmp(node->type, "unary_op") == 0) { 
        char *operand = eval_expr(node->left); //-x
        char *temp    = new_temp(); //t1
        char  buf[64];
        sprintf(buf, "%s = -%s", temp, operand); //t1=-x
        add_tac(buf); //Store this instruction in the linked list
        free(operand);
        return temp;
    }

    if (strcmp(node->type, "pre_increment") == 0) {
        char *operand = eval_expr(node->left); //x
        char  buf[64];
        sprintf(buf, "%s = %s + 1", operand, operand);
        add_tac(buf);
        return operand; //// returns "x" because x is updated in place
    }

    if (strcmp(node->type, "pre_decrement") == 0) {
        char *operand = eval_expr(node->left);
        char  buf[64];
        sprintf(buf, "%s = %s - 1", operand, operand);
        add_tac(buf);
        return operand;
    }

    if (strcmp(node->type, "post_increment") == 0) {
        char *operand = eval_expr(node->left);
        char *temp    = new_temp();
        char  buf[64];
        sprintf(buf, "%s = %s", temp, operand); // "t1 = x" save OLD value
        add_tac(buf);
        sprintf(buf, "%s = %s + 1", operand, operand); // "x = x + 1" update x
        add_tac(buf);
        return temp;
    }

    if (strcmp(node->type, "post_decrement") == 0) {
        char *operand = eval_expr(node->left);
        char *temp    = new_temp();
        char  buf[64];
        sprintf(buf, "%s = %s", temp, operand);
        add_tac(buf);
        sprintf(buf, "%s = %s - 1", operand, operand);
        add_tac(buf);
        return temp;+
    } /*generate_expr_silent(a + b*2)
    left  = generate_expr_silent(a)  → returns "a"
    right = generate_expr_silent(b*2)
                left  = "b"
                right = "2"
                temp  = "t1"
                add_tac("t1 = b * 2")
                return "t1"
    temp = "t2"
    add_tac("t2 = a + t1")
    return "t2"

List now has:
    t1 = b * 2
    t2 = a + t1*/

    if (strcmp(node->type, "op")         == 0 ||
        strcmp(node->type, "multi_op")   == 0 ||
        strcmp(node->type, "comp_op")    == 0 ||
        strcmp(node->type, "logical_op") == 0 ||
        strcmp(node->type, "bit_op")     == 0) {
        char *left  = eval_exprt(node->left);  
        char *right = eval_expr(node->right);
        char *temp  = new_temp();
        char  buf[128];
        sprintf(buf, " %s = %s %s %s", temp, left, node->value, right); //a+b   t1= left  node->value  right
        add_tac(buf);
        free(left);
        free(right);
        return temp;
    }

    if (strcmp(node->type, "log_not_op") == 0) {
        char *operand = eval_expr(node->left);
        char *temp    = new_temp();
        char  buf[64];
        sprintf(buf, "%s = !%s", temp, operand);//t1=!x
        add_tac(buf);
        free(operand);
        return temp;
    }

    if (strcmp(node->type, "bit_not_op") == 0) {
        char *operand = eval_expr(node->left);
        char *temp    = new_temp();
        char  buf[64];
        sprintf(buf, "%s = ~%s", temp, operand);
        add_tac(buf);
        free(operand);
        return temp;
    }

    if (strcmp(node->type, "address_of") == 0) {
        char *operand = eval_expr(node->left); 
        char *temp    = new_temp();
        char  buf[64];
        sprintf(buf, "%s = &%s", temp, operand); //t1=&b
        add_tac(buf);
        free(operand);
        return temp;
    }

    return NULL;
}

void process_stmt(ast *node); //forward decleration to tell compiler this function exists before we define it

void process_stmt(ast *node) {
    if (!node) return;0

    if (strcmp(node->type, "sequence") == 0) {  
       process_stmt(node->left);// process first statement
        process_stmt(node->right);// process 2nd statement
        return; // STOP HERE, don't check anything else   
    }
// never reaches here for sequence nodes otherwise will check everything 
    if (strcmp(node->type, "declaration") == 0)
        return;    //Pure declaration with no value — nothing to generate, just skip.   int x;  no TAC needed

    if (strcmp(node->type, "declaration_init") == 0) {  //int x = 5;
        char *expr = eval_expr(node->left);  //Generate TAC for the right side expression.
                                                         // For int x = a + b, this generates temp for a+b and returns it.
        char  buf[128];
        sprintf(buf, "%s = %s", node->value, expr); //"x = t1" where node->value = "x" and expr = "t1".
        add_tac(buf);
        free(expr);
        return;
    }
           //x = expr simple assign or compound assign like x += expr //x=x+a*b
    if (strcmp(node->type, "assign") == 0) {
        int is_compound = (strcmp(node->value, "+=") == 0 ||
                           strcmp(node->value, "-=") == 0 ||
                           strcmp(node->value, "*=") == 0 ||
                           strcmp(node->value, "/=") == 0 ||
                           strcmp(node->value, "%=") == 0);    //Check if it's a compound assignment like +=
        if (!is_compound) { //x=a//a=t1 
            char *expr =eval_expr(node->left); //Generate TAC for the right side expression, get temp/var holding the result.
            char  buf[128];
            sprintf(buf, "%s = %s", node->value, expr); //Generate TAC for simple assignment: "x = t1" where node->value = "x" and expr = "t1".
            add_tac(buf);
            free(expr);
        } else {
            char *var  = node->left->value;
            char *expr = eval_expr(node->right);
            char *temp = new_temp();
            char  buf[128];
            char  op   = node->value[0];
            sprintf(buf, "%s = %s %c %s", temp, var, op, expr);
              // "t1 = x + 5"
            add_tac(buf);
            sprintf(buf, "%s = %s", var, temp);
              // "x = t1"
            add_tac(buf);
            free(expr);
        }   //Compound x += 5 becomes two instructions: t1 = x + 5 then x = t1. //x=x+5 
        return;
    }

    if (strcmp(node->type, "if") == 0) {
        char *L1   = new_label();   // "L1" — if body start
        char *Lend = new_label();   // "L2" — after if
        char *cond = eval_expr(node->left);  //holds the condition expression. For if(a > 5) this generates t1 = a > 5 and returns "t1".
        char  buf[128];
        sprintf(buf, "if %s goto %s", cond, L1);    // "if t1 goto L1"
        add_tac(buf);
        sprintf(buf, "goto %s", Lend);               //goto L2 
        add_tac(buf);
        char *l1_label = malloc(32);
        sprintf(l1_label, "%s:", L1);           //L1
        add_tac(l1_label);
        process_stmt(node->right);  // generate if body Recursively generate TAC for everything inside the if block.
        char *lend_label = malloc(32);
        sprintf(lend_label, "%s:", Lend);       //L2
        add_tac(lend_label);
        free(cond); free(L1); free(Lend);
        free(l1_label); free(lend_label);
        return;    
        /* if(a > 5) {
    b = 10;
}
    t1 = a > 5        ← eval_expr(condition)
if t1 goto L1     ← condition true? jump to body
goto L2           ← condition false? skip body
L1:               ← body starts here
b = 10            ← process_stmt(body)
L2:               ← everyone meets here
        condition true  → jump to L1 → run body → fall to L2
        condition false → fall through goto L2  → skip body*/
    }

    if (strcmp(node->type, "if-else") == 0) {
        char *Lif  = new_label();   // L1 — if body
        char *Lend = new_label();   // L2 — else body
        char *cond = eval_expr(node->left); //if(x>8) //t1=x>8
        char  buf[128];
        sprintf(buf, "if %s goto %s", cond, Lif); // if condition true → jump to L1
        add_tac(buf);
        process_stmt(node->right->left);                                                                             
        sprintf(buf, "goto %s", Lend);  // after else, skip if body goto l2
        add_tac(buf);
        char *lif_label = malloc(32);
        sprintf(lif_label, "%s:", Lif);   // L1: if body starts
        add_tac(lif_label);
        process_stmt(node->right->right); // if body runs if condition true
        char *lend_label = malloc(32);
        sprintf(lend_label, "%s:", Lend);  // L2: everyone meets here
        add_tac(lend_label);
        free(cond); free(Lif); free(Lend);
        free(lif_label); free(lend_label);
        return; 
                /*Note — else body comes BEFORE if body in TAC because:
                if cond goto L1  ← if true jump OVER else body
                else body        ← runs when condition false
                goto L2          ← else done, skip if body
                L1:              ← if body starts here
                if body
                L2:              ← both paths end here*/
    }

    if (strcmp(node->type, "while") == 0) {
        char *Lstart = new_label(); //LOOP start
        char *Lbody  = new_label();  //loop body
        char *Lend   = new_label();// loop end
        char  buf[128];
        char *lstart_label = malloc(32);
        sprintf(lstart_label, "%s:", Lstart);  // "L1:" — come back here
        add_tac(lstart_label); //store condition
        char *cond = eval_expr(node->left);  //Generate condition TAC. For while(i < 10) generates t1 = i < 10 returns "t1".
        sprintf(buf, "if %s goto %s", cond, Lbody);  // if true run body
        add_tac(buf);
        sprintf(buf, "goto %s", Lend); // if false exit loop
        add_tac(buf);
        char *lbody_label = malloc(32);
        sprintf(lbody_label, "%s:", Lbody);  // "L2:" body starts
        add_tac(lbody_label);
        process_stmt(node->right);  // loop body
        sprintf(buf, "goto %s", Lstart);  // go back to check condition, Store "goto L1" — body done, go back to start, check condition again.
        add_tac(buf);
        char *lend_label = malloc(32);
        sprintf(lend_label, "%s:", Lend); // "L3:" loop end
        add_tac(lend_label);
        free(cond); free(Lstart); free(Lbody); free(Lend);
        free(lstart_label); free(lbody_label); free(lend_label);
        return;  
/*L1: ←──────────────────┐
    check condition     │
    true  → goto L2     │
    false → goto L3     │
L2:                     │
    body executes       │
    goto L1 ────────────┘
L3:
    continue program*/

    }

    if (strcmp(node->type, "return") == 0) { //node->left holds what we're returning. Generate TAC for it. For return x + 1, generates t1 = x + 1 returns "t1".

        char *expr = eval_expr(node->left);
        char  buf[64];
        sprintf(buf, "return %s", expr);
        add_tac(buf);
        free(expr);
        return; /*return x;      → add_tac("return x")
                return x + 1;  → add_tac("t1 = x + 1")//a+b*c
               → add_tac("return t1")*/
    }

    if (strcmp(node->type, "io") == 0) { //scanf,printf
        if (node->left) { //node->left = first argument. For printf("hello", x): //scanf("Enter number: %d", &x);
                        //node->left = "hello" string
                        //generates io "hello"
            char *expr = eval_expr(node->left);
            char  buf[64];
            sprintf(buf, "io %s", expr);//Enter number: %d
            add_tac(buf);
            free(expr); 

        }
        if (node->right) { //node->right = second argument if exists. For printf("hello", x):
                        //node->right = x
                        //generates io x
            char *expr = eval_expr(node->right);//&x
            char  buf[64];
            sprintf(buf, "io %s", expr);//&x
            add_tac(buf);
            free(expr);
        }
        return;
    }

    if (strcmp(node->type, "pre_increment")  == 0 ||
        strcmp(node->type, "post_increment") == 0 ||
        strcmp(node->type, "pre_decrement")  == 0 ||
        strcmp(node->type, "post_decrement") == 0) {
        eval_expr(node); //ignore t1 
        return;
    }
}


tac_instr *get_tac_list() {    // get head of list
    return tac_head;  //Returns pointer to the first instruction in the linked list of TAC instructions. The pseudo assembly phase will call this function to retrieve the generated TAC and convert it to assembly.
}

int get_tac_count() {
    return tac_count;
}

void generate_tac(ast *root) {
    process_stmt(root);  // walk entire AST silently, store TAC in list
}
