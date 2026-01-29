#ifndef _STRING_BUILDER_H_
#define _STRING_BUILDER_H_

struct string_builder {
    int size;
    char* buffer;
};
void init_str_builder(struct string_builder* builder);
void str_append(struct string_builder* builder, const char* str);
void int_append(struct string_builder* builder, int i);
void float_append(struct string_builder* builder, float f);
char* end_str_builder(struct string_builder* builder);


#endif
