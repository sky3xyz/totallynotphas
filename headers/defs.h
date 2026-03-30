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
void hunter_init(struct Hunter* hunter, const char* name, int id, struct Room* startRoom, struct CaseFile* caseFile, enum EvidenceType device);
void hunter_updateState(struct Hunter* hunter);
int hunter_gatherEvidence(struct Hunter* hunter);
void hunter_move(struct Hunter* hunter, struct Room* destinationRoom);
void hunter_takeTurn(struct Hunter* hunter);
void hunter_equipmentSwap(struct Hunter* hunter);
void hunter_exitSimulation(struct Hunter* hunter);
void* hunter_thread(void* arg);
bool hunter_returnToVan_handler(struct Hunter* hunter);

// hunterArray helper func.
int hunterArray_init(struct HunterArray* hunterArray);
int hunterArray_append(struct HunterArray* hunterArray, struct Hunter* hunter);
int hunterArray_grow(struct HunterArray* hunterArray);
int hunterArray_clear(struct HunterArray* hunterArray);
int hunterArray_canAppend(struct HunterArray* hunterArray);

// ghost helper func.
enum GhostType ghost_randomGhostType();
void ghost_init(struct Ghost* ghost, struct House* house);
    //, const int id, enum GhostType type, struct Room* startRoom);
void ghost_move(struct Ghost* ghost, struct Room* destinationRoom);
void ghost_idle(struct Ghost* ghost);
void ghost_haunt(struct Ghost* ghost);
void ghost_takeTurn(struct Ghost* ghost);
void* ghost_thread(void* arg);
bool ghost_updateCheckBoredom(struct Ghost* ghost, int hunterConut);

// casefile helper func.
void casefile_init(struct CaseFile* caseFile);
void casefile_addEvidence(struct CaseFile* caseFile, enum EvidenceType ev);

// evidence helper func.
bool evidence_has(EvidenceByte bits, enum EvidenceType ev);
void evidence_set(EvidenceByte* bits, enum EvidenceType ev);
void evidence_clear(EvidenceByte* bits, enum EvidenceType ev);
/* from helpers.h
bool evidence_is_valid_ghost(EvidenceByte mask);
*/

// deadlock prevention helper func.
int deadlock_lockRooms(struct Room* roomOne, struct Room* roomTwo);
int deadlock_unlockRooms(struct Room* roomOne, struct Room* roomTwo);
bool deadlock_whoFirst(const struct Room* roomOne, const struct Room* roomTwo);

#endif // DEFS_H
