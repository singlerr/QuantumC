/* negative test: apply_CX(q, q) aliasing should error */
void test_fn(void) {
    qubit q;
    apply_CX(q, q);
}
