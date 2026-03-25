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
    struct Room* currentRoom = ghost->currentRoom;

    if (ghost == NULL || currentRoom == NULL || destinationRoom == NULL) return;
    if (currentRoom == destinationRoom) return;

    deadlock_lockRooms(currentRoom, destinationRoom);

    if (currentRoom->presentHunters_cnt != 0) {
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
    usleep(rand_int_threadsafe(5, 15));
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


void ghost_takeTurn(struct Ghost* ghost);
void* ghost_thread(void* arg);