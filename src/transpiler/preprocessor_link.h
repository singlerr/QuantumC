#ifndef _PREPROCESSOR_LINK_H_
#define _PREPROCESSOR_LINK_H_

#include "ast.h"
#include "preprocessor/stringbuilder.h"
#include <stdio.h>

int init_ctx (struct string_builder *sb, FILE *f);
int preprocessor_lex ();
int tr_process (const char *content, ast_node **out);

#endif
