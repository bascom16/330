#include <math.h>
#include <stdlib.h>

#include "missile.h"
#include "config.h"

#define RAND_SEED 42
#define FIRE_LOC_1 (LCD_W / 4)
#define FIRE_LOC_2 (LCD_W / 2) 
#define FIRE_LOC_3 (LCD_W * 3 / 4)
#define FIRE_LOC_BORDER_L (LCD_W * 3 / 8)
#define FIRE_LOC_BORDER_R (LCD_W * 5 / 8)

// define missile SM states
enum missile_state_t {
    INIT_ST,
    IDLE_ST,
    MOVE_ST,
    EXPLODE_GROW_ST,
    EXPLODE_SHRINK_ST,
    IMPACT_ST,
};

void finalize_launch(missile_t *);

color_t get_color(missile_type_t);

/******************** Missile Init & Launch Functions ********************/

// Different _launch_ functions are used depending on the missile type.

// Initialize the missile as an idle missile. When initialized to the idle
// state, a missile doesn't appear nor does it move. The launch flag should
// also be set to false. Other missile_t members will be set up at launch.
void missile_init(missile_t *missile) {
    missile->currentState = IDLE_ST;
    missile->launch = false;
}

// Launch the missile as a player missile. This function takes an (x, y)
// destination of the missile (as specified by the user). The origin is the
// closest "firing location" to the destination (there are three firing
// locations evenly spaced along the bottom of the screen).
void missile_launch_player(missile_t *missile, coord_t x_dest, coord_t y_dest) {
    if (x_dest < FIRE_LOC_BORDER_L) { // if dest is on the left 3/8 of the screen, left fire location
        missile->x_origin = FIRE_LOC_1;
    } else if (x_dest < FIRE_LOC_BORDER_R) { // if dest is in the middle 1/4 of the screen, middle fire location
        missile->x_origin = FIRE_LOC_2;
    } else { // if dest is on the right 3/8 of the screen, right fire location
        missile->x_origin = FIRE_LOC_3;
    }
    missile->y_origin = LCD_H; // bottom of screen
    missile->x_dest = x_dest;
    missile->y_dest = y_dest;
    finalize_launch(missile);
}

// Launch the missile as an enemy missile. This will randomly choose the
// origin and destination of the missile. The origin is somewhere near the
// top of the screen, and the destination is the very bottom of the screen.
void missile_launch_enemy(missile_t *missile) {
    srand(RAND_SEED);
    // origin is top of screen with random x value
    missile->x_origin = (rand() % LCD_W);
    missile->y_origin = 0;
    // destination is bottom of screen with random x value
    missile->x_dest = (rand() % LCD_W);
    missile->y_dest = LCD_H;
    finalize_launch(missile);
}

// Launch the missile as a plane missile. This function takes the (x, y)
// location of the plane as an argument and uses it as the missile origin.
// The destination is randomly chosen along the bottom of the screen.
void missile_launch_plane(missile_t *missile, coord_t x_orig, coord_t y_orig) {
    srand(RAND_SEED);
    // origin is plane location
    missile->x_origin = x_orig;
    missile->y_origin = y_orig;
    // destination is bottom of screen with random x value
    missile->x_dest = (rand() % LCD_W);
    missile->y_dest = LCD_H;
    finalize_launch(missile);
}

/******************** Missile Control & Tick Functions ********************/

// Used to indicate that a moving missile should be detonated. This occurs
// when an enemy or a plane missile is located within an explosion zone.
void missile_explode(missile_t *missile) {
    missile->explode_me = true;
}

