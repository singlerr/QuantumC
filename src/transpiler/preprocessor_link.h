#ifndef _PREPROCESSOR_LINK_H_
#define _PREPROCESSOR_LINK_H_

#include "preprocessor/stringbuilder.h"
#include <stdio.h>

struct ast;

int init_ctx (struct string_builder *sb, FILE *f);
int preprocessor_lex ();
int tr_process (const char *content, struct ast **out);

#endif
