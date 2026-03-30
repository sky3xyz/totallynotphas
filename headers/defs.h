#ifndef DEFS_H
#define DEFS_H
// shared libraries between all files
#include <semaphore.h>
#include <stdbool.h>

/*
    You are free to rename all of the types and functions defined here.

    The ghost ID must remain the same for the validator to work correctly.
*/


#define C_ERR_OK 0
#define C_ERR_NO_EVIDENCE 14
#define C_ERR_NOT_OK -99
#define C_ERR_ENOMEM 12 // stolen from errno cmd / man 3 errno -- Cannot allocate memory
#define C_ERR_EINVAL 22 // stolen from errno cmd / man 3 errno -- Invalid argument
#define MAX_ROOM_NAME 64
#define MAX_HUNTER_NAME 64
#define MAX_ROOMS 24
#define MAX_ROOM_OCCUPANCY 8
#define MAX_CONNECTIONS 8
#define ENTITY_BOREDOM_MAX 15
#define HUNTER_FEAR_MAX 15
#define DEFAULT_GHOST_ID 68057
#define INITAL_HUNTER_ID 22001

typedef unsigned char EvidenceByte;
struct Room;
struct RoomNode;
struct Ghost;
struct Hunter;
struct HunterArray;

enum LogReason {
    LR_EVIDENCE = 0,
    LR_BORED = 1,
    LR_AFRAID = 2
};

enum EvidenceType {
    EV_EMF          = 1 << 0,
    EV_ORBS         = 1 << 1,
    EV_RADIO        = 1 << 2,
    EV_TEMPERATURE  = 1 << 3,
    EV_FINGERPRINTS = 1 << 4,
    EV_WRITING      = 1 << 5,
    EV_INFRARED     = 1 << 6,
};

enum GhostType {
    GH_POLTERGEIST  = EV_FINGERPRINTS | EV_TEMPERATURE | EV_WRITING,
    GH_THE_MIMIC    = EV_FINGERPRINTS | EV_TEMPERATURE | EV_RADIO,
    GH_HANTU        = EV_FINGERPRINTS | EV_TEMPERATURE | EV_ORBS,
    GH_JINN         = EV_FINGERPRINTS | EV_TEMPERATURE | EV_EMF,
    GH_PHANTOM      = EV_FINGERPRINTS | EV_INFRARED    | EV_RADIO,
    GH_BANSHEE      = EV_FINGERPRINTS | EV_INFRARED    | EV_ORBS,
    GH_GORYO        = EV_FINGERPRINTS | EV_INFRARED    | EV_EMF,
    GH_BULLIES      = EV_FINGERPRINTS | EV_WRITING     | EV_RADIO,
    GH_MYLING       = EV_FINGERPRINTS | EV_WRITING     | EV_EMF,
    GH_OBAKE        = EV_FINGERPRINTS | EV_ORBS        | EV_EMF,
    GH_YUREI        = EV_TEMPERATURE  | EV_INFRARED    | EV_ORBS,
    GH_ONI          = EV_TEMPERATURE  | EV_INFRARED    | EV_EMF,
    GH_MOROI        = EV_TEMPERATURE  | EV_WRITING     | EV_RADIO,
    GH_REVENANT     = EV_TEMPERATURE  | EV_WRITING     | EV_ORBS,
    GH_SHADE        = EV_TEMPERATURE  | EV_WRITING     | EV_EMF,
    GH_ONRYO        = EV_TEMPERATURE  | EV_RADIO       | EV_ORBS,
    GH_THE_TWINS    = EV_TEMPERATURE  | EV_RADIO       | EV_EMF,
    GH_DEOGEN       = EV_INFRARED     | EV_WRITING     | EV_RADIO,
    GH_THAYE        = EV_INFRARED     | EV_WRITING     | EV_ORBS,
    GH_YOKAI        = EV_INFRARED     | EV_RADIO       | EV_ORBS,
    GH_WRAITH       = EV_INFRARED     | EV_RADIO       | EV_EMF,
    GH_RAIJU        = EV_INFRARED     | EV_ORBS        | EV_EMF,
    GH_MARE         = EV_WRITING      | EV_RADIO       | EV_ORBS,
    GH_SPIRIT       = EV_WRITING      | EV_RADIO       | EV_EMF,
};

