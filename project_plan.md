## evidence.c
### data defs
**EvidenceByte** -> unsigned char
**CaseFile Struct** ->
 - [ ] ``EvidenceByte`` evidence
 - [ ]  ``sem_t`` mutex
 - [ ] ``boolean`` solved


## house.c 
### data defs
 - [ ] ``struct RoomStack rooms[MAX_ROOMS]``
 - [ ] ``struct HunterCollection hunters``
 - [ ] `` struct CaseFile *casefile_ptr``
 - [ ] ``struct Ghost hauntingGhost``

### ghost.c
#### data defs
**struct Ghost ghost**
 - [ ] ``int id``
 - [ ] ``enum GhostType type``
 - [ ] ``struct Room *room_ptr``
 - [ ] ``int boredom``
 - [ ] ``boolean exited``


### room.c
#### data defs
 - [ ] char name[]
 - [ ] struct Room* connected_rooms
 - [ ] struct 