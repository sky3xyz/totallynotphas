#include "helpers.h"
#include "defs.h"
#include <string.h>
// my favorite valorant agent

bool deadlock_whoFirst(const struct Room* roomOne, const struct Room* roomTwo) {
    if (roomOne->id != roomTwo->id) return (roomOne->id > roomTwo->id);

    // tie breaker -- compare size (not length) of room name
    return (strlen(roomOne->name) > strlen(roomTwo->name));

    // tie tie breaker -- return roomOne
    return true;
}

int deadlock_lockRooms(struct Room* roomOne, struct Room* roomTwo) {
    if (roomOne == NULL || roomTwo == NULL) return C_ERR_EINVAL;
    
    if (roomOne == roomTwo) {
        sem_wait(&roomOne->mutex);
        return C_ERR_OK;
    }

    struct Room *firstRoom = roomOne;
    struct Room *secondRoom = roomTwo;

    if (!deadlock_whoFirst(roomOne, roomTwo)) {
        firstRoom = roomTwo;
        secondRoom = roomOne;
    }

    sem_wait(&firstRoom->mutex);
    sem_wait(&secondRoom->mutex);
    return C_ERR_OK;
}

int deadlock_unlockRooms(struct Room* roomOne, struct Room* roomTwo) {
    if (roomOne == NULL || roomTwo == NULL) return C_ERR_EINVAL;
    
    if (roomOne == roomTwo) {
        sem_post(&roomOne->mutex);
        return C_ERR_OK;
    }

    struct Room *firstRoom = roomOne;
    struct Room *secondRoom = roomTwo;

    if (!deadlock_whoFirst(roomOne, roomTwo)) {
        firstRoom = roomTwo;
        secondRoom = roomOne;
    }

    // unlock in reverse order of lock. pthreads man page said so :shrug:
    sem_post(&secondRoom->mutex);
    sem_post(&firstRoom->mutex);
    return C_ERR_OK;
}