#include <quantumc.h>
#include <stdint.h>


int64_t random()
{
    qubit q[64];
    for (int i = 0; i < 7; i++)
    {
        apply_H(q[i]);
    }

    uint8_t meas = measure(uint64_t, q);

    return meas;
}
