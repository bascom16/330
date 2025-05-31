#include <stdio.h>

#include "esp_log.h"
#include "board.h"
#include "game.h"
#include "config.h"
#include "graphics.h"
#include "nav.h"
#include "com.h"

#include "hw.h"
#include "lcd.h"
#include "joy.h"
#include "pin.h"

#define MSG_NEW_GAME "Welcome to Tic-Tac-Toe! Player X will begin."
#define MSG_NEXT_PLAYER_X "It is now Player X's turn."
#define MSG_NEXT_PLAYER_O "It is now Player O's turn."
#define MSG_WIN_X "Player X wins!"
#define MSG_WIN_O "Player O wins!"
#define MSG_DRAW "The game ends in a draw."

void game_init();

void start_new_game();

void switch_turn();

bool check_valid_mark();

void set_rc();

void process_mark();

bool check_end_game();

// debug
static const char *TAG = "lab05";

// uart buffers
static uint8_t rec_buf = 0;
static uint8_t send_buf = 0;

// buffer control
#define BUF_SHIFT 4
#define R_HEX 0xF0
#define C_HEX 0x0F

// track if a command has been received via uart
static int8_t r_rec;
static int8_t c_rec;
static bool rec_flag = false;

// loctation vars
static int8_t r = 0;
static int8_t c = 0;

// track current player turn
static mark_t current_turn;

// game state machine states
enum game_st_t {
    init_st,
    new_game_st,
    wait_mark_st,
    mark_st,
    wait_restart_st
};

// state machine current state
static enum game_st_t current_state;

// Initialize the state machine
void game_init() {
    current_state = init_st;
}

// Tick function
void game_tick() {
    // Transitions
    switch(current_state) {
        case init_st:
            current_state = new_game_st; // transition to new game
            break;
        case new_game_st:
            current_state = wait_mark_st; // transition to wait mark
            graphics_drawMessage(MSG_NEW_GAME, CONFIG_MESS_CLR, CONFIG_BACK_CLR);
            break;
        case wait_mark_st:
            // make a mark if A is pressed
            if (!pin_get_level(HW_BTN_A)) { // A press
                ESP_LOGI(TAG, "A pressed!");
                if (check_valid_mark()) { // validate & send
                    ESP_LOGI(TAG, "Press is valid!");
                    send_buf = (r << BUF_SHIFT) + (c & C_HEX);
                    com_write(&send_buf, sizeof(send_buf)); // send loc
                    ESP_LOGI(TAG, "Mark made");
                    current_state = mark_st;
                    break;
                }
            }
            // make a mark if a byte is received
            if (com_read(&rec_buf, sizeof(rec_buf))) {
                rec_flag = true;
                ESP_LOGI(TAG, "A received!");
                r_rec = (rec_buf & R_HEX) >> BUF_SHIFT;
                c_rec = rec_buf & C_HEX;
                if (check_valid_mark()) { // same check as local
                    ESP_LOGI(TAG, "Received is valid!");
                    current_state = mark_st;
                    break;
                }
            }
            current_state = wait_mark_st;
            break;
        case mark_st:
            if (check_end_game()) { // transition to restart state on end of game
                current_state = wait_restart_st;
            } else { // normal case. Switch turn
                current_state = wait_mark_st;
                switch_turn();
                graphics_drawMessage(current_turn == X_m ? MSG_NEXT_PLAYER_X : MSG_NEXT_PLAYER_O, CONFIG_MESS_CLR, CONFIG_BACK_CLR);
            }
            break;
        case wait_restart_st:
            if (!pin_get_level(HW_BTN_START)) { // restart game on start press
                current_state = new_game_st;
            } else {
                current_state = wait_restart_st;
            }
            break;
        default:
            printf("Error!");
    }

    // Actions
    switch(current_state) {
        case init_st:
            break;
        case new_game_st:
            start_new_game();
            break;
        case wait_mark_st:
            break;
        case mark_st:
            process_mark();
            ESP_LOGI(TAG, "Mark processed!");
            break;
        case wait_restart_st:
            break;
        default:
            printf("Error!");
    }
}

// flush buffer, reset display, turn, and set nav to center
void start_new_game() {
    // flush buffer
    while (com_read(&rec_buf, sizeof(rec_buf))) {}
    // clear board
    board_clear();
    // draw background
    lcd_fillScreen(CONFIG_BACK_CLR);
    graphics_drawGrid(CONFIG_GRID_CLR);
    // set turn to x
    current_turn = X_m;
    // set nav to center
    nav_set_loc(CONFIG_BOARD_R / 2, CONFIG_BOARD_C / 2);
}

// switch current turn
void switch_turn() {
    current_turn = (current_turn == X_m) ? O_m : X_m;
}

// process the button press by location to add mark
void process_mark() {
    if (current_turn == X_m) { // draw X
        graphics_drawX(r, c, CONFIG_MARK_CLR);
    } else if (current_turn == O_m) { // draw O
        graphics_drawO(r, c, CONFIG_MARK_CLR);
    }
    rec_flag = false;
}

// check if nav is valid location and set mark
bool check_valid_mark() {
    set_rc();
    return board_set(r, c, current_turn);
}

// sets r and c to received values of r & c or current location
void set_rc() {
    if (rec_flag == false) { // no location received, use local
        nav_get_loc(&r, &c);
    } else { // received location
        r = r_rec;
        c = c_rec;
    }
}

// returns true if game is over, false otherwise
bool check_end_game() {
    // check win
    if (board_winner(current_turn)) {
        graphics_drawMessage(current_turn == X_m ? MSG_WIN_X : MSG_WIN_O, CONFIG_MESS_CLR, CONFIG_BACK_CLR);
        return true;
    }
    // check draw
    if (board_mark_count() >= CONFIG_BOARD_SPACES ) {
        graphics_drawMessage(MSG_DRAW, CONFIG_MESS_CLR, CONFIG_BACK_CLR);
        return true;
    }
    return false;
}