enum GhostAction {
    GA_IDLE=0,
    GA_HAUNT=1,
    GA_MOVE=2
};

enum HunterAction {
    HA_EVIDENCE = 0,
    HA_MOVE = 1,
    HA_CNGEQ = 2
};

struct CaseFile {
    EvidenceByte collected; // Union of all of the evidence bits collected between all hunters
    bool         solved;    // True when >=3 unique bits set
    sem_t        mutex;     // Used for synchronizing both fields when multithreading
};

struct RoomNode {
    struct Room *room;
    struct RoomNode *next;
};

struct RoomStack {
    struct RoomNode *top;
    int count;
};

struct Ghost {
    int id;
    enum GhostType type;
    struct Room* currentRoom;
    int boredom;
    bool in_simulation;
};

struct Hunter {
    char name[MAX_HUNTER_NAME];
    int id;

    struct Room* currentRoom;
    struct CaseFile* caseFile_ptr;

    enum EvidenceType currentDevice;
    struct RoomStack path;

    int fear;
    int boredom;
    enum LogReason exitReason;

    bool returningToVan;
    bool exitedSimulation;
};

struct HunterArray {
    struct Hunter* data;
    int contains;
    int capacity;
};

struct Room {
    sem_t mutex;

    // statics
    char name[MAX_ROOM_NAME];
    int id;
    EvidenceByte roomEvidence;
    bool exit;

    // only accessed by one thread (ghost)
    struct Ghost* currentGhost;

    // connected rooms
    struct Room* connectedRooms[MAX_CONNECTIONS];
    int connectedRoom_cnt;

    // hunters
    struct Hunter* presentHunters[MAX_ROOM_OCCUPANCY];
    int presentHunters_cnt;
};

struct House {
    struct Room rooms[MAX_ROOMS];
    struct Room* starting_room;
    struct HunterArray hunters;
    struct CaseFile caseFile;
    struct Ghost ghost;
    int room_count;
};


// hunter helper func.
/**  
 * @brief Initalize new Hunter.
 * @param[in] hunter Hunter to initalize.
 * @param[in] name Hunter Name.
 * @param[in] id Hunter ID.
 * @param[in] startRoom Hunter's starting Room.
 * @param[in] caseFile Pointer to CaseFile shared between Hunters.
 * @param[in] device Hunter's starting Evidence Device.
 */
void hunter_init(struct Hunter* hunter, const char* name, int id, struct Room* startRoom, struct CaseFile* caseFile, enum EvidenceType device);

/**
 * @brief Helper Function to update Hunter's state.
 * @param[in,out] hunter Hunter to update.
 */
void hunter_updateState(struct Hunter* hunter);

/**
 * @brief Attempt to gather evidence from the current room.
 * @param[in] hunter Hunter performing the action.
 * @returns C_ERR_OK on success. Otherwise a non-zero error code.
 */
int hunter_gatherEvidence(struct Hunter* hunter);

/**
 * @brief Move the Hunter to a destination room.
 * @param[in] hunter Hunter to move.
 * @param[in] destinationRoom Destination Room to move into.
 */
void hunter_move(struct Hunter* hunter, struct Room* destinationRoom);

/**
 * @brief Execute one turn for the Hunter.
 * @param[in] hunter Hunter to take a turn.
 */
void hunter_takeTurn(struct Hunter* hunter);

/**
 * @brief Swap Hunter's currently held equipment.
 * @param[in] hunter Hunter to perform the swap for.
 */
void hunter_equipmentSwap(struct Hunter* hunter);

/**
 * @brief Handle Hunter exiting the simulation.
 * @param[in] hunter Hunter exiting the simulation.
 */
void hunter_exitSimulation(struct Hunter* hunter);

/**
 * @brief Thread entry function for running a Hunter.
 * @param[in] arg Pointer passed to the thread.
 * @returns Thread exit value.
 */
void* hunter_thread(void* arg);

/**
 * @brief Handle the Hunter's return to the van.
 * @param[in] hunter Hunter returning to van.
 * @returns true if the Hunter should return/is returning to the van, false otherwise.
 */
