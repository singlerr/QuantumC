# Quantum Random Number Generator

## Algorithm Description

This quantum algorithm generates a uniformly distributed random number.

### Generating An Integer $n \in [a, b)$

$$
\begin{equation}
    n = a + \left(\sum_{i=0}^{n-1} 2^ib_i \mod (b-a)\right)
\end{equation}
$$

### Generating A Floating-Point $f \in [a, b]$

$$
\begin{equation}
    f = a + \frac{b-a}{2^n-1} \sum_{i=0}^{n-1} 2^ib_i
\end{equation}
$$

## Quantum Circuit Represenation