// Tick the state machine for a single missile.
void missile_tick(missile_t *missile) {
    // transitions
    switch (missile->currentState) {
        case INIT_ST:
            missile->launch = false;
            missile->currentState = IDLE_ST;
            break;
        case IDLE_ST:
            if (missile->launch) { // go to move state if launch flag is true
                missile->currentState = MOVE_ST;
            } else {
                missile->currentState = IDLE_ST;
            }
            break;
        case MOVE_ST:
            if (missile->explode_me) { // go to exp state if exp flag is true, go to impact if y coord is at bottom
                missile->currentState = EXPLODE_GROW_ST;
            } else if (missile->y_current <= LCD_H) {
                missile->currentState = IMPACT_ST;
            } else {
                missile->currentState = MOVE_ST;
            }
            break;
        case EXPLODE_GROW_ST:
            if (missile->radius >= CONFIG_EXPLOSION_MAX_RADIUS) { // go to shrink if rad is maxxed out
                missile->currentState = EXPLODE_SHRINK_ST;
            } else {
                missile->currentState = EXPLODE_GROW_ST;
            }
            break;
        case EXPLODE_SHRINK_ST:
            if (missile->radius <= 0) { // go back to idle when rad is 0
                missile->currentState = IDLE_ST;
            } else {
                missile->currentState = EXPLODE_SHRINK_ST;
            }
            break;
        case IMPACT_ST:
            missile->currentState = EXPLODE_GROW_ST;
            break;
        default:
            // uh oh
    };

    // actions
    switch (missile->currentState) {
        case INIT_ST:
            break;
        case IDLE_ST:
            break;
        case MOVE_ST:
            missile->length += (missile->type == MISSILE_TYPE_PLAYER) 
                ? CONFIG_PLAYER_MISSILE_DISTANCE_PER_TICK : CONFIG_ENEMY_MISSILE_DISTANCE_PER_TICK;
            float fraction = missile->length / missile->total_length;
            missile->x_current = missile->x_origin + fraction * (missile->x_dest = missile->x_origin);
            missile->y_current = missile->y_origin + fraction * (missile->y_dest = missile->y_origin);
            lcd_drawLine(missile->x_origin, missile->y_origin, missile->x_current, missile->y_current, get_color(missile->type));
            break;
        case EXPLODE_GROW_ST:
            // increase radius
            missile->radius += CONFIG_EXPLOSION_RADIUS_CHANGE_PER_TICK;
            // draw circle
            lcd_fillCircle(missile->x_current, missile->y_current, missile->radius, get_color(missile->type));
            break;
        case EXPLODE_SHRINK_ST:
            // decrease radius
            missile->radius -= CONFIG_EXPLOSION_RADIUS_CHANGE_PER_TICK;
            // draw circle
            lcd_fillCircle(missile->x_current, missile->y_current, missile->radius, get_color(missile->type));
            break;
        case IMPACT_ST:
            // increment impact counter
            break;
        default:
            // uh oh
    };
}

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
    // check if missile is exploding
    if (!missile_is_exploding(missile)) {
        return false;
    }
    // check if point is inside radius of explosion
    int32_t dx = missile->x_current - x;
    int32_t dy = missile->y_current - y;
    return (pow(dy, 2) + pow(dx, 2)) <= pow(missile->radius, 2);
}

/******************** Missile Helper Functions ********************/

// initialize values before launch. To be called by individual launch functions.
void finalize_launch(missile_t *missile) {
    missile->length = 0;
    missile->launch = true;
    missile->explode_me = false;
    missile->total_length = sqrtf(pow((missile->y_dest - missile->y_origin), 2) 
                                + pow((missile->x_dest - missile->x_origin), 2));
    missile->x_current = missile->x_origin;
    missile->y_current = missile->y_origin;
}

// return missile color based on missile type
color_t get_color(missile_type_t type) {
    switch (type) { // from missile type
        case MISSILE_TYPE_ENEMY:
            return CONFIG_COLOR_ENEMY_MISSILE;
        case MISSILE_TYPE_PLANE:
            return CONFIG_COLOR_PLANE_MISSILE;
        case MISSILE_TYPE_PLAYER:
            return CONFIG_COLOR_PLAYER_MISSILE;
        default:
            return CONFIG_COLOR_BACKGROUND;
    }
}