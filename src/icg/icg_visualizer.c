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

static int temp_count  = 0;
static int label_count = 0;
static int instr_count = 0;

char *new_temp() {
    char *buf = malloc(16);
    sprintf(buf, "t%d", ++temp_count);
    return buf;
}

char *new_label() {
    char *buf = malloc(16);
    sprintf(buf, "L%d", ++label_count);
    return buf;
}

void print_instr(const char *instr) {
    printf("  %-4d %s\n", ++instr_count, instr);
}

void print_label(const char *label) {
    printf("  %-4d %s:\n", ++instr_count, label);
}

void print_section(const char *title) {
    printf("\n-- %s --\n", title);
}


char *generate_expr(ast *node) { //takes ast and generated intermediate code return string
    if (!node) return NULL;

    /* base cases */
    if (strcmp(node->type, "integer")  == 0 ||
        strcmp(node->type, "decimal")  == 0 ||
        strcmp(node->type, "char_lit") == 0 ||
        strcmp(node->type, "string")   == 0)
        return strdup(node->value);

    if (strcmp(node->type, "id") == 0) //a
        return strdup(node->value);

    // unary minus 
    if (strcmp(node->type, "unary_op") == 0) {
        char *operand = generate_expr(node->left); //-a opr =a
        char *temp    = new_temp();
        char  buf[64];//biffer to store instruction
        sprintf(buf, "%s = -%s", temp, operand);
        print_instr(buf);
        free(operand);
        return temp;
    }// -a expr ,,, out t1=-a ,,, return t1

    // pre increment: ++x 
    if (strcmp(node->type, "pre_increment") == 0) {
        char *operand = generate_expr(node->left);
        char  buf[64];
        sprintf(buf, "%s = %s + 1", operand, operand);
        print_instr(buf);
        return operand;
    }

    // pre decrement: --x 
    if (strcmp(node->type, "pre_decrement") == 0) {
        char *operand = generate_expr(node->left);
        char  buf[64];
        sprintf(buf, "%s = %s - 1", operand, operand);
        print_instr(buf);
        return operand;
    }

    // post increment: x++ 
    if (strcmp(node->type, "post_increment") == 0) {
        char *operand = generate_expr(node->left);
        char *temp    = new_temp();
        char  buf[64];
        sprintf(buf, "%s = %s", temp, operand);
        print_instr(buf);
        sprintf(buf, "%s = %s + 1", operand, operand);
        print_instr(buf);
        return temp;
    }

    // post decrement: x-- 
    if (strcmp(node->type, "post_decrement") == 0) {
        char *operand = generate_expr(node->left);
        char *temp    = new_temp();
        char  buf[64];
        sprintf(buf, "%s = %s", temp, operand);
        print_instr(buf);
        sprintf(buf, "%s = %s - 1", operand, operand);
        print_instr(buf);
        return temp;
    }

    // binary ops 
    if (strcmp(node->type, "op")         == 0 ||
        strcmp(node->type, "multi_op")   == 0 ||
        strcmp(node->type, "comp_op")    == 0 ||
        strcmp(node->type, "logical_op") == 0 ||
        strcmp(node->type, "bit_op")     == 0) { //a+b
        char *left  = generate_expr(node->left);//a
        char *right = generate_expr(node->right);//b
        char *temp  = new_temp();//t1
        char  buf[128];
        sprintf(buf, "%s = %s %s %s", temp, left, node->value, right);//t1=a+b
        print_instr(buf);
        free(left);
        free(right);
        return temp;
    }

    // logical not 
    if (strcmp(node->type, "log_not_op") == 0) {
        char *operand = generate_expr(node->left);
        char *temp    = new_temp();
        char  buf[64];
        sprintf(buf, "%s = !%s", temp, operand);//t1+!a
        print_instr(buf);
        free(operand);
        return temp;
    }

    // bitwise not 
    if (strcmp(node->type, "bit_not_op") == 0) {
        char *operand = generate_expr(node->left);
        char *temp    = new_temp();
        char  buf[64];
        sprintf(buf, "%s = ~%s", temp, operand); //t1=~a
        print_instr(buf);
        free(operand);
        return temp;
    }

    // address of 
    if (strcmp(node->type, "address_of") == 0) {
        char *operand = generate_expr(node->left);
        char *temp    = new_temp();
        char  buf[64];
        sprintf(buf, "%s = &%s", temp, operand);
        print_instr(buf);
        free(operand);
        return temp;
    }

    return NULL;
}

void generate_statement(ast *node); //tells compiler functions exists