bool hunter_returnToVan_handler(struct Hunter* hunter);

// ghost helper func.

/**
 * @brief Select a random Ghost type.
 * @returns Random GhostType value.
 */
enum GhostType ghost_randomGhostType();

/**
 * @brief Initialize a Ghost instance.
 * @param[out] ghost Ghost to initialize.
 * @param[in] house House the Ghost belongs to.
 */
void ghost_init(struct Ghost* ghost, struct House* house);

/**
 * @brief Move the Ghost to a destination room.
 * @param[in] ghost Ghost to move.
 * @param[in] destinationRoom Destination Room to move into.
 */
void ghost_move(struct Ghost* ghost, struct Room* destinationRoom);

/**
 * @brief Perform Ghost idle behavior for one simulation step/turn.
 * @param[in] ghost Ghost to idle.
 */
void ghost_idle(struct Ghost* ghost);

/**
 * @brief Perform Ghost haunting behavior for one simulation step/turn.
 * @param[in] ghost Ghost to haunt.
 */
void ghost_haunt(struct Ghost* ghost);

/**
 * @brief Execute one turn for the Ghost.
 * @param[in] ghost Ghost to take a turn.
 */
void ghost_takeTurn(struct Ghost* ghost);

/**
 * @brief Thread entry function for running a Ghost.
 * @param[in] arg Ghost pointer passed to the thread.
 * @returns Thread exit value (typically NULL).
 */
void* ghost_thread(void* arg);

/**
 * @brief Update boredom state and determine whether the Ghost should leave simulation.
 * @param[in] ghost Ghost to update.
 * @param[in] hunterConut Number of hunters currently in the simulation.
 * @returns true if the Ghost becomes bored enough to leave simulation, false otherwise.
 */
bool ghost_updateCheckBoredom(struct Ghost* ghost, int hunterConut);

// casefile helper func.

/**
 * @brief Initialize an empty CaseFile.
 * @param[out] caseFile CaseFile to initialize.
 */
void casefile_init(struct CaseFile* caseFile);

/**
 * @brief Add an evidence type to the shared CaseFile.
 * @param[in] caseFile CaseFile to modify.
 * @param[in] ev Evidence type to add.
 */
void casefile_addEvidence(struct CaseFile* caseFile, enum EvidenceType ev);

// evidence helper func.

/**
 * @brief Check whether a given evidence type is present in bits.
 * @param[in] bits Evidence bits.
 * @param[in] ev Evidence type to check for.
 * @returns true if ev is present in bits, false otherwise.
 */
bool evidence_has(EvidenceByte bits, enum EvidenceType ev);

/**
 * @brief Set an evidence type in bits.
 * @param[in,out] bits Evidence bits to modify.
 * @param[in] ev Evidence type to set.
 */
void evidence_set(EvidenceByte* bits, enum EvidenceType ev);

/**
 * @brief Clear an evidence type in bits.
 * @param[in,out] bits Evidence bits to modify.
 * @param[in] ev Evidence type to clear.
 */
void evidence_clear(EvidenceByte* bits, enum EvidenceType ev);

// deadlock prevention helper func.

/**
 * @brief Lock two rooms in an order that prevents deadlocks.
 * @param[in] roomOne First room to lock.
 * @param[in] roomTwo Second room to lock.
 * @returns C_ERR_OK on success, otherwise a non-zero error code.
 */
int deadlock_lockRooms(struct Room* roomOne, struct Room* roomTwo);

/**
 * @brief Unlocks two rooms in an order that prevents deadlocks.
 * @param[in] roomOne First room to unlock.
 * @param[in] roomTwo Second room to unlock.
 * @returns C_ERR_OK on success, otherwise a non-zero error code.
 */
int deadlock_unlockRooms(struct Room* roomOne, struct Room* roomTwo);

/**
 * @brief Decide which room should be locked first to avoid deadlocks.
 * @param[in] roomOne First candidate room.
 * @param[in] roomTwo Second candidate room.
 * @returns true if roomOne should be considered first, false if roomTwo should be first.
 */
bool deadlock_whoFirst(const struct Room* roomOne, const struct Room* roomTwo);
#endif // DEFS_H
