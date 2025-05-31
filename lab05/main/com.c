#include "hw.h"
#include "pin.h"
#include "com.h"

#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#define UART_PORT UART_NUM_2

#define BAUD 115200
#define SRC_CLK 122

#define TX HW_EX8
#define RX HW_EX7

// Initialize the communication channel.
// Return zero if successful, or non-zero otherwise.
int32_t com_init(void) {
    const uart_port_t uart_num = UART_PORT;
    uart_config_t uart_config = { // configure UART config according to documentation
    .baud_rate = BAUD,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = UART_SCLK_DEFAULT,
    };

    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));

    // Set UART pins(TX: HW_EX8, RX: HW_EX8, RTS & CTS not used)
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, TX, RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // Install the driver
    const uint32_t rx_buffer_size = UART_HW_FIFO_LEN(PORT_NUM)*2;
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, rx_buffer_size, 0, 0, NULL, 0));
    // pull up pin 7
    pin_pullup(HW_EX7, true);

    return 0;
}

// Free resources used for communication.
// Return zero if successful, or non-zero otherwise.
int32_t com_deinit(void) {
    if (uart_is_driver_installed(UART_PORT)) { // ensure driver exists
        uart_driver_delete(UART_PORT);
    }
    return 0;
}

// Write data to the communication channel. Does not wait for data.
// *buf: pointer to data buffer
// size: size of data in bytes to write
// Return number of bytes written, or negative number if error.
int32_t com_write(const void *buf, uint32_t size) {
    // fn will not wait and returns num bytes written
    return uart_tx_chars(UART_PORT, buf, size);
}

// Read data from the communication channel. Does not wait for data.
// *buf: pointer to data buffer
// size: size of data in bytes to read
// Return number of bytes read, or negative number if error.
int32_t com_read(void *buf, uint32_t size) {
    return uart_read_bytes(UART_PORT, buf, size, 0);
}