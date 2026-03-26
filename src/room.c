#include "defs.h"
#include <stdio.h>

void room_init(struct Room* room, const char* name, bool is_exit) {
    static int roomID = 0;
    room->id = roomID++;

    sem_init((&room->mutex), 0, 1);
    room->roomEvidence = 0;
    room->exit = is_exit;
    strncpy(room->name, name, MAX_ROOM_NAME - 1);
    room->name[MAX_ROOM_NAME - 1] = '\0';
    room->currentGhost = NULL;
    room->connectedRoom_cnt = 0;
    room->presentHunters_cnt = 0;

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        room->connectedRooms[i] = NULL;
    }   

    for (int i = 0; i < MAX_ROOM_OCCUPANCY; i++) {
        room->presentHunters[i] = NULL;
    }
}

void room_addEvidence(struct Room* room, EvidenceByte evidence) {
    sem_wait(&room->mutex);
    evidence_set(&room->roomEvidence, evidence);
    sem_post(&room->mutex);
}

bool room_addHunter(struct Room* room, struct Hunter* hunter) {
    for (int i = 0; i < MAX_ROOM_OCCUPANCY; i++) {
        if (room->presentHunters[i] == NULL) {
            room->presentHunters[i] = hunter;
            room->presentHunters_cnt++;
            fprintf(stderr, "[INFO] HUNTER ADDED TO ROOM\n");
            return true;
        }
    }
    fprintf(stderr, "[ERROR] HUNTER UNABLE TO BE ADDED TO ROOM\n");
    return false;
}

int room_removeHunter(struct Room* room, struct Hunter* hunter) {
    for (int i = 0; i < MAX_ROOM_OCCUPANCY; i++) {
        if (room->presentHunters[i] == hunter) {
            room->presentHunters[i] = NULL;
            room->presentHunters_cnt--;
            fprintf(stderr, "[INFO] HUNTER REMOVED FROM ROOM\n");
            return C_ERR_OK;
        }
    }
    fprintf(stderr, "[ERROR] HUNTER UNABLE TO BE REMOVED FROM ROOM\n");
    return C_ERR_NOT_OK;
}

int room_connect(struct Room* a, struct Room* b) {
    if (a->connectedRoom_cnt < MAX_CONNECTIONS && b->connectedRoom_cnt < MAX_CONNECTIONS) {
        a->connectedRooms[a->connectedRoom_cnt++] = b;
        b->connectedRooms[b->connectedRoom_cnt++] = a;
        return C_ERR_OK;
    }
    return C_ERR_NOT_OK;
}

bool room_isFull(struct Room* room) {
    return !(room->presentHunters_cnt < MAX_ROOM_OCCUPANCY);
}