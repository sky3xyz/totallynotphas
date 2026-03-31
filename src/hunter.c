#include "defs.h"
#include "helpers.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void hunter_init(struct Hunter* hunter, const char* name, int id, struct Room* startRoom, struct CaseFile* caseFile, enum EvidenceType device) {
    strncpy(hunter->name, name, MAX_HUNTER_NAME);
    hunter->name[MAX_HUNTER_NAME-1] = '\0';

    roomStack_init(&hunter->path);
    roomStack_push(&hunter->path, startRoom);

    hunter->id = id;
    hunter->currentDevice = device;
    hunter->currentRoom = startRoom;
    hunter->caseFile_ptr = caseFile;
    hunter->boredom = 0;
    hunter->fear = 0;
    hunter->returningToVan = false;
    hunter->exitedSimulation = false;

    log_hunter_init(hunter->id, hunter->currentRoom->name, hunter->name, hunter->currentDevice);
}

int hunter_gatherEvidence(struct Hunter* hunter) {
    // null checks
    if (hunter == NULL) {
        fprintf(stderr, "[ERROR] hunter_gatherEvidence: hunter was NULL\n");
        return C_ERR_EINVAL;
    }

    if (hunter->caseFile_ptr == NULL) {
        fprintf(stderr, "[ERROR] hunter_gatherEvidence: hunter->caseFile_ptr was NULL\n");
        return C_ERR_EINVAL;
    }

    if (sem_wait(&hunter->currentRoom->mutex) != 0) {
        fprintf(stderr, "[ERROR] hunter_gatherEvidence: failed to sem_wait on currentRoom\n");
        return C_ERR_NOT_OK;
    }
    
    if (evidence_has(hunter->currentRoom->roomEvidence, hunter->currentDevice) == true) {
        evidence_clear(&hunter->currentRoom->roomEvidence, hunter->currentDevice);
        sem_post(&hunter->currentRoom->mutex);
        casefile_addEvidence(hunter->caseFile_ptr, hunter->currentDevice);
        log_evidence(hunter->id, hunter->boredom, hunter->fear, hunter->currentRoom->name, hunter->currentDevice);

        return C_ERR_OK;
    }

    sem_post(&hunter->currentRoom->mutex);
    // fprintf(stderr, "[INFO] hunter_gatherEvidence: hunter did not find evidence\n");
    return C_ERR_NO_EVIDENCE;
}

void hunter_updateState(struct Hunter* hunter) {
    // null check
    if (hunter == NULL) {
        fprintf(stderr, "[ERROR] hunter_updateCheckState: hunter was NULL\n");
        return;
    }
    if (hunter->currentRoom == NULL) {
        fprintf(stderr, "[ERROR] hunter_updateCheckState: hunter->currentRoom was NULL\n");
        return;
    }

    // update fear/boredom
    sem_wait(&hunter->currentRoom->mutex);
    if (hunter->currentRoom->currentGhost != NULL) {
        hunter->boredom = 0;
        hunter->fear++;
        // fprintf(stderr, "[INFO] hunter (%d) state updated, ghost present\n", hunter->id);
        sem_post(&hunter->currentRoom->mutex);
        return;
    }
    sem_post(&hunter->currentRoom->mutex);

    hunter->boredom++;
    // fprintf(stderr, "[INFO] hunter (%d) state updated, ghost not present\n", hunter->id);
}

void hunter_move(struct Hunter* hunter, struct Room* destinationRoom) { 
    if (hunter == NULL || destinationRoom == NULL) {
        fprintf(stderr, "[ERROR] hunter_move: init null check failed\n");
        return;
    }

    struct Room* currentRoom = hunter->currentRoom;

    if (currentRoom == NULL) {
        fprintf(stderr, "[ERROR] hunter_move: 2nd null check failed\n");
        return;
    }

    if (currentRoom == destinationRoom) {
        // fprintf(stderr, "[INFO] hunter_move: attempted to move to same room\n");
        return;
    }

    deadlock_lockRooms(currentRoom, destinationRoom);

    if (room_isFull(destinationRoom)) {
        // fprintf(stderr, "[INFO] hunter_move: destination room %s is full\n", destinationRoom->name);
        deadlock_unlockRooms(currentRoom, destinationRoom);
        return;
    }

    if (room_removeHunter(currentRoom, hunter) != C_ERR_OK) {
        if (currentRoom->exit == false) {
            deadlock_unlockRooms(currentRoom, destinationRoom);
            return; 
        }
    }

    if (!room_addHunter(destinationRoom, hunter)) {
        room_addHunter(currentRoom, hunter);
        deadlock_unlockRooms(currentRoom, destinationRoom);
        return;
    }

    if (!hunter->returningToVan) {
        roomStack_push(&hunter->path, currentRoom);
    }

    hunter->currentRoom = destinationRoom;

    log_move(hunter->id, hunter->boredom, hunter->fear, currentRoom->name, destinationRoom->name, hunter->currentDevice);

    deadlock_unlockRooms(currentRoom, destinationRoom);
}

