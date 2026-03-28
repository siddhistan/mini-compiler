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
    char *instr;
    struct tac_instr *next;
} tac_instr;

tac_instr *tac_head = NULL;
tac_instr *tac_tail = NULL;
int tac_count = 0;

void add_tac(const char *instr) {
    tac_instr *node = malloc(sizeof(tac_instr));
    node->instr = strdup(instr);
    node->next  = NULL;
    if (!tac_head) {
        tac_head = node;
        tac_tail = node;
    } else {
        tac_tail->next = node;
        tac_tail = node;
    }
    tac_count++;
}

static int temp_count  = 0;
static int label_count = 0;

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


char *generate_expr_silent(ast *node) {
    if (!node) return NULL;

    if (strcmp(node->type, "integer")  == 0 ||
        strcmp(node->type, "decimal")  == 0 ||
        strcmp(node->type, "char_lit") == 0 ||
        strcmp(node->type, "string")   == 0)
        return strdup(node->value);

    if (strcmp(node->type, "id") == 0)
        return strdup(node->value);

    if (strcmp(node->type, "unary_op") == 0) {
        char *operand = generate_expr_silent(node->left);
        char *temp    = new_temp();
        char  buf[64];
        sprintf(buf, "%s = -%s", temp, operand);
        add_tac(buf);
        free(operand);
        return temp;
    }

    if (strcmp(node->type, "pre_increment") == 0) {
        char *operand = generate_expr_silent(node->left);
        char  buf[64];
        sprintf(buf, "%s = %s + 1", operand, operand);
        add_tac(buf);
        return operand;
    }

    if (strcmp(node->type, "pre_decrement") == 0) {
        char *operand = generate_expr_silent(node->left);
        char  buf[64];
        sprintf(buf, "%s = %s - 1", operand, operand);
        add_tac(buf);
        return operand;
    }

    if (strcmp(node->type, "post_increment") == 0) {
        char *operand = generate_expr_silent(node->left);
        char *temp    = new_temp();
        char  buf[64];
        sprintf(buf, "%s = %s", temp, operand);
        add_tac(buf);
        sprintf(buf, "%s = %s + 1", operand, operand);
        add_tac(buf);
        return temp;
    }

    if (strcmp(node->type, "post_decrement") == 0) {
        char *operand = generate_expr_silent(node->left);
        char *temp    = new_temp();
        char  buf[64];
        sprintf(buf, "%s = %s", temp, operand);
        add_tac(buf);
        sprintf(buf, "%s = %s - 1", operand, operand);
        add_tac(buf);
        return temp;
    }

    if (strcmp(node->type, "op")         == 0 ||
        strcmp(node->type, "multi_op")   == 0 ||
        strcmp(node->type, "comp_op")    == 0 ||
        strcmp(node->type, "logical_op") == 0 ||
        strcmp(node->type, "bit_op")     == 0) {
        char *left  = generate_expr_silent(node->left);
        char *right = generate_expr_silent(node->right);
        char *temp  = new_temp();
        char  buf[128];
        sprintf(buf, "%s = %s %s %s", temp, left, node->value, right);
        add_tac(buf);
        free(left);
        free(right);
        return temp;
    }

    if (strcmp(node->type, "log_not_op") == 0) {
        char *operand = generate_expr_silent(node->left);
        char *temp    = new_temp();
        char  buf[64];
        sprintf(buf, "%s = !%s", temp, operand);
        add_tac(buf);
        free(operand);
        return temp;
    }

    if (strcmp(node->type, "bit_not_op") == 0) {
        char *operand = generate_expr_silent(node->left);
        char *temp    = new_temp();
        char  buf[64];
        sprintf(buf, "%s = ~%s", temp, operand);
        add_tac(buf);
        free(operand);
        return temp;
    }

    if (strcmp(node->type, "address_of") == 0) {
        char *operand = generate_expr_silent(node->left);
        char *temp    = new_temp();
        char  buf[64];
        sprintf(buf, "%s = &%s", temp, operand);
        add_tac(buf);
        free(operand);
        return temp;
    }

    return NULL;
}



void generate_statement_silent(ast *node);


