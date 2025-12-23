#include <quantumc.h>


int hadamard() {
    qubit q;
    apply_H(q);
    
    int meas = measure(uint8_t, q);

    return meas;
}
