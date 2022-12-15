#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "random.h"

void random_pin(char *pin) {
    srand(time(NULL)); // NOLINT(cert-msc51-cpp)
    sprintf(pin, "%04u", rand() / (RAND_MAX / 9999)); // NOLINT(cert-msc50-cpp)
}