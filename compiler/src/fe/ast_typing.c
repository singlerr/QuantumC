#include "ast_typing.h"
#include "ast_sqz.h"
#include "ast_sem.h"
#include "symrec.h"
#include "diagnostics.h"
#include <string.h>

BOOL is_pointer_compatible(const type_t *a, const type_t *b);
BOOL is_struct_compatible(const type_t *a, const type_t *b);
BOOL is_array_compatible(const type_t *a, const type_t *b);
BOOL is_func_compatible(const type_t *a, const type_t *b);
BOOL is_spec_compatible(const struct _sqz_decl_spec *a, const struct _sqz_decl_spec *b);
BOOL pointer_equals(const type_t *a, const type_t *b);
BOOL _is_spec_compatible(const struct sem_decl_spec *a, const struct sem_decl_spec *b);

BOOL type_equals(const type_t *a, const type_t *b)
{
    if (!a || !b)
    {
        return FALSE;
    }

    if (a == b)
    {
        return TRUE;
    }

    if (!SHALLOW_TYPE_EQUAL(a, b))
    {
        return FALSE;
    }

    if (IS_PTR(a) || IS_ARRAY(a))
    {
        return pointer_equals(a, b);
    }

    if (IS_STRUCT(a) && IS_STRUCT(b))
    {
        return is_struct_compatible(a, b);
    }

    if (IS_FUNC(a) && IS_FUNC(b))
    {
        return is_func_compatible(a, b);
    }

    return TRUE;
}

