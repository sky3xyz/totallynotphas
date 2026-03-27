#include "defs.h"
#include "helpers.h"
#include <stddef.h>
#include <stdio.h>

void casefile_init(struct CaseFile* caseFile) {
    if (caseFile == NULL) {
        fprintf(stderr, "[ERROR] casefile_init: caseFile was NULL\n");
        return;
    }

    caseFile->collected = 0;
    caseFile->solved = false;

    if (sem_init(&caseFile->mutex, 0, 1) != C_ERR_OK) {
        fprintf(stderr, "[ERROR] casefile_init: caseFile->mutex failed to init\n");
        return;
    }

    fprintf(stderr, "[INFO] casefile_init: casefile init success\n");
}

void casefile_addEvidence(struct CaseFile* caseFile, enum EvidenceType ev) {
    if (caseFile == NULL) {
        fprintf(stderr, "[ERROR] casefile_addEvidence: caseFile was NULL\n");
        return;
    }

    if (sem_wait(&caseFile->mutex) != C_ERR_OK) {
        fprintf(stderr, "[ERROR] casefile_addEvidence: failed to sem_wait on caseFile->mutex\n");
        return;
    }

    evidence_set(&caseFile->collected, ev);
    if (evidence_is_valid_ghost(caseFile->collected)) {
        caseFile->solved = true;
    }

    if (sem_post(&caseFile->mutex) != C_ERR_OK) {
        fprintf(stderr, "[ERROR] casefile_addEvidence: failed to sem_post on caseFile->mutex");
        return;
    }

    fprintf(stderr, "[INFO] casefile_addEvidence: evidence successfully added");
}

