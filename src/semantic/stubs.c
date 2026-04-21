// stubs.c
#include <stdio.h>

typedef struct ast {
    char *type;
    char *value;
    struct ast *left;
    struct ast *right;
} ast;


void optimize_tac() {}
void generate_target_code() {}


