#include "defs.h"
#include <stddef.h>
#include <stdlib.h>

void roomStack_init(struct RoomStack* roomStack) {
    roomStack->top = NULL;
    roomStack->count = 0;
}

int roomStack_push(struct RoomStack* roomStack, struct Room* room) {
    if (roomStack == NULL) return C_ERR_EINVAL;

    struct RoomNode* roomNode = (struct RoomNode*) calloc(1, sizeof(struct RoomNode));
    if (roomNode == NULL) return C_ERR_ENOMEM;

    roomNode->room = room;
    roomNode->next = roomStack->top;
    roomStack->top = roomNode;
    roomStack->count++;
    return C_ERR_OK;
}

struct Room* roomStack_pop(struct RoomStack* roomStack) {
    if (roomStack == NULL || roomStack->top == NULL) return NULL;

    struct RoomNode* roomNode = roomStack->top;
    struct Room* room;

    room = roomNode->room;
    roomStack->top = roomNode->next;
    roomStack->count--;
    free(roomNode);
    return room;
}

int roomStack_clear(struct RoomStack* roomStack) {
    if (roomStack == NULL) return C_ERR_EINVAL;

    while (roomStack->top != NULL) {
        roomStack_pop(roomStack);
    }
    return C_ERR_OK;
}

bool roomStack_isEmpty(const struct RoomStack* roomStack) {
    return (roomStack == NULL || roomStack->top == NULL);
}