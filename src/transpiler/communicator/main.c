#include <stdio.h>
#include <stdlib.h>

#include "sender.h"


int main(void)
{
    fprintf(stdout, "=== QuantumC Runtime ===\n\n");

    char* response = authenticate("vT35YnA3brgOal9-xdWQ7-yEvUaWg7dTw4qaVD7gh1ae", "");
    if (response) {
        fprintf(stdout, "Obtaining the bearer token was successful!\n");
        fprintf(stdout, "%s\n", response);
    }

    free(response);

    // fprintf(stdout, "=== Final Result ===\n\n");

    return 0;
}
