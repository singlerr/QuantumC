/* negative test: consuming qubit inside loop body should error */
void test_fn(void) {
    qubit q;
    int i;
    for (i = 0; i < 3; i++) {
        measure(q);
    }
}
