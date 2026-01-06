#ifndef _SYMREC_H_
#define _SYMREC_H_

#include "ast_types.h"
#include "type.h"

#ifndef PRIM_VOID
#define PRIM_VOID prim_void
#endif

#ifndef PRIM_CHAR
#define PRIM_CHAR prim_char
#endif

#ifndef PRIM_SHORT
#define PRIN_SHORT prim_short
#endif

#ifndef PRIM_INT
#define PRIM_INT prim_int
#endif

#ifndef PRIM_LONG
#define PRIM_LONG prim_long
#endif

#ifndef PRIM_FLOAT
#define PRIM_FLOAT prim_float
#endif

#ifndef PRIM_DOUBLE
#define PRIM_DOUBLE prim_double
#endif

#ifndef PRIM_SIGNED
#define PRIM_SIGNED prim_signed
#endif

#ifndef PRIM_UNSIGNED
#define PRIM_UNSIGNED prim_unsigned
#endif

#ifndef PRIM_COMPLEX
#define PRIM_COMPLEX prim_complex
#endif

#ifndef PRIM_IMAGINARY
#define PRIM_IMAGINARY prim_imaginary
#endif

#ifndef PRIM_QUBIT
#define PRIM_QUBIT prim_qubit
#endif

#ifndef PRIM_ANGLE
#define PRIM_ANGLE prim_angle
#endif

#ifndef PUT_TYPE
#define PUT_TYPE(name, type, size) puttype(name, type, mk_type(name, mk_type_meta(size), 0))
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
extern typerec_t *PRIM_INT;
extern typerec_t *PRIM_LONG;
extern typerec_t *PRIM_FLOAT;
extern typerec_t *PRIM_DOUBLE;
extern typerec_t *PRIM_SIGNED;
extern typerec_t *PRIM_UNSIGNED;
extern typerec_t *PRIM_COMPLEX;
extern typerec_t *PRIM_IMAGINARY;
extern typerec_t *PRIM_QUBIT;
extern typerec_t *PRIM_ANGLE;

symrec_t *putsym(const char *name);
symrec_t *getsym(const char *name);
symrec_t *getorcreatesym(const char *name);

typerec_t *puttype(const char *name, ast_node_type type_type, const type_t *type);
typerec_t *gettype(const char *name);
typerec_t *clone_type_rec(const typerec_t *o);

void init_type();

#endif