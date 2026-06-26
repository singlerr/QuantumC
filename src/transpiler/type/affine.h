#ifndef _AFFINE_H_
#define _AFFINE_H_

#include "check.h"

struct ast;

/* Global affine Δ context driven by grammar actions. */
void affine_fn_enter (void);
void affine_fn_exit (void);
void affine_snap_push (void);          /* snapshot before if-body or loop-body */
void affine_if_noelse (void);          /* if without else: no new consumption */
void affine_prep_else (void);          /* before else-branch: save post-then, restore pre */
void affine_join (void);               /* after else-branch: check & merge */
void affine_loop_push (void);          /* snapshot before loop body */
void affine_loop_pop (void);           /* after loop: check no new consumption */
void affine_check_call (const char *fname, struct ast *arglist);

#endif
