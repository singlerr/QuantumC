
# Datatypes

All data types have same structure as Rust does except for the types for Quantum

## Primitive types
- Boolean
  - The boolean type or bool can take one of two values, `true` or `false`
- Numeric
  - Numeric types can have two kinds of types - `integer`, `float` and machine dependent types: `usize`, `isize`
  - `integer` consist of:


| Type | Minimum | Maximum          |
| ---- | ------- | ---------------- |
| u8   | 0       | 2<sup>8</sup>-1  |
| u16  | 0       | 2<sup>16</sup>-1 |
| u32  | 0       | 2<sup>32</sup>-1 |
| u64  | 0       | 2<sup>64</sup>-1 |

| Type | Minimum | Maximum          |
| ---- | ------- | ---------------- |
| i8   | -2^7    | 2<sup>7</sup>-1  |
| u16  | -2^15   | 2<sup>15</sup>-1 |
| u32  | -2^31   | 2<sup>31</sup>-1 |
| u64  | -2^63   | 2<sup>63</sup>-1 |
- Textual
  - `char` and `str`
  - `char` is a Unicode scalar type, represented as a 32-bit unsigned word
  - `str` is the sequence of `u8`, represented the same way as `[u8]`
- Never
  - The never type `!` is a type with no values, representing the result of computations that never complete

## Sequence types
- Tuple
  - Tuple types are a family of structural types for heterogeneous lists of other types
  - The syntax for a tuple type is parenthesized, comma-separated list of types
  - e.g. `(u8,i8)`
- Array
  - An array is a fixed-size sequence of `N` elements of type `T`. The array type is written as `[T; N]`
- Slice
  - A slice is a dynamically sized type representing a view into a sequence of elements of type `T`. The slice type is written as `[T]`

## User-defined types
- Struct
  - A `struct` type is a heterogeneous product of other types, called the fields of the type
- Enum
  - An enumerated type is a nominal, heterogeneous disjoint union type, denoted by the name of an enum item
- Union
  - A union type is a nominal, heterogenous C-like union, denoted by the name of an union item

## Function types
- Functions
  - When referred to, a function item, or the constructor of a tuple-like struct or enum variant, yields a zero-sized value of its function item type
- Closures
  - A closure expression produces a closure value with a unique, anonymous type that cannot be written out. A closure type is approximately equivalent to a struct which contains the captured value

## Pointer types
- References
- Raw pointers
- Function pointers

## Trait types
- Trait objects
- Impl trait

