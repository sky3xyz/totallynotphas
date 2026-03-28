#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int hunterArray_init(struct HunterArray *hunterArray) {
    if (hunterArray == NULL) {
        fprintf(stderr, "[ERROR] hunterArray_init: hunterArray was NULL\n");
        return C_ERR_EINVAL;
    }

    hunterArray->contains = 0;
    hunterArray->capacity = 1;
    hunterArray->data = (struct Hunter*) calloc(hunterArray->capacity, sizeof(*hunterArray->data));

    if (hunterArray->data == NULL) {
        fprintf(stderr, "[ERROR] hunterArray_init: hunterArray calloc failed\n");
        hunterArray->capacity = 0;
        return C_ERR_ENOMEM;
    }
    
    fprintf(stderr, "[INFO] hunterArray_init: hunterArray init done\n");
    return C_ERR_OK;
}

int hunterArray_canAppend(struct HunterArray* hunterArray) {
    if (hunterArray == NULL) {
        fprintf(stderr, "[ERROR] hunterArray_canAppend: hunterArray is NULL\n");
        return C_ERR_EINVAL;
    }

    if (hunterArray->contains < hunterArray->capacity) {
        return C_ERR_OK;
    }

    fprintf(stderr, "[ERROR] hunterArray_canAppend: hunterArray full.\n");
    return C_ERR_NOT_OK;
}

int hunterArray_append(struct HunterArray* hunterArray, struct Hunter* hunter) {
    if (hunterArray == NULL || hunter == NULL) {
        fprintf(stderr, "[ERROR] hunterArray_append: hunterArray and/or hunter was NULL.\n");
        return C_ERR_EINVAL;
    }

    if (hunterArray_canAppend(hunterArray) != C_ERR_OK) {
        int error = hunterArray_grow(hunterArray);
        if (error != C_ERR_OK) {
            fprintf(stderr, "[ERROR] hunterArray_append: hunterArray_grow: grow failed,\n");
            return error;
        }
    }
    hunterArray->data[hunterArray->contains] = *hunter;
    hunterArray->contains++;
    fprintf(stderr, "[INFO] hunterArray_append: appended hunter\n");
    return C_ERR_OK;
}

int hunterArray_grow(struct HunterArray *hunterArray) {
    if (hunterArray == NULL) {
        fprintf(stderr, "[ERROR] hunterArray_grow: hunterArray was NULL\n");
        return C_ERR_EINVAL;
    }

    int newCapacity = hunterArray->capacity + 2;

    struct Hunter *tmp = calloc(newCapacity, sizeof(*hunterArray->data));

    if (tmp == NULL) {
        fprintf(stderr, "[ERROR] hunterArray_grow: CALLOC failed.\n");
        return C_ERR_ENOMEM;
    }
    
    for (int i = 0; i < hunterArray->capacity; i++) {
        tmp[i] = hunterArray->data[i];
    }

    free(hunterArray->data);

    hunterArray->data = tmp;
    hunterArray->capacity = newCapacity;

    fprintf(stderr, "[INFO] hunterArray_grow: hunterArray grew to %d\n", newCapacity);
    return C_ERR_OK;
}

int hunterArray_clear(struct HunterArray *hunterArray) {
    if (hunterArray == NULL) {
        fprintf(stderr, "[ERROR] hunterArray_clear: hunterArray was NULL\n");
        return C_ERR_EINVAL;
    }

    free(hunterArray->data);
    hunterArray->data = NULL;
    hunterArray->contains = 0;
    hunterArray->capacity = 0;

    fprintf(stderr, "[INFO] hunterArray_clear: cleared hunterArray\n");
    return C_ERR_OK;
}