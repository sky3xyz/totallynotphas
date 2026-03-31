#include "defs.h"
#include "helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

int createThreads(struct House* house, pthread_t* arr);
void printResults(struct House* house);

int main() {
    struct House house;
    house_init(&house);
    house_populate_rooms(&house);
    ghost_init(&house.ghost, &house);
    
    // take input
    const enum EvidenceType* all_ev = NULL;
    int ev_cnt = get_all_evidence_types(&all_ev);
    char name_input[MAX_HUNTER_NAME];
    printf("========================================\n");
    printf("Legally Distinct Ghost Hunting Game\n");
    printf("Willow House Investigation\n");
    printf("========================================\n");

    while (1) {
        printf("\nEnter hunter name (or \"done\"): ");
        fgets(name_input, sizeof(name_input), stdin);
        name_input[strcspn(name_input, "\n")] = '\0';
        
        if (strcmp(name_input, "done") == 0) break;

        int hunter_id = -1;
        while(hunter_id == -1) {
            int c;
            printf("Enter hunter ID: ");
            if (scanf("%d", &hunter_id) != 1) {
                printf("Invalid ID, try again.\n");
            }
            while ((c = getchar()) != '\n' && c != EOF);
        }
        
        
        enum EvidenceType initialDevice = all_ev[rand_int_threadsafe(0, ev_cnt)];

        struct Hunter buddy;
        hunterArray_append(&house.hunters, &buddy);

        struct Hunter* h_ptr = &house.hunters.data[house.hunters.contains - 1];
        hunter_init(h_ptr, name_input, hunter_id, house.starting_room, &house.caseFile, initialDevice);

        printf("Added hunter %s (%d).\n", h_ptr->name, h_ptr->id);
    }

    if (house.hunters.contains == 0) {
        printf("[ERROR] No hunters added. Bye.\n");
        house_clear(&house);
        return 0;
    }

    for (int i = 0; i < house.hunters.contains; i++) {
        struct Hunter* h_ptr = &house.hunters.data[i];
        room_addHunter(house.starting_room, h_ptr);
    }
    
    pthread_t thread_arr[house.hunters.contains + 1];
    if (createThreads(&house, thread_arr) != 0) {
        printf("[ERROR] %s: error occurred when creating thread(s). check stderr.\n", "main");
    }

    pthread_join(thread_arr[0], NULL);
    for (int i = 0; i < house.hunters.contains; i++) {
        if (pthread_join(thread_arr[i+1], NULL) != 0) {
            fprintf(stderr, "[ERROR] %s: failed to join hunter thread %s (ID: %d)", "main", house.hunters.data[i].name, house.hunters.data[i].id);
        }
    }

    printResults(&house);
    house_clear(&house);

    return 0;
}

int createThreads(struct House* house, pthread_t* arr) {
    if (pthread_create(&arr[0], NULL, ghost_thread, &house->ghost) != 0) {
        fprintf(stderr, "[ERROR] createThreads: failed to create Ghost thread.\n");
        house_clear(house);
        return C_ERR_NOT_OK;
    }

    for (int i = 0; i < house->hunters.contains; i++) {
        if (pthread_create(&arr[i+1], NULL, hunter_thread, &house->hunters.data[i]) != 0) {
            fprintf(stderr, "[ERROR] createThreads: failed to create hunter thread for %s (ID: %d)", house->hunters.data[i].name, house->hunters.data[i].id);
        }
    }
    return C_ERR_OK;
}

void printResults(struct House* house) {
    const enum EvidenceType* all_ev = NULL;
    int ev_cnt = get_all_evidence_types(&all_ev);
    int collected_cnt = 0;
    int goodHunter_cnt = 0;

    printf("\n\n\n");
    printf("==============================\n");
    printf("Investigation Results\n");
    printf("==============================\n");
    printf("\n");

    // hunter statuses
    printf("=== HUNTER STATUS ===\n");
    for (int i = 0; i < house->hunters.contains; i++) {
        struct Hunter* hunter = &house->hunters.data[i];
        if (hunter->exitReason == LR_EVIDENCE){
            printf("\t- %s[\u2713]%s %s (ID: %d) exit reason: %s\n", "\x1b[32m", "\x1b[30m",hunter->name, hunter->id, exit_reason_to_string(hunter->exitReason));
            goodHunter_cnt++;
        }

        else printf("\t- %s[X]%s %s (ID: %d) exit reason: %s\n", "\x1b[31m", "\x1b[30m",hunter->name, hunter->id, exit_reason_to_string(hunter->exitReason));
    }
    printf("\n");

    // evidence
    printf("Shared Case File Checklist\n");
    printf("=== EVIDENCE COLLECTED ===\n");
    for (int i = 0; i < ev_cnt; i++) {
        if (evidence_has(house->caseFile.collected, all_ev[i])) {
            printf("\t- %s[\u2713]%s %s\n", "\x1b[32m", "\x1b[30m", evidence_to_string(all_ev[i]));
            collected_cnt++;
        } else {
            printf("\t- %s[X]%s %s\n", "\x1b[31m", "\x1b[30m", evidence_to_string(all_ev[i]));
        }
    }
    printf("\n");

    // ghost identitfication
    printf("=== VICTORY? ===\n");

    const enum GhostType* allGhosts = NULL;
    int ghost_cnt = get_all_ghost_types(&allGhosts);
    const char* foundGhost = "Insufficent Evidence";

    if (house->caseFile.solved) {
        for (int i = 0; i <ghost_cnt; i++) {
            if (house->caseFile.collected == (EvidenceByte)allGhosts[i]) {
                foundGhost = ghost_to_string(allGhosts[i]);
                break;
            }
        }
    }
    printf("Ghost Guess -> %s\n", foundGhost);
    printf("Hunters that exited due to evidence -> %d\n", goodHunter_cnt);
    printf("Actual Ghost Type -> %s\n", ghost_to_string(house->ghost.type));

    printf("\n\n=== RESULT ===\n");
    if (house->caseFile.solved) {
        printf("%sSUCCESS%s: Case solved!\n", "\x1b[32m", "\x1b[30m");
        printf("Hunters Win!\n");
    } else {
        printf("%sFAILED%s: Not enough evidence found.\n", "\x1b[31m", "\x1b[30m");
        printf("Ghost Wins!\n");
    }
    printf("====\n");
}