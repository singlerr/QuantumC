#ifndef _FILEBUF_H_
#define _FILEBUF_H_

#include <stdio.h>

#define BUFCHNK 4096

typedef struct filebuf
{
    char *buf;
    int pos;
    int bufsize;
} FILEBUF;

// check buf is initialized and ready to call readchar
int buf_initialized();

// allocate new buffer
// if previous buffer exists, frees
void init_buf();

// read one character from FILE stream and store it to buffer.
// return char read
int readchar(FILE *fin);

// read all contents read up so far, pointing buffer position to head
// return size of contents read
int getbuf(char **buf);

// read all contents read up so far, without resetting buffer position
// return size of contents read
int peekbuf(char **buf);

// read single character in the buffer, without manipulating buffer position
int peekchar();

// truncate all buf contents
void resetbuf();

// skip whitespaces
void skip_whitespace(FILE *fin);

#endif