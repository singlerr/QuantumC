
# Datatypes

## Primitive types
- Boolean
  - The boolean type or bool can take one of two values, `true` or `false`
- Numeric
  - Numeric types can have two kinds of types - `integer`, `float` and machine dependent types: `usize`, `isize`
  - `integer` consist of:

| Type | Minimum | Maximum |
| ---- | ------- | ------- |
| u8   | 0       | 2^8-1   |
| u16  | 0       | 2^16-1  |
| u32  | 0       | 2^32-1  |
| u64  | 0       | 2^64-1  |

| Type | Minimum | Maximum |
| ---- | ------- | ------- |
| i8   | -2^7    | 2^7-1   |
| u16  | -2^15   | 2^15-1  |
| u32  | -2^31   | 2^31-1  |
| u64  | -2^63   | 2^63-1  |
- Textual
  - `char` and `str`
  - `char` is a Unicode scalar type, represented as a 32-bit unsigned word
  - `str` is the sequence of `u8`, represented the same way as `[u8]`
- Never

## Sequence types
- Tuple
- Array
- Slice

## User-defined types
- Struct
- Enum
- Union

## Function types
- Functions
- Closures

## Pointer types
- References
- Raw pointers
- Function pointers

## Trait types
- Trait objects
- Impl trait

