typedef unsigned int uint;
uint8_t eight_qubit_bell_state()
{
    qubit q[8];

    apply_H(q[0]);
    for (int i = 0; i < 7; i++)
    {
        apply_CX(q[i], q[i + 1]);
    }

    uint8_t meas = measure(q);

    // Expected to return 0b00000000 == 0 in 50% and 0b11111111 == 255 in 50%.
    return meas;
}
