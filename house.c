#include "defs.h"

void house_init(struct House* house) {
    house->starting_room = NULL;
    hunterArray_init(&house->hunters);
    casefile_init(&house->caseFile);
    ghost_init(&house->ghost);
    house->room_count = 0;
}