#include "defs.h"
#include <stdio.h>


int hunterArray_init(struct HunterArray *hunterArray) {
    if (hunterArray == NULL) {
        fprintf(stderr, "[ERROR] hunterArray was NULL\n");
        return C_ERR_EINVAL;
    }

    hunterArray->contains = 0;
    hunterArray->capacity = 1;
    hunterArray->data = calloc(hunterArray->capacity, sizeof(*hunterArray->data));

    if (hunterArray->data == NULL) {
        fprintf(stderr, "[ERROR] HUNTERARRAY CALLOC FAILED\n");
        hunterArray->capacity = 0;
        return C_ERR_ENOMEM;
    }
    
    fprintf(stderr, "[INFO] HUNTERARRAY INIT\n");
    return C_ERR_OK;
}

int hunterArray_canAppend(struct HunterArray* hunterArray) {
    if (hunterArray == NULL) {
        fprintf(stderr, "[ERROR] NULL HUNTERARRAY PASSED INTO _CANAPPEND\n");
        return C_ERR_EINVAL;
    }

    if (hunterArray->contains < hunterArray->capacity) {
        return C_ERR_OK;
    }

    fprintf(stderr, "[ERROR] HUNTERARRAY FULL. CONTAINS=%d CAPACITY=%d\n", hunterArray->contains, hunterArray->capacity);
    return C_ERR_NOT_OK;
}

int hunterArray_append(struct HunterArray* hunterArray, struct Hunter* hunter) {
    if (hunterArray == NULL) {
        fprintf(stderr, "[ERROR] HUNTERARRAY NULL");
        return C_ERR_EINVAL;
    }

    if (hunterArray_canAppend(hunterArray) == C_ERR_ENOMEM && hunterArray_grow(&hunterArray) != C_ERR_OK) {
        return C_ERR_NOT_OK;
    }
    hunterArray->data[hunterArray->contains] = *hunter;
    return C_ERR_OK;
}
