#ifndef _AST_H_
#define _AST_H_

struct ast_node
{
    int code;
    int child_count;

    union
    {
        double d;
        float f;
        int i;
        unsigned int ui;
        int b;
        const char *str;
    } data;

    struct ast_node **children;
};

struct ast_node *new_ast_node(int code, ...);
struct ast_node *new_ast_node_name(int code, const char *name, ...);
void append_child(struct ast_node *node, const struct ast_node *child);
const char *to_ast_string(int code);

#endif