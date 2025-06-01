#include "plane.h"
#include "missile.h"
#include "lcd.h"
#include "config.h"

#define X_START (LCD_H / 4)
#define Y_START LCD_W

#define LEFT_BOUNDARY 0 // idk how it's measured

static int16_t x_pos;
static int16_t y_pos;

static missile_t *missile;

static bool explode_flag;

static uint8_t frame_counter;

// define missile SM states
enum plane_state_t {
    INIT_ST,
    IDLE_ST,
    FLY_ST,
};

static enum plane_state_t plane_state;

/******************** Plane Init Function ********************/

// Initialize the plane state machine. Pass a pointer to the missile
// that will be (re)launched by the plane. It will only have one missile.
void plane_init(missile_t *plane_missile) {
    missile = plane_missile;
    plane_state = INIT_ST;
    x_pos = X_START;
    y_pos = Y_START;
    explode_flag = false;
    frame_counter = CONFIG_PLANE_IDLE_TIME_TICKS;
}

/******************** Plane Control & Tick Functions ********************/

// Trigger the plane to explode.
void plane_explode(void) {
    explode_flag = true;
}

// State machine tick function.
void plane_tick(void) {
    // transitions
    switch (plane_state) {
        case INIT_ST:
            plane_state = IDLE_ST;
            break;
        case IDLE_ST:
            if (frame_counter >= CONFIG_PLANE_IDLE_TIME_TICKS) { // fly if time elapsed
                plane_state = FLY_ST;
            } else { // stay idle
                plane_state = IDLE_ST;
            }
            break;
        case FLY_ST:
            if (explode_flag || x_pos <= LEFT_BOUNDARY) {
                frame_counter = 0;
                explode_flag = false;
                plane_state = IDLE_ST;
            } else {
                plane_state = FLY_ST;
            }
            break;
        default:
            //someting wong 
    }

    // actions
    switch (plane_state) {
        case INIT_ST:
            break;
        case IDLE_ST:
            frame_counter++;
            break;
        case FLY_ST:
            x_pos++;
            lcd_drawTriangle(   x_pos, y_pos, 
                                x_pos + CONFIG_PLANE_WIDTH, y_pos + (CONFIG_PLANE_HEIGHT / 2), 
                                x_pos + CONFIG_PLANE_WIDTH, y_pos - (CONFIG_PLANE_HEIGHT / 2), 
                                CONFIG_COLOR_PLANE);
            if (x_pos == LCD_W / 2) {
                // plane is halfway across the screen, shoot
                missile_launch_plane(missile, x_pos, y_pos);
            }
            break;
        default:
            //someting wong 
    }
}

/******************** Plane Status Function ********************/

// Return the current plane position through the pointers *x,*y.
void plane_get_pos(coord_t *x, coord_t *y) {
    *x = x_pos;
    *y = y_pos;
}

// Return whether the plane is flying.
bool plane_is_flying(void) {
    return plane_state == FLY_ST;
}