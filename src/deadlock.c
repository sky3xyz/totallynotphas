#include "helpers.h"
#include "defs.h"
#include <string.h>
#include <stdio.h>
// my favorite valorant agent

bool deadlock_whoFirst(const struct Room* roomOne, const struct Room* roomTwo) {
    if (roomOne->id != roomTwo->id) return (roomOne->id > roomTwo->id);

    // tie breaker -- compare size (not length) of room name
    if (strlen(roomOne->name) != strlen(roomTwo->name)) return strlen(roomOne->name) > strlen(roomTwo->name);

    // tie tie breaker -- return roomOne
    fprintf(stderr, "[WARN] DEADLOCK_WHOFIRST BOTH CASES FAIL\n");
    return true;
}

int deadlock_lockRooms(struct Room* roomOne, struct Room* roomTwo) {
    if (roomOne == NULL || roomTwo == NULL) {
        fprintf(stderr, "[ERROR] DEADLOCK_LOCKROOMS ROOMONE OR ROOMTWO NULL\n");
        return C_ERR_EINVAL;
    } 
    
    if (roomOne == roomTwo) {
        sem_wait(&roomOne->mutex);
        fprintf(stderr, "[INFO] DEADLOCK LOCK COMPLETE\n");
        return C_ERR_OK;
    }

    struct Room *firstRoom = roomOne;
    struct Room *secondRoom = roomTwo;

    if (!deadlock_whoFirst(roomOne, roomTwo)) {
        firstRoom = roomTwo;
        secondRoom = roomOne;
    }

    if(sem_wait(&firstRoom->mutex) != 0) fprintf(stderr, "[ERROR] DEADLOCK_LOCK UNABLE TO SEM_WAIT RM\n");
    if (sem_wait(&secondRoom->mutex) != 0) fprintf(stderr, "[ERROR] DEADLOCK_LOCK UNABLE TO SEM_WAIT RM2\n");
    fprintf(stderr, "[INFO] DEADLOCK LOCK COMPLETE\n");
    return C_ERR_OK;
}

int deadlock_unlockRooms(struct Room* roomOne, struct Room* roomTwo) {
    if (roomOne == NULL || roomTwo == NULL) {
        fprintf(stderr, "[ERROR] DEADKLOCK_UNLOCK RM1 OR RM2 NULL\n");
        return C_ERR_EINVAL;
    }
    
    if (roomOne == roomTwo) {
        sem_post(&roomOne->mutex);
        fprintf(stderr, "[INFO] DEADLOCK UNLOCK COMPLETE\n");
        return C_ERR_OK;
    }

    struct Room *firstRoom = roomOne;
    struct Room *secondRoom = roomTwo;

    if (!deadlock_whoFirst(roomOne, roomTwo)) {
        firstRoom = roomTwo;
        secondRoom = roomOne;
    }

    // unlock in reverse order of lock. pthreads man page said so :shrug:
    if (sem_post(&secondRoom->mutex) != 0) fprintf(stderr, "[ERROR] DEADLOCK_UNLOCK UNABLE TO SEM_POST RM2\n");
    if (sem_post(&firstRoom->mutex) != 0) fprintf(stderr, "[ERROR] DEADLOCK_UNLOCK UNABLE TO SEM_POST RM1\n");
    fprintf(stderr, "[INFO] DEADLOCK UNLOCK SUCCESSFUL\n");
    return C_ERR_OK;
}