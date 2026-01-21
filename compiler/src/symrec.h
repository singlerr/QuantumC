#ifndef _SYMREC_H_
#define _SYMREC_H_

#include "ast_types.h"
#include "type.h"

#ifndef PRIM_TYPES

#define PRIM_VOID prim_void
#define PRIM_CHAR prim_char
#define PRIN_SHORT prim_short
#define PRIM_INT32 prim_int32
#define PRIM_INT64 prim_int64
#define PRIM_INT PRIM_INT32
#define PRIM_LONG prim_long
#define PRIM_FLOAT32 prim_float32
#define PRIM_FLOAT64 prim_float64
#define PRIM_FLOAT prim_float32
#define PRIM_DOUBLE prim_double
#define PRIM_SIGNED prim_signed
#define PRIM_UNSIGNED prim_unsigned
#define PRIM_COMPLEX prim_complex
#define PRIM_IMAGINARY prim_imaginary
#define PRIM_QUBIT prim_qubit
#define PRIM_ANGLE prim_angle
#define PRIM_DURATION prim_duration
#define PRIM_STRING prim_string
#define PRIM_BIT prim_bit
#define PRIM_BOOL prim_bool

#endif

#ifndef PUT_TYPE
#define PUT_TYPE(name, type, size) puttype(name, type, mk_type(name, mk_type_meta(size), 0))
#endif

#ifndef PUT_SIZED_TYPE
#define PUT_SIZED_TYPE(name, type, size) putsizedtype(name, type, mk_type(name, mk_type_meta(size), 0))
#endif

typedef struct _symrec
{
    char *name;
    struct _symrec *next;
} symrec_t;

typedef struct _typerec
{
    char *name;
    ast_node_type type_type;
    type_t *handle;
    struct _typerec *next;
} typerec_t;

extern symrec_t *sym_table;
extern typerec_t *type_table;

extern typerec_t *PRIM_VOID;
extern typerec_t *PRIM_CHAR;
extern typerec_t *PRIM_SHORT;
extern typerec_t *PRIM_INT32;
extern typerec_t *PRIM_INT64;
extern typerec_t *PRIM_LONG;
extern typerec_t *PRIM_FLOAT32;
extern typerec_t *PRIM_FLOAT64;
extern typerec_t *PRIM_DOUBLE;
extern typerec_t *PRIM_SIGNED;
extern typerec_t *PRIM_UNSIGNED;
extern typerec_t *PRIM_COMPLEX;
extern typerec_t *PRIM_IMAGINARY;
extern typerec_t *PRIM_QUBIT;
extern typerec_t *PRIM_ANGLE;
extern typerec_t *PRIM_STRING;
extern typerec_t *PRIM_DURATION;
extern typerec_t *PRIM_BIT;
extern typerec_t *PRIM_BOOL;

symrec_t *putsym(const char *name);
symrec_t *getsym(const char *name);
symrec_t *getorcreatesym(const char *name);

typerec_t *puttype(const char *name, ast_node_type type_type, const type_t *type);
typerec_t *putsizedtype(const char *name, ast_node_type type_type, const type_t *type);
typerec_t *gettype(const char *name);
typerec_t *getsizedtype(const char *name, int size);
typerec_t *clone_type_rec(const typerec_t *o);

void init_type();

#endif