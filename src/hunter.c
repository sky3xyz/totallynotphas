#include "defs.h"
#include "helpers.h"
#include <string.h>
#include <stdio.h>

/*
void hunter_move(struct Hunter* hunter, struct Room* destinationRoom);
void hunter_takeTurn(struct Hunter* hunter);
void hunter_equipmentSwap(struct Hunter* hunter);
void hunter_exitRoom(struct Hunter* hunter);
void* hunter_thread(void* arg);
*/

void hunter_init(struct Hunter* hunter, const char* name, int id, struct Room* startRoom, struct CaseFile* caseFile, enum EvidenceType device) {
    strncpy(hunter->name, name, MAX_HUNTER_NAME);
    hunter->name[MAX_HUNTER_NAME-1] = '\0';

    roomStack_init(&hunter->path);

    hunter->id = id;
    hunter->currentDevice = device;
    hunter->currentRoom = startRoom;
    hunter->caseFile_ptr = caseFile;
    hunter->boredom = 0;
    hunter->fear = 0;
    hunter->returningToVan = false;
    hunter->exitedSimulation = false;

    log_hunter_init(hunter->id, hunter->currentRoom->name, hunter->name, hunter->currentDevice);
    fprintf(stderr, "[INFO] hunter_init: Hunter %s (ID: %d) init.\n", hunter->name, hunter->id);
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

    // take snapshot of current evidence in room
    if (sem_wait(&hunter->currentRoom->mutex) != 0) {
        fprintf(stderr, "[ERROR] hunter_gatherEvidence: failed to sem_wait on currentRoom\n");
        return C_ERR_NOT_OK;
    }
    enum EvidenceType roomEvidence = hunter->currentRoom->roomEvidence;
    if (sem_post(&hunter->currentRoom->mutex) != 0) {
        fprintf(stderr, "[ERROR] hunter_gatherEvidence: failed to sem_post on currentRoom\n");
        return C_ERR_NOT_OK;
    }

    if (evidence_has(roomEvidence, hunter->currentDevice) == true) {
        casefile_addEvidence(hunter->caseFile_ptr, hunter->currentDevice);
        log_evidence(hunter->id, hunter->boredom, hunter->fear, hunter->currentRoom->name, hunter->currentDevice);
        return C_ERR_OK;
    }

    fprintf(stderr, "[INFO] hunter_gatherEvidence: hunter did not find evidence\n");
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
    if (hunter->currentRoom->currentGhost != NULL) {
        hunter->boredom = 0;
        hunter->fear++;
        fprintf(stderr, "[INFO] hunter (%d) state updated, ghost present\n", hunter->id);
        return;
    }

    hunter->boredom++;
    fprintf(stderr, "[INFO] hunter (%d) state updated, ghost not present\n", hunter->id);
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
        fprintf(stderr, "[INFO] hunter_move: attempted to move to same room\n");
        return;
    }
}