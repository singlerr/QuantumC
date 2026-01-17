#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include <stdio.h>

struct program;

void set_codegen_output(FILE *);
void gen_program(struct program *);

#endif