void generate_statement_silent(ast *node) {
    if (!node) return;

    if (strcmp(node->type, "sequence") == 0) {
        generate_statement_silent(node->left);
        generate_statement_silent(node->right);
        return;
    }

    if (strcmp(node->type, "declaration") == 0)
        return;

    if (strcmp(node->type, "declaration_init") == 0) {
        char *expr = generate_expr_silent(node->left);
        char  buf[128];
        sprintf(buf, "%s = %s", node->value, expr);
        add_tac(buf);
        free(expr);
        return;
    }

    if (strcmp(node->type, "assign") == 0) {
        int is_compound = (strcmp(node->value, "+=") == 0 ||
                           strcmp(node->value, "-=") == 0 ||
                           strcmp(node->value, "*=") == 0 ||
                           strcmp(node->value, "/=") == 0 ||
                           strcmp(node->value, "%=") == 0);
        if (!is_compound) {
            char *expr = generate_expr_silent(node->left);
            char  buf[128];
            sprintf(buf, "%s = %s", node->value, expr);
            add_tac(buf);
            free(expr);
        } else {
            char *var  = node->left->value;
            char *expr = generate_expr_silent(node->right);
            char *temp = new_temp();
            char  buf[128];
            char  op   = node->value[0];
            sprintf(buf, "%s = %s %c %s", temp, var, op, expr);
            add_tac(buf);
            sprintf(buf, "%s = %s", var, temp);
            add_tac(buf);
            free(expr);
        }
        return;
    }

    if (strcmp(node->type, "if") == 0) {
        char *L1   = new_label();
        char *Lend = new_label();
        char *cond = generate_expr_silent(node->left);
        char  buf[128];
        sprintf(buf, "if %s goto %s", cond, L1);
        add_tac(buf);
        sprintf(buf, "goto %s", Lend);
        add_tac(buf);
        char *l1_label = malloc(32);
        sprintf(l1_label, "%s:", L1);
        add_tac(l1_label);
        generate_statement_silent(node->right);
        char *lend_label = malloc(32);
        sprintf(lend_label, "%s:", Lend);
        add_tac(lend_label);
        free(cond); free(L1); free(Lend);
        free(l1_label); free(lend_label);
        return;
    }

    if (strcmp(node->type, "if-else") == 0) {
        char *Lif  = new_label();
        char *Lend = new_label();
        char *cond = generate_expr_silent(node->left);
        char  buf[128];
        sprintf(buf, "if %s goto %s", cond, Lif);
        add_tac(buf);
        generate_statement_silent(node->right->left);
        sprintf(buf, "goto %s", Lend);
        add_tac(buf);
        char *lif_label = malloc(32);
        sprintf(lif_label, "%s:", Lif);
        add_tac(lif_label);
        generate_statement_silent(node->right->right);
        char *lend_label = malloc(32);
        sprintf(lend_label, "%s:", Lend);
        add_tac(lend_label);
        free(cond); free(Lif); free(Lend);
        free(lif_label); free(lend_label);
        return;
    }

    if (strcmp(node->type, "while") == 0) {
        char *Lstart = new_label();
        char *Lbody  = new_label();
        char *Lend   = new_label();
        char  buf[128];
        char *lstart_label = malloc(32);
        sprintf(lstart_label, "%s:", Lstart);
        add_tac(lstart_label);
        char *cond = generate_expr_silent(node->left);
        sprintf(buf, "if %s goto %s", cond, Lbody);
        add_tac(buf);
        sprintf(buf, "goto %s", Lend);
        add_tac(buf);
        char *lbody_label = malloc(32);
        sprintf(lbody_label, "%s:", Lbody);
        add_tac(lbody_label);
        generate_statement_silent(node->right);
        sprintf(buf, "goto %s", Lstart);
        add_tac(buf);
        char *lend_label = malloc(32);
        sprintf(lend_label, "%s:", Lend);
        add_tac(lend_label);
        free(cond); free(Lstart); free(Lbody); free(Lend);
        free(lstart_label); free(lbody_label); free(lend_label);
        return;
    }

    if (strcmp(node->type, "return") == 0) {
        char *expr = generate_expr_silent(node->left);
        char  buf[64];
        sprintf(buf, "return %s", expr);
        add_tac(buf);
        free(expr);
        return;
    }

    if (strcmp(node->type, "io") == 0) {
        if (node->left) {
            char *expr = generate_expr_silent(node->left);
            char  buf[64];
            sprintf(buf, "io %s", expr);
            add_tac(buf);
            free(expr);
        }
        if (node->right) {
            char *expr = generate_expr_silent(node->right);
            char  buf[64];
            sprintf(buf, "io %s", expr);
            add_tac(buf);
            free(expr);
        }
        return;
    }

    if (strcmp(node->type, "pre_increment")  == 0 ||
        strcmp(node->type, "post_increment") == 0 ||
        strcmp(node->type, "pre_decrement")  == 0 ||
        strcmp(node->type, "post_decrement") == 0) {
        generate_expr_silent(node);
        return;
    }
}

/* GET TAC LIST
   Called by pseudo assembly phase to get TAC */

tac_instr *get_tac_list() {
    return tac_head;
}

int get_tac_count() {
    return tac_count;
}

/*MASTER FUNCTION — generate_tac  Silent — stores TAC into linked list  Called from parser_semantic.y after semantic passes*/

void generate_tac(ast *root) {
    generate_statement_silent(root);
}
