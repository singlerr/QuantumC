namespace SingleQubit {
    open Microsoft.Quantum.Intrinsic;

    @EntryPoint()
    operation Hadamard() : Int {
        use q = Qubit();
        H(q);

        let meas = M(q);

        Reset(q);

        return if (meas == One) {return 1} else {return 0};
    }
}
