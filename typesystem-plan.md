# QuantumC Type System — Implementation Plan

> Hand-off spec for Claude Code. Implement the steps **in order**; build after
> each step and do not proceed until the build is green. This file is the source
> of truth for the type-system rework.

## Goal

Replace the stalled, ad-hoc type checking in QuantumC with a principled design:

1. **Types as a tagged-union ADT** (one discriminant + per-constructor payload).
2. **Hash-consed (interned) types** so type equality is pointer equality.
3. **Bidirectional type checking** (`synth ⇒` / `check ⇐`) instead of the dead
   `infer_*` code.
4. **Affine qubit tracking** via a split context Γ (classical, duplicable) / Δ
   (quantum, usage-tracked), enforcing no-cloning and measurement-consumes.

This is a refactor of an existing flex/bison transpiler that lowers C99+quantum
source to OpenQASM. Preserve existing behavior for the classical path; add the
type discipline incrementally.

## Repo map (relevant files, under `src/transpiler/`)

- `type.h` / `type.c` — `type_t` (currently `name` + `typemeta_t *meta` +
  `next` chain) and `typemeta_t` (flat struct: size, node_type, fields, enums,
  index, args, func). **This is the main thing to redesign.**
- `symrec.h` / `symrec.c` — symbol table (`symrec_t`) + type table
  (`typerec_t`, an intern table) + the `PRIM_*` primitive type records +
  `init_type()`.
- `ast_types.h` — the `ast_node_type` enum, including `AST_TYPE_*` constructors
  (POINTER, ARRAY, STRUCT, UNION, FUNC, QUBIT, BIT, ANGLE, …). Reuse these as
  the type discriminant.
- `ast_typing.h` / `ast_typing.c` — structural compatibility predicates
  (`type_equals`, `is_type_compatible`, `is_casting_compatible`,
  `is_param_compatible`) and the big `IS_*` macro block. These become the
  subsumption check at mode-switch points.
- `ast_sem.c` (~1587 lines) — semantic/lowering pass from the `sqz_*` parse AST
  to the target IR. **Lines ~1222–1584 are a fully commented-out `infer_*`
  family** — that is the stalled attempt to delete and replace.
- `builtin_gate.c` — `apply_X/Y/Z/S/T/H/CNOT/CZ/CX/CCX/RX/RY/RZ` via the
  `BUILTIN_GATE` macro; each builds an `EXPR_QUANTUM_GATE` over a `qubit_list`.
- `builtin_measure.c` — `measure(T, q)` builds an `EXPR_QUANTUM_MEASUREMENT`.
- `parser.y` / `scanner.l` — front end (gates/measure are NOT grammar; they are
  builtin function calls dispatched in the semantic pass).

## Build & verify

```sh
cd src/transpiler
make            # dev build (-g, debug parser/lexer)
make PROD=1     # -Werror -Wall — must pass before declaring a step done
```

Smoke tests: `tests/test_1.c`, `demos/random/random.c`, and the 8-qubit Bell
example in `README.md`. After the quantum step, add negative tests (below).

---

## Target design

### Layer 1 — Types as a tagged union

Redefine the core type representation. Reuse `ast_node_type`'s `AST_TYPE_*`
members as the discriminant (or introduce a dedicated `type_kind`; keep one
source of truth). Replace the flat `typemeta_t` + `next` chain with explicit
per-constructor payloads:

```c
typedef struct type type;

typedef struct field { const char *name; type *ty; struct field *next; } field;
typedef struct param { type *ty;          struct param *next; } param;

struct type {
    ast_node_type kind;          // AST_TYPE_INT / _POINTER / _ARRAY / _QUBIT ...
    union {
        /* base types (int, float, qubit, bit, angle, void, ...) carry nothing */
        struct { type *elem; }               ptr;     // τ*
        struct { type *elem; long n; }       array;   // τ[n], n is a literal
        struct { field *fields; }            record;  // struct / union
        struct { type *ret; param *params; } func;    // (τ…) → τ
        struct { type *inner; }              bang;    // !τ  (duplicable; optional)
        struct { const char *name; type *def; } named;// typedef / nominal struct
    } as;
};
```

Notes:
- Keep `qubit[n]`'s `n` as a non-negative integer **literal** for now (per
  `specs/types.md`). Do not attempt dependent sizes yet.
- `bang` / `!τ` is optional scaffolding; do not wire it in until a concrete need
  appears (see Layer 3). Leave the case present but unused.
- Migrate `type.c` constructors (`mk_type`, `clone_type`, …) and every
  `IS_*`/`*_equals`/`*_compatible` site in `ast_typing.c` to the new shape. The
  `next`-chain walks in `pointer_equals`/`is_pointer_compatible`/
  `is_array_compatible` become `as.ptr.elem` / `as.array.elem` recursion.

### Layer 2 — Hash-consing / interning

Add `type *intern_type(type *candidate)` backed by the existing
`type_table` (`typerec_t`). It returns the canonical pointer for a
structurally-equal type, registering `candidate` if none exists. All type
construction goes through it. Then:

```c
// after interning, structural equality collapses to identity:
#define type_equals(a, b)  ((a) == (b))
```

