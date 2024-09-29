#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "lib.h"

/* terminal_t 
 *
 * terminal_idx -- the index of the terminal in process_num 
 * x_cursor -- the x position of the cursor in the terminal
 * y_cursor -- the y position of the cursor in the terminal
 * color -- the assigned color of text in the terminal
 * save_buf -- the buffer used for keyboard input
 * save_idx -- the index into the save_buf
 * curr_task -- the current process that is running on that terminal
 * video_addr -- the assigned 4-kB page in memory that the terminal
 *				 writes to if its process is running in the background
 */
typedef struct terminal_desc {
	uint8_t terminal_idx;
	int x_cursor;
	int y_cursor;
	int color;
	uint8_t save_buf[128];
	int save_idx;
	int curr_task;
	uint32_t video_addr;
} terminal_t;

/* Array of terminal structs for 3 teriminals. */
static terminal_t terminals[3] __attribute__((unused));

/* Save the buffer */
void store_buf(int8_t* buf, int* idx);

/* Load the buffer */
void load_buf(int8_t* buf, int idx);

/* Function to initialize opening of the terminal.
 */
int32_t terminal_open(const uint8_t* filename);

/* Function to initialize closing of the terminal.
 */
int32_t terminal_close(int32_t fd);

/* Function to read line buffered input from the terminal
 * into buf argument.
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

/* Function to write string from buf onto terminal screen.
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

/* Function to get scancode from keyboard.
 * Code referenced from https://wiki.osdev.org/PS2_Keyboard 
 */
unsigned char getScancode();

/* Function to get character from scancode.
 * Code referenced from https://wiki.osdev.org/PS2_Keyboard 
 */
unsigned char getChar();

/* Keyboard handler function.
 */
void keyboard_handler(void);

#endif
