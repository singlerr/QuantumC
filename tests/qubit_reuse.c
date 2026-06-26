/* negative test: reuse after measure should error */
void test_fn(void) {
    qubit q;
    measure(q);
    measure(q);
}
