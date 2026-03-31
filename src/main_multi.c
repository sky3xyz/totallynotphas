#include <stdio.h>
#include <pthread.h>
#define EINVAL 22

int main(int argc, char* argv[]) {
    int 
    if (argc == 1) {
        printf("%s: Missing Arguments.\n\n", argv[0]);
        printf("Usage: %s [N_SIM]\n   or: %s [N_SIM] [N_HUNT]\n\n", argv[0], argv[0]);
        printf("N_SIM: Number of times to run the Simulation\n");
        printf("N_HUNT: Number of Hunters present in each Simulation\n");
        return EINVAL;
    }
    


}