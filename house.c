#include "defs.h"

void house_init(struct House* house) {
    if (house == NULL) return;

    house->starting_room = NULL;
    // hunterArray_init(&house->hunters);
    // casefile_init(&house->caseFile);
    // ghost_init(&house->ghost);
    house->room_count = 0;
}

void house_clear(struct House* house) {
    if (house == NULL) return;

    for (int i = 0; i < house->room_count; i++) {
        sem_destroy(&house->rooms[i].mutex);
    }

    house->starting_room = NULL;
    house->room_count = 0;
}