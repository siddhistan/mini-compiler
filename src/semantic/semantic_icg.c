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


int is_declared(char *var, declaration *head) {
    while (head) {
        if (strcmp(head->name, var) == 0) return 1;
        head = head->next;
    }
    return 0;
}

char *get_type(char *var, declaration *head) {
    while (head) {
        if (strcmp(head->name, var) == 0) return head->type;
        head = head->next;
    }
    return NULL;
}


char *get_expr_type(ast *node, declaration *head) {
    if (!node) return NULL;

    if (strcmp(node->type, "integer")  == 0) return "int";
    if (strcmp(node->type, "decimal")  == 0) return "float";
    if (strcmp(node->type, "char_lit") == 0) return "char";
    if (strcmp(node->type, "string")   == 0) return "string";

    if (strcmp(node->type, "id") == 0)
        return get_type(node->value, head);

    if (strcmp(node->type, "logical_op")  == 0 ||
        strcmp(node->type, "comp_op")     == 0 ||
        strcmp(node->type, "multi_op")    == 0 ||
        strcmp(node->type, "log_not_op")  == 0)
        return "int";

    if (strcmp(node->type, "unary_op") == 0)
        return get_expr_type(node->left, head);

    if (strcmp(node->type, "pre_increment")  == 0 ||
        strcmp(node->type, "pre_decrement")  == 0 ||
        strcmp(node->type, "post_increment") == 0 ||
        strcmp(node->type, "post_decrement") == 0)
        return get_expr_type(node->left, head);

    char *left  = get_expr_type(node->left,  head);
    char *right = get_expr_type(node->right, head);

    if (left && right) {
        if (strcmp(left, "double") == 0 || strcmp(right, "double") == 0) return "double";
        if (strcmp(left, "float")  == 0 || strcmp(right, "float")  == 0) return "float";
        return "int";
    }

    return left ? left : right;
}

int types_compatible(char *target, char *source) {
    if (!target || !source)                        return 1;
    if (strcmp(target, source) == 0)               return 1;
    if (strcmp(target, "float")  == 0 &&
        strcmp(source, "int")    == 0)             return 1;
    if (strcmp(target, "double") == 0 &&
        strcmp(source, "int")    == 0)             return 1;
    if (strcmp(target, "double") == 0 &&
        strcmp(source, "float")  == 0)             return 1;
    return 0;
}


int check_duplicate(declaration *head) {
    int errors = 0;
    for (declaration *i = head; i; i = i->next)
        for (declaration *j = i->next; j; j = j->next)
            if (strcmp(i->name, j->name) == 0)
                errors++;
    return errors;
}


int check_undeclared(ast *root, declaration *head) {
    if (!root) return 0;
    int errors = 0;

    if (strcmp(root->type, "id") == 0)
        if (!is_declared(root->value, head))
            errors++;

    errors += check_undeclared(root->left,  head);
    errors += check_undeclared(root->right, head);
    return errors;
}


int check_type_mismatch(ast *root, declaration *head) {
    if (!root) return 0;
    int errors = 0;

    if (strcmp(root->type, "assign") == 0) {
        char *var_name  = NULL;
        char *expr_type = NULL;

        int is_compound = (strcmp(root->value, "+=") == 0 ||
                           strcmp(root->value, "-=") == 0 ||
                           strcmp(root->value, "*=") == 0 ||
                           strcmp(root->value, "/=") == 0 ||
                           strcmp(root->value, "%=") == 0);

        if (!is_compound) {
            var_name  = root->value;
            expr_type = get_expr_type(root->left, head);
        } else {
            if (root->left && strcmp(root->left->type, "id") == 0) {
                var_name  = root->left->value;
                expr_type = get_expr_type(root->right, head);
            }
        }

        if (var_name) {
            char *var_type = get_type(var_name, head);
            if (var_type && expr_type && !types_compatible(var_type, expr_type))
                errors++;
        }
    }

    if (strcmp(root->type, "declaration_init") == 0) {
        char *var_type  = get_type(root->value, head);
        char *expr_type = get_expr_type(root->left, head);
        if (var_type && expr_type && !types_compatible(var_type, expr_type))
            errors++;
    }

    errors += check_type_mismatch(root->left,  head);
    errors += check_type_mismatch(root->right, head);
    return errors;
}


int check_division_by_zero(ast *root) {
    if (!root) return 0;
    int errors = 0;

    if (strcmp(root->type, "op") == 0 &&
        root->value &&
        strcmp(root->value, "/") == 0)
        if (root->right &&
            strcmp(root->right->type, "integer") == 0 &&
            strcmp(root->right->value, "0")      == 0)
            errors++;

    errors += check_division_by_zero(root->left);
    errors += check_division_by_zero(root->right);
    return errors;
}


int run_semantic_analysis(ast *root, declaration *head) {
    int errors = 0;
    errors += check_duplicate(head);
    errors += check_undeclared(root, head);
    errors += check_type_mismatch(root, head);
    errors += check_division_by_zero(root);
    return errors;
}