Keep a slow structural comparator only for the intern-table bucket check.
Nominal types compare by `as.named.name`; follow `as.named.def` only when a
structural view is explicitly needed.

### Layer 3 — Bidirectional checking (replaces `infer_*`)

Delete the commented `infer_*` block in `ast_sem.c`. Add two mutually recursive
functions over the expression IR:

```c
typedef struct context context;   // see Layer 4

// synthesis (⇒): compute a type; may update ctx->delta usage
type *synth_expr(context *ctx, expression *e);
// checking (⇐): verify e against an expected type
bool  check_expr(context *ctx, expression *e, type *expected);
```

Mode discipline (Dunfield–Krishnaswami):
- **Synthesize**: identifiers, literals, function calls, array indexing, member
  access, casts (cast supplies its own target type).
- **Check**: variable-declaration initializers, assignment RHS, call arguments
  against parameter types, `return` against the function's return type.
- The single mode-switch rule: `check(ctx, e, τ)` = let `τ' = synth(ctx, e)` in
  `is_type_compatible(τ', τ)` (this is subsumption — reuse the existing
  predicate). Insert an explicit cast node here when `τ' ≠ τ` but compatible
  (see classical conversions below).

Classical C conversions (integer promotion, usual arithmetic conversions,
array→pointer decay): handle inside `synth`/`check` and **insert explicit cast
nodes into the IR** (CompCert/Clight style) so `codegen.c` never has to infer a
conversion. `is_casting_compatible` is the seed for the legality check.

### Layer 4 — Affine qubit context

Split the typing context. Linearity is a property of the **binding/usage**, not
the type, so the `consumed` flag lives on the binding:

```c
typedef struct binding {
    symrec_t *sym;
    type *ty;
    int  scope_level;        // existing get_scope_level()
    bool consumed;           // affine: set on measure/reset/move
    struct binding *next;
} binding;

struct context {
    binding *gamma;          // classical: duplicable, free use
    binding *delta;          // quantum: affine, usage-tracked
};
```

Rules to enforce:
- **Gate application** (`builtin_gate.c`, e.g. `apply_CX(a, b)`): every argument
  must resolve to a qubit binding in Δ that is not `consumed`; the argument
  bindings must be **pairwise distinct** (reject aliasing like
  `apply_CX(q, q)` — that is cloning). A gate does **not** consume: leave
  `consumed` false.
- **Measurement** (`builtin_measure.c`, `measure(T, q)`): `q` must be a
  non-consumed qubit binding in Δ; set `consumed = true`; the result type is
  the classical `T`. This resolves the `// FIXME: How to handle measure?` in
  `convert_variable_declaration` (`ast_sem.c`).
- **Classical values** always live in Γ and are freely duplicable.

Control-flow merge (important):
- `if/else`: snapshot Δ before the branches, type-check each arm from the
  snapshot, then require both arms to have consumed the **same** qubit set;
  reject divergent consumption. Provide `context_snapshot(ctx)` (copy the
  binding lists; types are interned so they are shared) and
  `context_join(a, b)`.
- Loops: a qubit consumed inside a body that may iterate more than once is a
  linearity error (it would consume twice). Detect and reject.

---

## Implementation order (each step must build clean)

1. **Layer 1 first.** Introduce the tagged-union `type` and migrate
   `type.c` + all `ast_typing.c` sites and any `ast_sem.c` users. No behavior
   change intended; existing tests still pass.
2. **Layer 2.** Add `intern_type`, route all construction through it, collapse
   `type_equals` to pointer identity. Verify tests still pass.
3. **Layer 3, classical only.** Add `synth_expr`/`check_expr`, delete the dead
   `infer_*` block, wire literals/identifiers/binary/cast/calls, and insert
   explicit conversion casts. Validate against `tests/test_1.c` and the random
   demo.
4. **Layer 4.** Add `context` with Γ/Δ, thread it through `synth`/`check`,
   enforce gate/measure rules, then add branch/loop merge. Add negative tests:
   - reuse after measure → error
   - `apply_CX(q, q)` aliasing → error
   - qubit consumed in a loop body → error
   - divergent consumption across if/else arms → error
5. **(Optional, later)** `!τ` duplicable wrapper and `qubit[n]` dependent sizes
   (LICS-2020 direction). Only if needed.

## Constraints

- Stay C99; `make PROD=1` (`-Werror -Wall`) must pass.
- Do not change classical C semantics (per `specs/types.md`).
- Keep changes incremental and reviewable; commit per step.
- Update `specs/types.md` with the typing judgments as they land.

## References (papers; PDFs available from the project owner)

- Bidirectional checking: Dunfield & Krishnaswami, *Bidirectional Typing*,
  ACM Computing Surveys 2022; foundational: Pierce & Turner, *Local Type
  Inference*, TOPLAS 2000.
- Classical C conversions / explicit casts: Blazy & Leroy, *Mechanized
  Semantics for the Clight Subset of C*; Norrish, *C formalised in HOL*
  (Cholera).
- Quantum affine typing / no-cloning: Selinger & Valiron, *A Lambda Calculus
  for Quantum Computation with Classical Control* (2006); Fu, Kishida, Ross &
  Selinger, *Linear Dependent Type Theory for Quantum Programming Languages*,
  LICS 2020.
