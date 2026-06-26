/* negative test: divergent consumption across if/else arms should error */
void test_fn(void) {
    qubit q;
    qubit r;
    if (1) {
        measure(q);
    } else {
        measure(r);
    }
}
