#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include <stdio.h>

struct sem_program;

void set_codegen_output(FILE *);
void gen_program(struct sem_program *);

#endif