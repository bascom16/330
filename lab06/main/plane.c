#include "plane.h"

#include "lcd.h"

#define X_START (LCD_H / 4)
#define Y_START LCD_W

#define FRAMES_OFFSCREEN 16
#define LEFT_BOUNDARY 0 // idk how it's measured

#define LENGTH 6
#define HEIGHT 4

static int16_t x_pos;
static int16_t y_pos;

static bool explode_flag;

static uint8_t frame_counter;

void shoot();

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
    plane_state = INIT_ST;
    x_pos = X_START;
    y_pos = Y_START;
    explode_flag = false;
    frame_counter = FRAMES_OFFSCREEN;
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
            if (frame_counter >= FRAMES_OFFSCREEN) { // fly if time elapsed
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
                                x_pos + LENGTH, y_pos + (HEIGHT/2), 
                                x_pos + LENGTH, y_pos - (HEIGHT/2), 
                                WHITE);
            shoot();
            break;
        default:
            //someting wrong 
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

// missile control
void shoot() {
    // 1 check if missile exists
    //      if so do nothing
    //      if not, 2 check if time has elapsed
    //          if so, 3 shoot
    //          otherwise 4 count up
}