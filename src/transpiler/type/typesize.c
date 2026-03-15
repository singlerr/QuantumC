#include "../err.h"
#include "base.h"
#include "param.h"

int
type_sizeof (type_t *ty)
{
  if (!ty)
    {
      error ("Type is null");
      return -1;
    }
  switch (ty->tag)
    {
    case TY_STRUCT:
      {
        struct_field_t *it;
        int struct_size = 0;
        if (ty->size != -1)
          {
            return ty->size;
          }

        ty_struct_t s = ty->ty.ty_struct;
        for (it = cvector_begin (s.fields); it != cvector_end (s.fields); it++)
          {
            int sz = type_sizeof (it->ty->ty);
            struct_size += sz;
          }

        ty->size = struct_size;
        return struct_size;
      }
      break;
    case TY_UNION:
      {
        struct_field_t *it;
        int union_size = 0;
        if (ty->size != -1)
          {
            return ty->size;
          }

        ty_struct_t s = ty->ty.ty_struct;
        for (it = cvector_begin (s.fields); it != cvector_end (s.fields); it++)
          {
            int sz = type_sizeof (it->ty->ty);
            if (sz > union_size)
              {
                union_size = sz;
              }
          }

        return union_size;
      }
      break;
    default:
      return ty->size;
    }

  return -1;
}
