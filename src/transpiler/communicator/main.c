#include <stdio.h>
#include <stdlib.h>

#include "dispatcher.h"


int main(void)
{
    fprintf(stdout, "=== QuantumC Runtime ===\n\n");

    char* response = ibm_authentication("sghtM2r_uguGk2lspqyI0lIU42UxsaIrViZeZVwQ5gGn");
    if (response) {
        fprintf(stdout, "Obtaining the bearer token was successful!\n");
        fprintf(stdout, "%s\n", response);
    }

    free(response);

    // fprintf(stdout, "=== Final Result ===\n\n");

    return 0;
}
