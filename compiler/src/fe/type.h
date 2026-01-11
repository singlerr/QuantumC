#ifndef _TYPE_H_
#define _TYPE_H_

struct _sqz_struct_field_decl;
struct _sqz_assign_expr;
struct _sqz_args;
struct _sqz_spec_qual;
struct _sqz_func_decl;
enum _ast_node_type;

typedef struct _typemeta
{
    int size;
    enum _ast_node_type node_type;
    struct _sqz_struct_decl *fields;
    struct _sqz_assign_expr *index;
    struct _sqz_args *args;
    struct _sqz_func_decl *func;
} typemeta_t;

typedef struct _type
{
    char *name;
    typemeta_t *meta;
    struct _type *next;
} type_t;

typemeta_t *mk_type_meta(int size);
typemeta_t *clone_type_meta(const typemeta_t *o);
type_t *mk_type(const char *name, const typemeta_t *meta, type_t *link);
type_t *clone_type(const type_t *o);

#endif