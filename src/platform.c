#include "platform.h"
#include <stdio.h>

#define JTAG_UART_BASE 0xFF201000
#define PIXEL_BUF_CTRL 0xFF203020
#define KEY_BASE       0xFF200050

// Hide the buffer inside the .c file so external code can't corrupt it directly
static short int buffer1[240][512];
static short int buffer2[240][512];

// POINTER CHANGE: This points to 16-bit color data, so use short int*
static volatile short int *pixel_buffer_start;

static volatile int *pixel_ctrl_ptr = (int *)PIXEL_BUF_CTRL;

/*
 * Puts a pixel to the CURRENT Back Buffer.
 * Includes the requested coordinate transformation:
 * (0,0) is in the center.
 * Y is inverted (positive is up).
 */
void put_pixel(int x, int y, short int color) {
    // 1. Bounds checking
    if (x > 160 || x < -159 || y > 119 || y < -120) return;

    // 2. Calculate the "Raw" VGA coordinates (0-319, 0-239)
    // Shift X so 0 becomes center (159)
    // Shift/Invert Y so 0 becomes center (119) and Up is positive
    int raw_x = 159 + x;
    int raw_y = 119 - y;

    // 3. Write to the current back buffer pointer
    // Formula: Base_Address + (Row * Width_Padding) + Column
    *(pixel_buffer_start + (raw_y * 512) + raw_x) = color;
}

void platform_init(void) {
    // Set the back buffer address
    *(pixel_ctrl_ptr + 1) = (int) &buffer1;
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = (short int *)(*(pixel_ctrl_ptr));
    platform_clear_screen();

    *(pixel_ctrl_ptr + 1) = (int) &buffer2;
    pixel_buffer_start = (short int *)(*(pixel_ctrl_ptr + 1));
    
}

/*
 * 1. Triggers the hardware to swap Front and Back buffers.
 * 2. Waits for the Vertical Sync (swap completion).
 * 3. Updates the pixel_buffer_start to point to the NEW Back Buffer.
 */
void platform_swap_buffers(void) {
    *(pixel_ctrl_ptr) = 1; // Trigger buffer swap
    
    wait_for_vsync();      // Block until swap is complete

    // Update global pointer to the NEW Back Buffer address
    // The hardware puts the address of the new back buffer in register offset +1
    pixel_buffer_start = (short int *)(*(pixel_ctrl_ptr + 1));
}

// Reads the raw value of the pushbutton data register
int platform_read_keys(void) {
    volatile int * key_ptr = (int *)KEY_BASE;
    return *key_ptr;
}

// Clears the CURRENT Back Buffer
void platform_clear_screen(void) {
    // We only need to clear the visible 320x240 area
    for (int y = 0; y < 240; y++) {
        // Calculate row offset once per row for speed
        volatile short int * row_ptr = pixel_buffer_start + (y * 512);
        
        for (int x = 0; x < 320; x++) {
             // Set pixel to black (0x0000)
             *(row_ptr + x) = 0x0000;
        }
    }
}

void wait_for_vsync(){
    int status;

    //*pixel_ctrl_ptr = 1;  // start synchronization process
    
    status = *(pixel_ctrl_ptr + 3);  // read status register

    while ((status & 0x01) != 0) {  // polling loop waiting for S bit to go to 0
        status = *(pixel_ctrl_ptr + 3);
    }
}

/*
 * Helper: Sends a single character to the JTAG UART.
 * modified to WAIT for space instead of dropping characters.
 */
void put_jtag(volatile int * JTAG_UART_ptr, char c) {
    int control;
    // Poll the control register until write space is available
    // The upper 16 bits (0xFFFF0000) hold the available space count
    do {
        control = *(JTAG_UART_ptr + 1); 
    } while ((control & 0xFFFF0000) == 0);

    *(JTAG_UART_ptr) = c;
}

/*
 * Public Function: Prints a string followed by a newline
 */
void printuart(const char *ptr) {
    volatile int *JTAG_UART_ptr = (int *)JTAG_UART_BASE;

    // Loop through the string until the null terminator
    while (*ptr != '\0') {
        put_jtag(JTAG_UART_ptr, *ptr);
        ptr++;
    }

    // Append the new line
    put_jtag(JTAG_UART_ptr, '\n'); 
}