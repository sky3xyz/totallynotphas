#include "defs.h"
#include "helpers.h"
#include <unistd.h>
#include <stdio.h>

void ghost_init(struct Ghost* ghost, struct House* house) {
    static int ghostID = DEFAULT_GHOST_ID;
    struct Room* initRoom = house_getRndRoom(house);

    ghost->id = ghostID++;
    ghost->boredom = 0;
    ghost->in_simulation = true;
    ghost->type = ghost_randomGhostType();

    sem_wait(&initRoom->mutex);
    ghost->currentRoom = initRoom;
    initRoom->currentGhost = ghost;
    log_ghost_init(ghost->id, ghost->currentRoom->name, ghost->type);
    sem_post(&initRoom->mutex);
}

enum GhostType ghost_randomGhostType() {
    const enum GhostType* typeList = NULL;
    int cnt = get_all_ghost_types(&typeList);
    int rand = rand_int_threadsafe(0, cnt);

    return typeList[rand];
}

void ghost_move(struct Ghost* ghost, struct Room* destinationRoom) {
    if (ghost == NULL || destinationRoom == NULL) {
        fprintf(stderr, "[ERROR] ghost_move: init null check failed\n");
        return;
    }
    
    struct Room* currentRoom = ghost->currentRoom;

    if (currentRoom == NULL) {
        fprintf(stderr, "[ERROR] ghost_move: 2nd null check failed\n");
        return;
    }

    if (currentRoom == destinationRoom) {
        // fprintf(stderr, "[INFO] ghost_move: attempted to move to same room\n");
        return;
    }

    deadlock_lockRooms(currentRoom, destinationRoom);

    if (ghost->currentRoom != currentRoom || currentRoom->currentGhost != ghost || currentRoom->presentHunters_cnt != 0) {
        deadlock_unlockRooms(currentRoom, destinationRoom);
        ghost_idle(ghost);
        return;
    }

    currentRoom->currentGhost = NULL;
    destinationRoom->currentGhost = ghost;
    ghost->currentRoom = destinationRoom;

    log_ghost_move(ghost->id, ghost->boredom, currentRoom->name, destinationRoom->name);
    deadlock_unlockRooms(currentRoom, destinationRoom);
}

void ghost_idle(struct Ghost* ghost) {
    log_ghost_idle(ghost->id, ghost->boredom, ghost->currentRoom->name);  
    usleep(rand_int_threadsafe(5000, 15000));
    (void)ghost; // use parameter to avoid warning
}

void ghost_haunt(struct Ghost* ghost) {
    if (ghost == NULL || ghost->currentRoom == NULL) return;

    const enum EvidenceType* allEvidence = NULL;
    int evidenceCnt = get_all_evidence_types(&allEvidence);
    enum EvidenceType options[3];
    EvidenceByte bits = (EvidenceByte)ghost->type;

    int cnt = 0;
    for (int i = 0; i < evidenceCnt; i++) {
        // change me, use evidence helper
        if ((bits & allEvidence[i]) != 0) {
            options[cnt++] = allEvidence[i];
        }
    }

    if (cnt > 0) {
        int evidenceI = rand_int_threadsafe(0, cnt);
        room_addEvidence(ghost->currentRoom, options[evidenceI]);
        log_ghost_evidence(ghost->id, ghost->boredom, ghost->currentRoom->name, options[evidenceI]);
    }
}

void ghost_takeTurn(struct Ghost* ghost) {
    // fprintf(stderr, "[INFO] GHOST TOOK TURN\n");
    if (ghost == NULL) {
        fprintf(stderr, "[ERROR] GHOST NULL\n");
        return;
    }

    if(ghost->currentRoom == NULL) {
        fprintf(stderr, "[WARN] GHOST NOT IN ANY ROOM\n");
        return;
    }

    if (!ghost->in_simulation) {
        // fprintf(stderr, "[INFO] GHOST NOT IN SIMULATION\n");
        return;
    }

    // take "snapshot" and quickly release the mutex
    struct Room *currentRoom = ghost->currentRoom;
    struct Room *neighbors[MAX_CONNECTIONS] = {0};
    int hunterCount = 0;
    int neighborCount = 0;

    sem_wait(&currentRoom->mutex);
    hunterCount = currentRoom->presentHunters_cnt;
    neighborCount = currentRoom->connectedRoom_cnt;

    for (int i = 0; i < neighborCount; i++) {
        neighbors[i] = currentRoom->connectedRooms[i];
    }
    sem_post(&currentRoom->mutex);

    // ghost is bored
    if (ghost_updateCheckBoredom(ghost, hunterCount)) {
        sem_wait(&currentRoom->mutex);

        if (currentRoom->currentGhost == ghost && currentRoom->presentHunters_cnt == 0) {
            char* prevRoom = currentRoom->name;
            currentRoom->currentGhost = NULL;
            ghost->currentRoom = NULL;
            ghost->in_simulation = false;
            log_ghost_exit(ghost->id, ghost->boredom, prevRoom);
        } else if (currentRoom->presentHunters_cnt > 0) {
            ghost->boredom = 0;
        }

        sem_post(&currentRoom->mutex);
        return;
    }

    int upperBound_actions = 3;
    if (hunterCount > 0 || neighborCount == 0) upperBound_actions = 2;
    enum GhostAction selectedAction = (enum GhostAction)rand_int_threadsafe(0, upperBound_actions);

    switch (selectedAction) {
        case GA_IDLE:
            ghost_idle(ghost);
            return;

        case GA_HAUNT:
            ghost_haunt(ghost);
            return;

        case GA_MOVE: {
            struct Room *destination = neighbors[rand_int_threadsafe(0, neighborCount)];
            ghost_move(ghost, destination);
            return;
        }

        default:
            fprintf(stderr, "[WARN] DEFAULT CASE ON SWITCH\n");
            ghost_idle(ghost);
            return;
    }
}

bool ghost_updateCheckBoredom(struct Ghost* ghost, int hunterCount) {
    if (hunterCount > 0) {
        ghost->boredom = 0;
    } else {
        ghost->boredom++;
    }

    return (ghost->boredom >= ENTITY_BOREDOM_MAX);
}

void* ghost_thread(void* arg) {
    // fprintf(stderr, "[INFO] GHOST THREAD STARTED\n");
    struct Ghost *ghost = arg;
    if (ghost == NULL) return NULL;

    while (ghost->in_simulation) {
        ghost_takeTurn(ghost);
        usleep(rand_int_threadsafe(5000, 15000));
    }
    return NULL;
}
