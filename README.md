# qRust

## Project Description

**qRust** is a Rust-inspired programming language designed for quantum computing, accompanied by a compiler that translates qRust code to OpenQASM.
As the [International Year of Quantum Science and Technology](https://quantum2025.org) unfolds, quantum computing and quantum information have seen remarkable breakthroughs.
Quantum computers are now more accessible than ever, enabling individuals with programming experience to execute quantum circuits on real quantum hardware and obtain results.

While Python’s straightforward syntax has made it the language of choice for many quantum computing tools (e.g., [Qiskit](https://www.ibm.com/quantum/qiskit) and [Cirq](https://quantumai.google/cirq)), optimal quantum speedup requires efficient post-processing of quantum data.
Rust’s lightweight yet rigorous nature, combined with its supportive community, makes it an excellent candidate for circuit construction and classical post-processing.
The **qRust** project aims to demonstrate Rust’s suitability as a quantum programming language.

### Rust and OpenQASM

Python is widely used for application development due to its ease of learning, but its runtime performance can be dawdling, especially for iterative tesks.
As quantum hardware advances, the volume of classical data to process will increase, making performance a critical factor.
[Rust](https://www.rust-lang.org) offers safe memory management, high performance, and zero-cost abstractions, promising users to both construct quantum circuits and process results within a single language.

[OpenQASM](https://openqasm.com), managed by IBM, is an open-source quantum intermediate representation for describing quantum circuits.
Quantum hardware architectures vary, necessitating the compilation of theoretical circuits into machine-specific instructions. 
For instance, Hadamard gates $H$ are converted as $(R_Z(\pi/2)) (\sqrt{X}) (R_Z(\pi/2))$ in IBM Eagle r3 architecture.
OpenQASM provides a universal abstraction, enabling theoretical quantum algorithms to be compiled for diverse hardware.
**qRust** leverages the strengths of both Rust and OpenQASM to facilitate efficient quantum programming.


## Example

Consider the creation of a Bell state, which demonstrates entanglement between two qubits: $\ket{\Phi^+} = \frac{1}{\sqrt{2}}\ket{00} + \frac{1}{\sqrt{2}}\ket{11}$.
To generate this state, apply a Hadamard gate ($H$) to `qubit 0`, followed by a controlled-NOT gate ($\text{CNOT}$) with `qubit 0` as the control and `qubit 1` as the target, performing $\text{CNOT}_{(0, 1)} (H \otimes I) \ket{00}$ on the circuit.
A sample **qRust** program and its corresponding OpenQASM output are shown below.

### qRust Example

```rust
// Example qRust code will be provided here.
```

### OpenQASM Example

```qasm
OPENQASM 3;
include "stdgates.inc";

qubit[2] q;
bit[2] c;

h q[0];
cnot q[0], q[1];

c = measure q;
```


## Quick Start

*Documentation for setup and usage will be provided as the project evolves.*


## Directory Structure

* [`specs/`](./specs): Contains the specifications for the **qRust** language.


## Contribution

We have not yet established an official contact method or project workflow, but we welcome your feedback and contributions.
Please use the [Issues](https://github.com/singlerr/qRust/issues) tab for bug reports or suggestions, or submit a pull request via the [Pull Requests](https://github.com/singlerr/qRust/pulls) tab.


## License

MIT License.
