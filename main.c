#include "defs.h"
#include "helpers.h"

int test();

int main() {
    /*
    1. Initialize a House structure.
    2. Populate the House with rooms using the provided helper function.
    3. Initialize all of the ghost data and hunters.
    4. Create threads for the ghost and each hunter.
    5. Wait for all threads to complete.
    6. Print final results to the console:
         - Type of ghost encountered.
         - The reason that each hunter exited
         - The evidence collected by each hunter and which ghost is represented by that evidence.
    7. Clean up all dynamically allocated resources and call sem_destroy() on all semaphores.
    */

    return test();
}

int test(void) {
    struct House house;

    house_init(&house);
    house_populate_rooms(&house);

    printf("Starting room: %s\n\n",
           house.starting_room ? house.starting_room->name : "NULL");

    for (int i = 0; i < house.room_count; i++) {
        printf("%s\n", house.rooms[i].name);

        for (int j = 0; j < house.rooms[i].connectedRoom_cnt; j++) {
            printf("   %s\n", house.rooms[i].connectedRooms[j]->name);
        }

        printf("\n");
    }

    house_clear(&house);
    return 0;
}
