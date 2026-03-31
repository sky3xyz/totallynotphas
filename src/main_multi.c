#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define EINVAL 22

int invalid_args(char* name);

int main(int argc, char* argv[]) {
    int N_SIM;
    int N_HUNT;
    int child_PID;
    if (argc != 2 && argc != 3) {
        return invalid_args(argv[0]);
    }

    N_SIM = atoi(argv[1]);
    if (N_SIM < 30) N_SIM = 30;

    if (argc == 3) {
        N_HUNT = atoi(argv[2]);
        if (N_HUNT < 1) invalid_args(argv[0]);
    }
}

int invalid_args(char* name) {
    printf("%s: Invalid Arguments.\n\n", name);
    printf("Usage: %s [N_SIM]\n   or: %s [N_SIM] [N_HUNT]\n\n", name, name);
    printf("N_SIM: Number of times to run the Simulation (Default: 30)\n");
    printf("N_HUNT: Number of Hunters present in each Simulation (Must be 1 or greater)\n");
    return EINVAL;
}