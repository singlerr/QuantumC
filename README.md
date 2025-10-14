# QuantumC

## Project Description

**QuantumC** is a C99 compatible programming language designed for quantum computing, accompanied by a compiler that translates C code to OpenQASM.
As the [International Year of Quantum Science and Technology](https://quantum2025.org) unfolds, quantum computing and quantum information have seen remarkable breakthroughs.
Quantum computers are now more accessible than ever, enabling individuals with programming experience to execute quantum circuits on real quantum hardware and obtain results.

While Python’s straightforward syntax has made it the language of choice for many quantum computing tools (e.g., [Qiskit](https://www.ibm.com/quantum/qiskit) and [Cirq](https://quantumai.google/cirq)), optimal quantum speedup requires efficient post-processing of quantum data.
The lightweight nature of the C programming language, combined with its tenacious community, makes it an excellent candidate for circuit construction and classical post-processing.
The **QuantumC** project aims to demonstrate C's clarity as a quantum programming language.

### C and OpenQASM

Python is widely used for application development due to its ease of learning, but its runtime performance can be dawdling, especially for iterative tesks.
As quantum hardware advances, the volume of classical data to process will increase, making performance a critical factor.
[OpenQASM](https://openqasm.com), managed by IBM, is an open-source quantum intermediate representation for describing quantum circuits.
Quantum hardware architectures vary, necessitating the compilation of theoretical circuits into machine-specific instructions. 
For instance, Hadamard gates $H$ are converted as $\left(R_Z\left(\frac{\pi}{2}\right)\right) \left(\sqrt{X}\right) \left(R_Z\left(\frac{\pi}{2}\right)\right)$ in IBM Eagle r3 architecture.
OpenQASM provides a universal abstraction, enabling theoretical quantum algorithms to be compiled for diverse hardware.
**QuantumC** leverages the strengths of both C and OpenQASM to facilitate efficient quantum programming.


## Example

Consider the creation of a Bell state, which demonstrates entanglement between two qubits: $\Ket{\Phi^+} = \frac{1}{\sqrt{2}}\ket{00} + \frac{1}{\sqrt{2}}\ket{11}$.
To generate this state, apply a Hadamard gate ($H$) to `qubit 0`, followed by a controlled-NOT gate ($\text{CNOT}$) with `qubit 0` as the control and `qubit 1` as the target, performing $\text{CNOT}_{(0, 1)} (H \otimes I) \ket{00}$ on the circuit.
A sample **QuantumC** program and its corresponding OpenQASM output are shown below.

### QuantumC Example

```c
// Example QuantumC code will be provided here.
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

* [`specs/`](./specs): Contains the specifications for the **QuantumC** language.
* [`compiler/`](./compiler): Contains the source code for the QuantumC compiler.


## Compilation

### Required Tools

This project is built with GCC and Make and uses Flex and Bison to generate the lexer and parser.
You will need a POSIX-like environment (Linux, macOS, or Windows Subsystem for Linux) with the following tools installed:
* `gcc` (C compiler), 
* `make` (building tool), 
* `flex` (lexer generator), and
* `bison` (parser generator).

### Recommended Installation Commands by Platforms

* Debian / Ubuntu:
```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y build-essential flex bison
```
Note: The package `build-essential` includes `gcc`, `make` and other basic build tools.

* Fedora / RHEL (DNF):
```bash
sudo dnf install -y gcc make flex bison
```
* Windows: use WSL (recommended) or an MSYS2 environment. In WSL (Ubuntu) run the Debian/Ubuntu commands above. If using native Windows toolchains, ensure `flex`/`bison` are available (MSYS2 packages or binaries).

### Building Steps

1. Open a terminal in the project root or the frontend directory `compiler/src/fe`.
2. Run `make` (the default target produces the `main` executable):
```bash
make
```
3. To remove generated files and object files:
```bash
make clean
```

### Troubleshooting

* If `make` fails with `bison: command not found` or `flex: command not found`, install those packages or run the build in WSL/macOS where the tools are available.
* If you see undefined reference errors at link time, check whether any required libraries are missing or whether object files failed to compile earlier in the output.


## Contribution

We have not yet established an official contact method or project workflow, but we welcome your feedback and contributions.
Please use the [Issues](https://github.com/singlerr/QuantumC/issues) tab for bug reports or suggestions, or submit a pull request via the [Pull Requests](https://github.com/singlerr/qRQuantumCust/pulls) tab.


## License

MIT License.
