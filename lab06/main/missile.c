#include "missile.h"

// define missile SM states
enum missile_state_t {
    INIT_ST,
    IDLE_ST,
    MOVE_ST,
    EXPLODE_GROW_ST,
    EXPLODE_SHRINK_ST,
    IMPACT_ST,
};

/******************** Missile Init & Launch Functions ********************/

// Different _launch_ functions are used depending on the missile type.

// Initialize the missile as an idle missile. When initialized to the idle
// state, a missile doesn't appear nor does it move. The launch flag should
// also be set to false. Other missile_t members will be set up at launch.
void missile_init(missile_t *missile);

// Launch the missile as a player missile. This function takes an (x, y)
// destination of the missile (as specified by the user). The origin is the
// closest "firing location" to the destination (there are three firing
// locations evenly spaced along the bottom of the screen).
void missile_launch_player(missile_t *missile, coord_t x_dest, coord_t y_dest);

// Launch the missile as an enemy missile. This will randomly choose the
// origin and destination of the missile. The origin is somewhere near the
// top of the screen, and the destination is the very bottom of the screen.
void missile_launch_enemy(missile_t *missile);

// Launch the missile as a plane missile. This function takes the (x, y)
// location of the plane as an argument and uses it as the missile origin.
// The destination is randomly chosen along the bottom of the screen.
void missile_launch_plane(missile_t *missile, coord_t x_orig, coord_t y_orig);

/******************** Missile Control & Tick Functions ********************/

// Used to indicate that a moving missile should be detonated. This occurs
// when an enemy or a plane missile is located within an explosion zone.
void missile_explode(missile_t *missile) {
    missile->explode_me = true;
}

// Tick the state machine for a single missile.
void missile_tick(missile_t *missile);

/******************** Missile Status Functions ********************/

// Return the current missile position through the pointers *x,*y.
void missile_get_pos(missile_t *missile, coord_t *x, coord_t *y) {
    *x = missile->x_current;
    *y=missile->y_current;
}

// Return the missile type.
missile_type_t missile_get_type(missile_t *missile) {
    return missile->type;
}

// Return whether the given missile is moving.
bool missile_is_moving(missile_t *missile) {
    return missile->currentState == MOVE_ST;
}

// Return whether the given missile is exploding. If this missile
// is exploding, it can explode another intersecting missile.
bool missile_is_exploding(missile_t *missile) {
    return  missile->currentState == EXPLODE_GROW_ST || 
            missile->currentState == EXPLODE_SHRINK_ST;
}

// Return whether the given missile is idle.
bool missile_is_idle(missile_t *missile) {
    return missile->currentState == IDLE_ST;
}

// Return whether the given missile is impacted.
bool missile_is_impacted(missile_t *missile) {
    return missile->currentState == IMPACT_ST;
}

// Return whether an object (e.g., missile or plane) at the specified
// (x,y) position is colliding with the given missile. For a collision
// to occur, the missile needs to be exploding and the specified
// position needs to be within the explosion radius.
bool missile_is_colliding(missile_t *missile, coord_t x, coord_t y) {
    // idk yet
    return false
}