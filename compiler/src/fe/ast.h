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

    struct ast_node *children[];
};

#endif