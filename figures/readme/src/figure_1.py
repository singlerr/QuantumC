from qiskit import QuantumCircuit


def main() -> None:
    qc = QuantumCircuit(8)

    qc.h(0)
    for i in range(7):
        qc.cx(i, i+1)
    qc.measure_all()

    style = {'name': 'iqp-dark'}
    qc.draw(output='mpl', filename='../readme_figure_1.png', scale=0.5, style=style)

    return


if __name__ == "__main__":
    main()
