#include "defs.h"
#include "helpers.h"
#include <unistd.h>

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
    sem_post(&initRoom->mutex);
}

enum GhostType ghost_randomGhostType() {
    EvidenceByte bits = 0;

    do {
        bits = (EvidenceByte)rand_int_threadsafe(0, 127); 
    } while (!evidence_is_valid_ghost(bits));

    return (enum GhostType)bits;
}

void ghost_move(struct Ghost* ghost, struct Room* destinationRoom) {
    if (ghost == NULL || destinationRoom == NULL) return;
    struct Room* currentRoom = ghost->currentRoom;

    if (currentRoom == NULL || currentRoom == destinationRoom ) return;

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
        if ((bits & allEvidence[i]) != 0) {
            options[cnt++] = allEvidence[i];
        }
    }

    if (cnt > 0) {
        room_addEvidence(ghost->currentRoom, options[rand_int_threadsafe(0, cnt - 1)]);
    }
}


void ghost_takeTurn(struct Ghost* ghost) {
    if (ghost == NULL || !ghost->in_simulation || ghost->currentRoom == NULL) return;

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
            currentRoom->currentGhost = NULL;
            ghost->currentRoom = NULL;
            ghost->in_simulation = false;
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
        case IDLE:
            ghost_idle(ghost);
            return;

        case HAUNT:
            ghost_haunt(ghost);
            return;

        case MOVE: {
            struct Room *destination = neighbors[rand_int_threadsafe(0, neighborCount-1)];
            ghost_move(ghost, destination);
            return;
        }

        default:
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
    struct Ghost *ghost = arg;
    if (ghost == NULL) return NULL;

    while (ghost->in_simulation) {
        ghost_takeTurn(ghost);
        usleep(rand_int_threadsafe(5000, 15000));
    }
    return NULL;
}