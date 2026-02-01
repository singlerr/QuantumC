# Contributing

## Contact

We have not yet established an official contact method or project workflow, but we welcome your feedback and contributions.
Please use the [Issues](https://github.com/singlerr/QuantumC/issues) tab for bug reports or suggestions, or submit a pull request via the [Pull Requests](https://github.com/singlerr/QuantumC/pulls) tab.

## Notes

### Workflow

### Compilation

### Networking

The networking component handles dispatching compiled QuantumC circuits to remote quantum backends, running them, and returning results to the host program.
The typical flow for a backend like IBM Quantum is shown below:

```python
from qiskit import QuantumCircuit, generate_preset_pass_manager
from qiskit_ibm_runtime import QiskitRuntimeService, SamplerV2 as Sampler

def run_quantum_circuit(qc: QuantumCircuit, throws: int, token: str, crn: str) -> dict:
    service = QiskitRuntimeService(channel='ibm_quantum_platform', token=token, instance=crn)
    backend = service.backends(simulator=False, operational=True)[0]

    pm = generate_preset_pass_manager(backend=backend, optimization_level=0)
    trans_qc = pm.run(qc)
    
    sampler = Sampler(mode=backend)
    job = sampler.run([trans_qc], shots=2*throws)
    result = job.result()
    counts = result[0].data.meas.get_counts()
    
    return counts
```

When a QuantumC program is executed, the networking layer should perform these steps as the code snippet above: authenticate the user, discover and select a backend, run any required transpilation or optimization, submit the job, monitor execution, and parse the results.
The project aims to support various quantum backend hosts, not limited to IBM.

#### libcurl vs. Qiskit C API

Using libcurl may require more low-level code, but it keeps the networking code self-contained and easier to integrate into different environments while minimizing external dependencies.
We prefer using [libcurl](https://curl.se/libcurl/) for backend communication rather than the prebuilt [Qiskit C](https://quantum.cloud.ibm.com/docs/en/api/qiskit-c) API for the following reasons:

* **Minimal Runtime Dependencies**: libcurl is a portable C library that does not require Python or Rust configured on the host machine.
* **Provider-Agnostic REST Access**: Direct HTTP calls make it easier to support multiple backend hosts and custom provider APIs.
* **Full Control of Requests**: Implementing authentication, error handling, job submission, and result parsing in C gives the user finer control.

## Task Queue

* [ ] Write QuantumC specifications.
    * [ ] [Flows](./specs/flows.md)
    * [ ] [Data Types](./specs/types.md)
    * [ ] [Lexical Structures](./specs/lexical.md)
    * [ ] [Syntax](./specs/syntax.md)
    * [ ] [Semantics](./specs/semantics.md)
* [ ] Verify the QuantumC workflow.
    * [ ] Qiskit C API Documentation
    * [ ] Qiskit Runtime REST API Documentation
    * [ ] OpenQASM Documentation
    * [ ] Cirq Documentation
    * [ ] Q# Documentation
* [ ] Implement printing functions for the main compilers with better graphics.
    * [ ] `print_ast`
    * [ ] `print_sqz`
* [ ] Implement main compilers.
    * [x] Parser
    * [x] AST Generation
    * [x] AST Squeezing
    * [ ] Semantic Analysis
    * [ ] Optimization (optional)
    * [ ] Code Generation
* [ ] Implement networking component of the compiler for job dispatching.
    * [ ] Making Requests
    * [ ] Authentication
    * [ ] Qiskit Transpiler Service
    * [ ] Qiskit Runtime
    * [ ] Result Parsing
    * [ ] Main Compiler Integration
* [ ] Debug memory leaks.
* [ ] Implement testing cases.
* [ ] Build demonstration programs.
    * [ ] Quantum Random Number Generator
    * [ ] Deutsch-Jozsa Algorithm
    * [ ] Genetic Quantum Algorithm
    * [ ] Quantum Approximate Optimization Algorithm
* [ ] Document and maintain the codebase.
    * [ ] QuantumC Compiler Code
    * [ ] QuantumC Networking Code
    * [ ] QuantumC Testing Code
    * [ ] Demonstration Manuals
