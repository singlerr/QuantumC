#ifndef _PREPROCESSOR_H_
#define _PREPROCESSOR_H_

struct directive_define
{
};

void begin_if(int val);
void begin_ifdef(int val);
void begin_ifndef(int val);

#endif