BOOL pointer_equals(const type_t *a, const type_t *b)
{
    if (!a || !b)
    {
        return FALSE;
    }

    if (!a->next || !b->next)
    {
        return FALSE;
    }

    if (!type_equals(a->next, b->next))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL is_pointer_compatible(const type_t *a, const type_t *b)
{
    if (!a || !b)
    {
        return FALSE;
    }

    if (!a->next || !b->next)
    {
        return FALSE;
    }

    if (!is_type_compatible(a->next, b->next))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL is_struct_compatible(const type_t *a, const type_t *b)
{
    if (a == b)
    {
        return TRUE;
    }

    if (!IS_STRUCT(a) || !IS_STRUCT(b))
    {
        return FALSE;
    }

    if (!a->next || !b->next)
    {
        return FALSE;
    }

    typemeta_t *a_meta = a->next->meta;
    typemeta_t *b_meta = b->next->meta;

    if (!a_meta || !b_meta)
    {
        return FALSE;
    }

    struct _sqz_struct_decl *a_field = a_meta->fields;
    struct _sqz_struct_decl *b_field = b_meta->fields;

    while (TRUE)
    {
        if (IS_INCL_NULL(a_field, b_field))
        {
            if (IS_EXCL_NULL(a_field, b_field))
            {
                return FALSE;
            }
            break;
        }

        struct _sqz_struct_field_decl *a_field_decl = a_field->field;
        struct _sqz_struct_field_decl *b_field_decl = b_field->field;

        while (TRUE)
        {
            if (IS_INCL_NULL(a_field_decl, b_field_decl))
            {
                if (IS_EXCL_NULL(a_field_decl, b_field_decl))
                {
                    return FALSE;
                }
                break;
            }

            struct _sqz_struct_field *a_decl = a_field_decl->decl_list;
            struct _sqz_struct_field *b_decl = b_field_decl->decl_list;

            while (TRUE)
            {
                if (IS_INCL_NULL(a_decl, b_decl))
                {
                    if (IS_EXCL_NULL(a_decl, b_decl))
                    {
                        return FALSE;
                    }
                    break;
                }

                type_t *a_decl_type = (a_decl->decl && a_decl->decl->type) ? a_decl->decl->type : a_field_decl->type;
                type_t *b_decl_type = (b_decl->decl && b_decl->decl->type) ? b_decl->decl->type : b_field_decl->type;

                if (!is_type_compatible(a_decl_type, b_decl_type))
                {
                    return FALSE;
                }

                a_decl = a_decl->next;
                b_decl = b_decl->next;

                if (IS_EXCL_NULL(a_decl, b_decl))
                {
                    return FALSE;
                }
            }

            a_field_decl = a_field_decl->next;
            b_field_decl = b_field_decl->next;

            if (IS_EXCL_NULL(a_field_decl, b_field_decl))
            {
                return FALSE;
            }
        }

        a_field = a_field->next;
        b_field = b_field->next;
    }

    return TRUE;
}

BOOL is_array_compatible(const type_t *a, const type_t *b)
{
    if (!a || !b)
    {
        return FALSE;
    }

    if (!a->next || !b->next)
    {
        return FALSE;
    }

    return is_type_compatible(a->next, b->next);
}

BOOL is_casting_compatible(const type_t *caster, const type_t *castee)
{
    if (SHALLOW_TYPE_EQUAL(caster, castee))
    {
        return TRUE;
    }

    if (IS_PTR(caster))
    {

        return TRUE;
    }

    if (IS_SCALAR(caster) && IS_SCALAR(castee))
    {
        return TRUE;
    }

    if (is_struct_compatible(caster, castee))
    {
        return TRUE;
    }

    return is_type_compatible(caster, castee);
}

BOOL is_func_compatible(const type_t *a, const type_t *b)
{
    if (!a || !b)
    {
        return FALSE;
    }

    if (!a->meta || !b->meta)
    {
        return FALSE;
    }

    if (!a->meta->func || !b->meta->func)
    {
        return FALSE;
    }

    struct _sqz_func_decl *a_func = a->meta->func;
    struct _sqz_func_decl *b_func = b->meta->func;

    if (!is_type_compatible(a_func->return_type, b_func->return_type))
    {
        return FALSE;
    }

    if (!is_args_compatible(a_func->params, b_func->params))
    {
        return FALSE;
    }

    if (!SPEC_EQUAL(a_func->spec, b_func->spec))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL is_type_compatible(const type_t *left, const type_t *right)
{

    if (IS_PTR(left) && IS_PTR(right))
    {
        return is_pointer_compatible(left, right);
    }
    else if (!(!IS_PTR(left) && !IS_PTR(right)))
    {
        return FALSE;
    }

    if (IS_SCALAR(left) && IS_SCALAR(right))
    {
        return TRUE;
    }

    if (IS_ARRAY(left) && IS_ARRAY(right))
    {
        return is_array_compatible(left, right);
    }

    if (IS_STRUCT(left) && IS_STRUCT(right))
    {
        return is_struct_compatible(left, right);
    }

    if (IS_FUNC(left) && IS_FUNC(right))
    {
        return is_func_compatible(left, right);
    }

    if (SHALLOW_TYPE_EQUAL(left, right))
    {
        return TRUE;
    }

    return FALSE;
}

BOOL is_param_compatible(const struct sem_args *a, const struct sem_args *b)
{
    struct sem_param_decl *a_param = NULL;
    struct sem_param_decl *b_param = NULL;
    struct sem_declarator *a_decl = NULL;
    struct sem_declarator *b_decl = NULL;
    while (TRUE)
    {
        if (IS_INCL_NULL(a, b))
        {
            if (IS_EXCL_NULL(a, b))
            {
                return FALSE;
            }
            break;
        }

        a_param = a->arg;
        b_param = b->arg;

        if (!is_type_compatible(a_param->type, b_param->type))
        {
            return FALSE;
        }

        if (!_is_spec_compatible(a_param->spec, b_param->spec))
        {
            return FALSE;
        }

        a_decl = a_param->decl;
        b_decl = b_param->decl;

        while (TRUE)
        {
            if (IS_INCL_NULL(a_decl, b_decl))
            {
                if (IS_EXCL_NULL(a_decl, b_decl))
                {
                    return FALSE;
                }

                break;
            }

            if (!is_type_compatible(a_decl->type, b_decl->type))
            {
                return FALSE;
            }

            a_decl = a_decl->next;
            b_decl = b_decl->next;
        }

        a = a->next;
        b = b->next;
    }

    return TRUE;
}

BOOL is_args_compatible(const sqz_args *a, const sqz_args *b)
{
    sqz_param_decl *a_param = NULL;
    sqz_param_decl *b_param = NULL;
    sqz_declarator *a_decl = NULL;
    sqz_declarator *b_decl = NULL;
    while (TRUE)
    {
        if (IS_INCL_NULL(a, b))
        {
            if (IS_EXCL_NULL(a, b))
            {
                return FALSE;
            }
            break;
        }

        a_param = a->arg;
        b_param = b->arg;

        if (!is_type_compatible(a_param->type, b_param->type))
        {
            return FALSE;
        }

        if (!is_spec_compatible(a_param->spec, b_param->spec))
        {
            return FALSE;
        }

        a_decl = a_param->decl;
        b_decl = b_param->decl;

        while (TRUE)
        {
            if (IS_INCL_NULL(a_decl, b_decl))
            {
                if (IS_EXCL_NULL(a_decl, b_decl))
                {
                    return FALSE;
                }

                break;
            }

            if (!is_type_compatible(a_decl->type, b_decl->type))
            {
                return FALSE;
            }

            a_decl = a_decl->next;
            b_decl = b_decl->next;
        }

        a = a->next;
        b = b->next;
    }

    return TRUE;
}

BOOL is_spec_compatible(const struct _sqz_decl_spec *a, const struct _sqz_decl_spec *b)
{
    if (!a || !b)
    {
        return FALSE;
    }

    return a->qualifier == b->qualifier && a->storage_class == b->storage_class;
}

BOOL _is_spec_compatible(const struct sem_decl_spec *a, const struct sem_decl_spec *b)
{
    if (!a || !b)
    {
        return FALSE;
    }

    return a->qualifier == b->qualifier && a->storage_class == b->storage_class;
}