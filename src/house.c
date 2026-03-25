#include "defs.h"
#include "helpers.h"

void house_init(struct House* house) {
    if (house == NULL) return;

    house->starting_room = NULL;
    // hunterArray_init(&house->hunters);
    // casefile_init(&house->caseFile);
    ghost_init(&house->ghost, house);
    house->room_count = 0;
}

struct Room* house_getRndRoom(struct House* house) {
    if (house == NULL || house->room_count == 0) return NULL;
    int roomID;

    do {
        roomID = rand_int_threadsafe(0, house->room_count - 1);
    } while(&house->rooms[roomID] != NULL);

    return &house->rooms[roomID];
}

void house_clear(struct House* house) {
    if (house == NULL) return;

    for (int i = 0; i < house->room_count; i++) {
        sem_destroy(&house->rooms[i].mutex);
    }

    house->starting_room = NULL;
    house->room_count = 0;
}