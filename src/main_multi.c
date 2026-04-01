#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define EINVAL 22

int invalid_args(char* name);

int main(int argc, char* argv[]) {
    const char *path = "./totally_not_phasmophoiba";
    int n_sim;
    int n_hunt = 14;
    int win_cnt = 0;
    int lose_cnt = 0;
    int other_cnt = 0;

    if (argc != 2 && argc != 3) {
        return invalid_args(argv[0]);
    }

    n_sim = atoi(argv[1]);
    
    if (n_sim < 30) {
        n_sim = 30;
        printf("%s: 30 simulation minimum. running 30 simulations\n", argv[0]);
    }

    if (argc == 3) {
        n_hunt = atoi(argv[2]);
        if (n_hunt < 1) return invalid_args(argv[0]);
    }

    pid_t *pids = calloc((size_t)n_sim, sizeof(pid_t));
    
    for (int i = 0; i < n_sim; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            char buf_hunt[32];
            snprintf(buf_hunt, sizeof(buf_hunt), "%d", n_hunt);

            char *child_argv[] = {(char*)path, (char*)"-report", buf_hunt, NULL};
            execv(path, child_argv);

            exit(EXIT_FAILURE); // execv failed
        }
        pids[i] = pid;
    }

    for (int i = 0; i < n_sim; i++) {
        int status = 0;

        if (pids[i])

        if (waitpid(pids[i], &status, 0) < 0) {
            other_cnt++;
            continue;
        }

        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            if (code == SIGUSR1) win_cnt++;
            else if (code == SIGUSR2) lose_cnt++;
            else other_cnt++;
        } else {
            other_cnt++;
        }
    }

    free(pids);

    printf("Simulation results for %d runs with %d hunters\n", n_sim, n_hunt);
    printf("Hunter Wins: %d\n", win_cnt);
    printf("Ghost Wins: %d\n", lose_cnt);
    if (lose_cnt > 0) {
        // avoid div by 0
        printf("Hunter Win/Ghost Win Ratio: %-3f:1\n", (float)win_cnt/(float)lose_cnt);
    } else {
        printf("Hunter Win/Ghost Win Ratio: N/A (0 Ghost Wins)\n");
    }
    return 0;
}

int invalid_args(char* name) {
    printf("%s: Invalid Arguments.\n\n", name);
    printf("Usage: %s [n_sim]\n   or: %s [n_sim] [n_hunt]\n\n", name, name);
    printf("n_sim: Number of times to run the Simulation (30 simulation mimimum, default 30)\n");
    printf("n_hunt: Number of Hunters present in each Simulation (mimimum of 1, default 14)\n");
    return EINVAL;
}