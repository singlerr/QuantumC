# Data Types

## Classical Data Types (Native C99)

**QuantumC** is C99-compatible for classical (non-quantum) parts of the language.

Summary:
* All standard C99 scalar types are available (e.g., `int`, `unsigned`, `float`, `char`, etc.).
    * Fixed-width integer types from `<inttypes.h>` and `<stdint.h>` are supported (e.g., `uint8_t`, `uint16_t`, `uint32_t`, etc.).
* Pointers, arrays, and structs behave as in standard C99 for classical data.

Notes:
* For the details of classical C99, please refer to [ISO/IEC 9899:TC3](https://www.open-std.org/JTC1/SC22/WG14/www/docs/n1256.pdf) document.
* It is highly recommended to use fixed-width types, especially when you are packing measurement results or interfacing with quantum simulators/backends.
* **QuantumC** does not change the semantics of classical types. It extends C with quantum-specific types and builtins described below.

Example (Classical):
```c
int counter = 0;
uint8_t measured = 0b11110000;
float probability = 0.5;
```

## Quantum Data Types

**QuantumC** introduces first-class quantum data types that represent qubits and collections of qubits.
These types are distinct from classical types and map to OpenQASM constructs when the compiler emits Quantum Intermediate Representation (QIR).

Main Quantum Types:
* `qubit`: Represent a single qubit on a quantum computer.
* `qubit[n]`: Fixed-size array of `n` qubits. The `n` must be a positive integer literal.
* `bit` (Classical): Represents a single classical bit produced by a measurement from `qubit` data type.
* `bit[n]` (Classical): Fixed-size array of classical bits produced by a measurement from `qubit[n]` data type. In source, they behave as classical storage for measurement results; when emitted to OpenQASM they map to `bit` arrays.

Measurement:
* Measurement convert quantum state into classical data.
* `measure(T, q)` where `T` is a classical return type, and `q` is a qubit or `qubit[n]`.
* The function returns a value of type `T` that encodes the measurement outcome(s).