void hunter_equipmentSwap(struct Hunter* hunter) {
    if (hunter == NULL) {
        fprintf(stderr, "[ERROR] hunter_equipmentSwap: hunter was NULL\n");
        return;
    }

    const enum EvidenceType* evidenceList = NULL;
    int cnt = get_all_evidence_types(&evidenceList);

    if (cnt == 0 || evidenceList == NULL) {
        fprintf(stderr, "[ERROR] hunter_equipmentSwap: NULL err\n");
        return;
    }

    enum EvidenceType newDevice;
    do {
        int rand = rand_int_threadsafe(0, cnt);
        newDevice = evidenceList[rand];
    } while (newDevice == hunter->currentDevice);

    log_swap(hunter->id, hunter->boredom, hunter->fear, hunter->currentDevice, newDevice);
    hunter->currentDevice = newDevice;
}

void hunter_exitSimulation(struct Hunter* hunter) {
    if (hunter == NULL || hunter->exitedSimulation) {
        fprintf(stderr, "[ERROR] hunter_exitSimulation: called improperly\n");
        return;
    }

    if (hunter->currentRoom != NULL) {
        sem_wait(&hunter->currentRoom->mutex);
        room_removeHunter(hunter->currentRoom, hunter);
        sem_post(&hunter->currentRoom->mutex);
        
        log_exit(hunter->id, hunter->boredom, hunter->fear, hunter->currentRoom->name, hunter->currentDevice, hunter->exitReason);
        hunter->currentRoom = NULL;
    }

    roomStack_clear(&hunter->path);
    hunter->exitedSimulation = true;
}

void hunter_takeTurn(struct Hunter* hunter) {
    // this is if statement slop, i am so sorry.
    if (hunter == NULL || hunter->exitedSimulation) return;

    hunter_updateState(hunter);

    if (hunter->currentRoom != NULL && hunter->currentRoom->exit) {
        roomStack_clear(&hunter->path);

        sem_wait(&hunter->caseFile_ptr->mutex);
        bool solved = hunter->caseFile_ptr->solved;
        sem_post(&hunter->caseFile_ptr->mutex);

        if(solved) {
            hunter->exitReason = LR_EVIDENCE;
            hunter_exitSimulation(hunter);
            return;
        }

        if (hunter->returningToVan) {
            log_return_to_van(hunter->id, hunter->boredom, hunter->fear, hunter->currentRoom->name, hunter->currentDevice, false);

            hunter_equipmentSwap(hunter);
            hunter->returningToVan = false;
            hunter->boredom = 0;
        }   
    }

    if (hunter->fear >= HUNTER_FEAR_MAX) {
        hunter->exitReason = LR_AFRAID;
        hunter_exitSimulation(hunter);
        return;
    } else if (hunter->boredom >= ENTITY_BOREDOM_MAX) {
        hunter->exitReason = LR_BORED;
        hunter_exitSimulation(hunter);
        return;
    }

    if (!hunter->returningToVan && !hunter->currentRoom->exit) {
        if (hunter_gatherEvidence(hunter) == C_ERR_OK) {
            hunter->returningToVan = true;
            log_return_to_van(hunter->id, hunter->boredom, hunter->fear, hunter->currentRoom->name, hunter->currentDevice, true);
        } else {
            if (rand_int_threadsafe(0, 100) < 15) {
                hunter->returningToVan = true;
                log_return_to_van(hunter->id, hunter->boredom, hunter->fear, hunter->currentRoom->name, hunter->currentDevice, true);
            }
        }
    }

    if (hunter->returningToVan) {
        struct Room* nextRoom = roomStack_pop(&hunter->path);
        if (nextRoom != NULL) {
            struct Room* oldRoom = hunter->currentRoom;

            hunter_move(hunter, nextRoom);

            if (hunter->currentRoom == oldRoom) {
                roomStack_push(&hunter->path, nextRoom);
            }
        } else if (hunter->currentRoom != NULL && !hunter->currentRoom->exit) {
                // stack empty but not in van, force leave simulation. avoid infinite loop.
                fprintf(stderr, "[ERROR] hunter_takeTurn: hunter %s (ID: %d) has empty RoomStack. Kicked from simulation.\n", hunter->name, hunter->id);
                hunter_exitSimulation(hunter);
                return;
            }
    } else {
        if (hunter->currentRoom->connectedRoom_cnt > 0) {
            int rand = rand_int_threadsafe(0, hunter->currentRoom->connectedRoom_cnt);
            struct Room* nextRoom = hunter->currentRoom->connectedRooms[rand];
            
            hunter_move(hunter, nextRoom);
        }
    }
}

void* hunter_thread(void* arg) {
    struct Hunter* hunter = (struct Hunter*)arg;
    if (hunter == NULL) return NULL;

    // fprintf(stderr, "[INFO] HUNTER THREAD STARTED: %s\n", hunter->name);

    while (!hunter->exitedSimulation) {
        hunter_takeTurn(hunter);
        usleep(rand_int_threadsafe(5000, 15000));
    }
    
    // fprintf(stderr, "[INFO] HUNTER THREAD ENDED: %s\n", hunter->name);
    return NULL;
}