void generate_statement(ast *node) {//actual function code
    if (!node) return;

    /* sequence */
    if (strcmp(node->type, "sequence") == 0) {
        generate_statement(node->left);
        generate_statement(node->right);
        return;
    }

    /* declaration — no TAC needed */
    if (strcmp(node->type, "declaration") == 0)
        return;

    /* declaration with init */
    if (strcmp(node->type, "declaration_init") == 0) {
        print_section("Declaration with Init");
        char *expr = generate_expr(node->left);
        char  buf[128];
        sprintf(buf, "%s = %s", node->value, expr);
        print_instr(buf);
        free(expr);
        return;
    }

    // simple and compound assignment 
    if (strcmp(node->type, "assign") == 0) {
        int is_compound = (strcmp(node->value, "+=") == 0 ||
                           strcmp(node->value, "-=") == 0 ||
                           strcmp(node->value, "*=") == 0 ||
                           strcmp(node->value, "/=") == 0 ||
                           strcmp(node->value, "%=") == 0);
        if (!is_compound) {
            print_section("Assignment");
            char *expr = generate_expr(node->left);
            char  buf[128];
            sprintf(buf, "%s = %s", node->value, expr);
            print_instr(buf);
            free(expr);
        } else {
            print_section("Compound Assignment");
            char *var  = node->left->value;
            char *expr = generate_expr(node->right);
            char *temp = new_temp();
            char  buf[128];
            char  op   = node->value[0];
            sprintf(buf, "%s = %s %c %s", temp, var, op, expr);
            print_instr(buf);
            sprintf(buf, "%s = %s", var, temp);
            print_instr(buf);
            free(expr);
        }
        return;
    }

    // if statement 
    if (strcmp(node->type, "if") == 0) {
        print_section("If Statement");
        char *L1   = new_label();
        char *Lend = new_label();
        char *cond = generate_expr(node->left);
        char  buf[128];
        sprintf(buf, "if %s goto %s", cond, L1);
        print_instr(buf);
        sprintf(buf, "goto %s", Lend);
        print_instr(buf);
        print_label(L1);
        generate_statement(node->right);
        print_label(Lend);
        free(cond); free(L1); free(Lend);
        return;
    }

    // if-else statement 
    if (strcmp(node->type, "if-else") == 0) {
        print_section("If-Else Statement");
        char *Lif  = new_label();
        char *Lend = new_label();
        char *cond = generate_expr(node->left);
        char  buf[128];
        sprintf(buf, "if %s goto %s", cond, Lif);
        print_instr(buf);
        generate_statement(node->right->left);   /* else body */
        sprintf(buf, "goto %s", Lend);
        print_instr(buf);
        print_label(Lif);
        generate_statement(node->right->right);  /* if body */
        print_label(Lend);
        free(cond); free(Lif); free(Lend);
        return;
    }

    // while statement 
    if (strcmp(node->type, "while") == 0) {
        print_section("While Loop");
        char *Lstart = new_label();
        char *Lbody  = new_label();
        char *Lend   = new_label();
        char  buf[128];
        print_label(Lstart);
        char *cond = generate_expr(node->left);
        sprintf(buf, "if %s goto %s", cond, Lbody);
        print_instr(buf);
        sprintf(buf, "goto %s", Lend);
        print_instr(buf);
        print_label(Lbody);
        generate_statement(node->right);
        sprintf(buf, "goto %s", Lstart);
        print_instr(buf);
        print_label(Lend);
        free(cond); free(Lstart); free(Lbody); free(Lend);
        return;
    }

    // return statement 
    if (strcmp(node->type, "return") == 0) {
        print_section("Return Statement");
        char *expr = generate_expr(node->left);
        char  buf[64];
        sprintf(buf, "return %s", expr);
        print_instr(buf);
        free(expr);
        return;
    }

    // io statement 
    if (strcmp(node->type, "io") == 0) {
        print_section("IO Statement");
        if (node->left) {
            char *expr = generate_expr(node->left);
            char  buf[64];
            sprintf(buf, "io %s", expr);
            print_instr(buf);
            free(expr);
        }
        if (node->right) {
            char *expr = generate_expr(node->right);
            char  buf[64];
            sprintf(buf, "io %s", expr);
            print_instr(buf);
            free(expr);
        }
        return;
    }

    // standalone expression 
    if (strcmp(node->type, "pre_increment")  == 0 ||
        strcmp(node->type, "post_increment") == 0 ||
        strcmp(node->type, "pre_decrement")  == 0 ||
        strcmp(node->type, "post_decrement") == 0) {
        print_section("Expression");
        generate_expr(node);
        return;
    }
}

void generate_tac(ast *root) {
    printf("\n========== Intermediate Code Generation (TAC) ==========\n");
    generate_statement(root);
    printf("\n---------------------------------------------------------\n");
    printf("  Total Instructions: %d\n", instr_count);
    printf("=========================================================\n\n");
}
