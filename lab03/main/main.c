#include "lcd.h"
#include "pin.h"
#include "watch.h"
#include "hw.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "lab07";

// timer tick vars
volatile uint64_t timer_ticks = 0;
volatile bool running = false;

// alarm defs
#define RESOLUTION (1 * 1000 * 1000) // 1MHz, 1 tick = 1us
#define ALARM_COUNT 10000

static bool timer_on_alarm_cb(gptimer_handle_t, const gptimer_alarm_event_data_t *, void *);

// timing vars
volatile int64_t isr_max; // Maximum ISR execution time (us)
volatile int32_t isr_cnt; // Count of ISR invocations 

#define PRINT_ISR_TIME_DELAY (100 * 5) // 5s at 100 Hz

// application main function for ESP_32
void app_main(void) {
    int64_t start, finish;

    // initalize hardware pins for buttons
    start = esp_timer_get_time();
    pin_init();
    finish = esp_timer_get_time();
    printf("Configure I/O pins:%lld microseconds\n", finish-start);

    // initialize gptimer
    start = esp_timer_get_time();
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = RESOLUTION, // 1MHz, 1 tick = 1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    // initalize alarm
    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0, // counter will reload with 0 on alarm event
        .alarm_count = ALARM_COUNT, // period = 1s @resolution 1MHz
        .flags.auto_reload_on_alarm = true, // enable auto-reload
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

    // start timer
    gptimer_event_callbacks_t cbs = {
    .on_alarm = timer_on_alarm_cb, // register user callback
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    ESP_ERROR_CHECK(gptimer_start(gptimer));
    finish = esp_timer_get_time();
    printf("Configure timer:%lld microseconds\n", finish-start);

    // Log
    start = esp_timer_get_time();
    ESP_LOGI(TAG, "Stopwatch update");
    finish = esp_timer_get_time();
    printf("ESP Log:%lld microseconds\n", finish-start);

    lcd_init(); // Initialize LCD display
    watch_init(); // Initialize stopwatch face
    for (;;) { // forever update loop
        watch_update(timer_ticks);
        if (isr_cnt >= PRINT_ISR_TIME_DELAY) {
            // print max ISR time every 5 seconds
            printf("ISR max time:%lld microseconds\n", isr_max);
            isr_max = 0;
            isr_cnt = 0;
        }
    }
}

// initialize the hardware pins for buttons a, b, and start
// returns 0 if successful, nonzero otherwise
uint8_t pin_init() {
    return
    // A
    pin_reset(HW_BTN_A) ||
    pin_input(HW_BTN_A, true) ||

    // B
    pin_reset(HW_BTN_B) ||
    pin_input(HW_BTN_B, true) ||

    // START
    pin_reset(HW_BTN_START) ||
    pin_input(HW_BTN_START, true);
}

// alarm function
static bool timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    int64_t start = esp_timer_get_time();
    if (!pin_get_level(HW_BTN_A)) { // if A, clock is running
        running = true;
    } else if (!pin_get_level(HW_BTN_B)) { // if B, clock stops
        running = false;
    } else if (!pin_get_level(HW_BTN_START)) { // if START, clock stops and resets
        running = false;
        timer_ticks = 0;
    }
    if (running) { // increment timer ticks while running
        timer_ticks++;
    }
    int64_t finish = esp_timer_get_time();
    isr_max = ((finish - start) >= isr_max) ? (finish - start) : isr_max;
    isr_cnt++;
    return false;
}