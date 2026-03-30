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


// house helper func.
/**
 * @brief Initializes House structure.
 * @param[in] house House structure to initalize.
 */
void house_init(struct House* house);
/**
 * @brief Return a random Room from the House.
 * @param[in] house House structure.
 * @returns Random Room struct from the House.
 */
struct Room* house_getRndRoom(struct House* house);
/**
 * @brief Clears a House and its members' memory
 * @param[in] house House structure.
 */
void house_clear(struct House* house);

// room helper func.
/**
 * @brief Initializes a Room structure.
 * @param[in] room Room structure.
 * @param[in] name Room Name.
 * @param[in] is_exit Is the room used to exit the simulation?
 */
void room_init(struct Room* room, const char* name, bool is_exit);
/**
 * @brief Adds evidence to Room.
 * @param[in] room Room to add evidence to.
 * @param[in] evidence EvidenceByte to add to Room.
 */
void room_addEvidence(struct Room* room, EvidenceByte evidence);
/**
 * @brief Adds Hunter to Room.
 * @param[in] room Room to add Hunter to.
 * @param[in] hunter Hunter to add to Room.
 * @returns true, if able to add Hunter to Room. false, if unable to add Hunter to Room.
 */
bool room_addHunter(struct Room* room, struct Hunter* hunter);
/**
 * @brief Removes Hunter from Room.
 * @param[in] room Room to remove Hunter from.
 * @param[in] hunter Hunter to remove from Room.
 * @returns C_ERR_OK, if Hunter is able to be removed from Room. C_ERR_NOT_OK, if Hunter is unable to be removed from Room.
 */
int room_removeHunter(struct Room* room, struct Hunter* hunter);
/**
 * @brief Creates a bi-directional link between two Rooms.
 * @param[in] a One of two Rooms to connect.
 * @param[in] b One of two Rooms to connect.
 * @returns C_ERR_OK, if Rooms are able to connect. C_ERR_NOT_OK, if Rooms are unable to be connected.
 */
int room_connect(struct Room* a, struct Room* b);
/**
 * @brief Checks if Room has reached maximum occupancy.
 * @param[in] room Room to check.
 * @returns true, if Room has reached maximum occupancy. false, if Room has not reached maximum occupancy.
 */
bool room_isFull(struct Room* room);


// roomStack helper func.
/**
 * @brief Initalizes a RoomStack.
 * @param[in] roomStack RoomStack to initalize.
 */
void roomStack_init(struct RoomStack* roomStack);
/**
 * @brief Adds a Room to the RoomStack.
 * @param[in] roomStack RoomStack to push Room to.
 * @param[in] room Room to push onto RoomStack.
 * @returns C_ERR_OK on success. C_ERR_ENOMEM on memory allocation failure. C_ERR_EINVAL when given an invalid RoomStack.
 */
int roomStack_push(struct RoomStack* roomStack, struct Room* room);
/**
 * @brief Pops and returns a Room from the top of the RoomStack.
 * @param[in] roomStack RoomStack to pop Room from.
 * @returns Room from top of the RoomStack. NULL if roomStack invalid or empty.
 */
struct Room* roomStack_pop(struct RoomStack* roomStack);
/**
 * @brief Clears and frees RoomStack's memory.
 * @param[in] roomStack RoomStack to clear.
 * @returns C_ERR_OK on success. C_ERR_EINVAL was given invalid RoomStack.
 */
int roomStack_clear(struct RoomStack* roomStack);
/**
 * @brief Checks if RoomStack is empty.
 * @param[in] roomStack RoomStack to check.
 * @returns true, if RoomStack is empty. false, otherwise.
 */
bool roomStack_isEmpty(const struct RoomStack* roomStack);

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

// hunterArray helper func.
/**
 * @brief Initialize a HunterArray to an empty state.
 * @param[in] hunterArray Pointer to the HunterArray to initialize.
 * @returns C_ERR_OK on success, non-zero on failure.
 */
int hunterArray_init(struct HunterArray* hunterArray);

/**
 * @brief Append a Hunter to the end of a HunterArray.
 * @param[in] hunterArray Pointer to the HunterArray to append to.
 * @param[in] hunter Pointer to the Hunter to append.
 * @returns C_ERR_OK on success, non-zero on failure.
 */
int hunterArray_append(struct HunterArray* hunterArray,
                        struct Hunter* hunter);

/**
 * @brief Grow the capacity of a HunterArray.
 * @param[in,out] hunterArray Pointer to the HunterArray whose capacity should grow.
 * @returns C_ERR_OK on success, non-zero on failure.
 */
int hunterArray_grow(struct HunterArray* hunterArray);

/**
 * @brief Clear a HunterArray, removing all stored Hunters.
 * @param[in,out] hunterArray Pointer to the HunterArray to clear.
 * @returns C_ERR_OK on success, non-zero on failure.
 */
int hunterArray_clear(struct HunterArray* hunterArray);

/**
 * @brief Check whether a HunterArray can append another Hunter without growing.
 * @param[in] hunterArray Pointer to the HunterArray to check.
 * @returns C_ERR_OK if the array can append, non-zero otherwise.
 */
int hunterArray_canAppend(struct HunterArray* hunterArray);

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
 * @param[in] arg Ghostointer passed to the thread.
 * @returns Thread exit value (typically NULL).
 */
void* ghost_thread(void* arg);

/**
 * @brief Update boredom state and determine whether the Ghost should change behavior.
 * @param[in,out] ghost Ghost to update.
 * @param[in] hunterConut Number of hunters currently in the simulation (spelling matches prototype).
 * @returns true if the Ghost becomes bored enough to act/change state, false otherwise.
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
 * @param[in,out] caseFile CaseFile to modify.
 * @param[in] ev Evidence type to add.
 */
void casefile_addEvidence(struct CaseFile* caseFile, enum EvidenceType ev);

// evidence helper func.
/**
 * @brief Check whether a given evidence type is present in a bitset.
 * @param[in] bits Evidence bitfield.
 * @param[in] ev Evidence type to check for.
 * @returns true if ev is present in bits, false otherwise.
 */
bool evidence_has(EvidenceByte bits, enum EvidenceType ev);

/**
 * @brief Set (enable) an evidence type in a bitset.
 * @param[in,out] bits Evidence bitfield to modify.
 * @param[in] ev Evidence type to set.
 */
void evidence_set(EvidenceByte* bits, enum EvidenceType ev);

/**
 * @brief Clear (disable) an evidence type in a bitset.
 * @param[in,out] bits Evidence bitfield to modify.
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
 * @brief Unlock two previously locked rooms.
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
