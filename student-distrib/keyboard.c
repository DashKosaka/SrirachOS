#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "syscall.h"
#include "filesys.h"

#define SCANCODE_ARRAY_SIZE 86

#define KEYBOARD_PORT 		0x60
#define MAX_PRESSED_CODE    0x58
#define MAX_RELEASED_CODE   0xD8
#define KEYBOARD_IRQ_NUM	1
#define PRESSED				1
#define NOT_PRESSED			0

#define UNIT_4_KB 			 0x1000

#define RELEASED_MASK		0x7F

#define DONT_PRINT			0xFF
#define ENTER_KEY			0x0A
#define BACKSPACE_KEY		0x0E
#define TAB_KEY				0x0F
#define L_CTL_KEY			0x1D
#define L_SHIFT_KEY			0x2A
#define L_KEY				0x26
#define C_KEY 				0x2E
#define T_KEY 				0x14
#define R_SHIFT_KEY			0x36
#define L_ALT_KEY			0x38
#define CAPSLOCK_KEY		0x3A
#define FUNCTION_START		0x3B
#define DOWN_ARROW			0X50
#define LEFT_ARROW			0X4B	
#define RIGHT_ARROW			0X4D
#define UP_ARROW			0X48
#define NULL_CHAR			'\0'

#define BUFSIZE				128

#define EXCP_RET			255

#define MAX_TERMINALS 		3
#define TERMINAL_1			0
#define TERMINAL_2			1
#define TERMINAL_3			2


/* Array indexed by scancode from by keyboard using Scancode Set 1.
 * All numbers and letters and puncutation represented. Indexes with
 * null char are not represented. Multiplied by two to make room for 
 * capital characters.
 */
static unsigned char scancode[SCANCODE_ARRAY_SIZE*2] = {
						  '\0','\0','1','2','3','4','5','6','7','8','9','0',
						  '-','=','\0', '\0', 'q', 'w','e','r','t','y','u',
						  'i','o','p','[',']','\n','\0','a','s','d','f','g',
						  'h','j','k','l',';','\'','`','\0','\\','z','x','c',
						  'v','b','n','m',',','.','/','\0','*','\0',' ','\0',
						  '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
						  '\0','\0','7','8','9','-','4','5','6','+','1','2',
						  '3','0','.','\0','\0',
						  '\0','\0','!','@','#','$','%','^','&','*','(',')',
						  '_','+','\0', '\0', 'Q', 'W','E','R','T','Y','U',
						  'I','O','P','{','}','\n','\0','A','S','D','F','G',
						  'H','J','K','L',':','"','~','\0','|','Z','X','C',
						  'V','B','N','M','<','>','?','\0','*','\0',' ','\0',
						  '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
						  '\0','\0','7','8','9','-','4','5','6','+','1','2',
						  '3','0','.','\0','\0'};


// keyboard_buf is the buffer of the current line (ended by new line).
// This buffer is updated with key presses.
static int8_t keyboard_buf[BUFSIZE];
// This buffer saves the last buffer along with the idx
static int8_t last_buf[BUFSIZE];
static int last_idx = 0;
// keyboard_buf_idx is the current index into keyboard_buf.
static int keyboard_buf_idx = 0;
// number of non null chars in the buffer
static int num_chars = 0;
// save_buf is the buffer used by terminal read to read input.
static int8_t save_buf[BUFSIZE];
// save_buf for terminal 1
static int8_t save_buf_1[BUFSIZE];
// save_buf for terminal 2
static int8_t save_buf_2[BUFSIZE];
// save_buf for terminal 3
static int8_t save_buf_3[BUFSIZE];
// global variable to keep track of capslock key
static int caps_lock_check = -1;
// global array keeping track of currently pressed keys
static unsigned char pressed_flags[SCANCODE_ARRAY_SIZE];
// global variable to keep track of which terminal is active
static int active_terminal;

/*	terminal_open()
 *
 *	INPUT:none
 *	OUTPUT:	none
 *	RETURN: 0 on success (always)
 *	DESCRIPTION: This function initializes save_buf and
 *				 keyboard_buf to 'empty' and returns 0 
 *				 every time.
 */
int32_t terminal_open(const uint8_t* filename) {
	// buffer is 'empty' if first index is null char
	keyboard_buf[0] = NULL_CHAR;
	last_buf[0] = NULL_CHAR;
	save_buf[0] = NULL_CHAR;
	save_buf_1[0] = NULL_CHAR;
	save_buf_2[0] = NULL_CHAR;
	save_buf_3[0] = NULL_CHAR;

	return 0;
}

/*	terminal_close()
 *
 *	INPUT:none
 *	OUTPUT:	none
 *	RETURN: -1 (always)
 *	DESCRIPTION: returns -1 every time
 */
int32_t terminal_close(int32_t fd) {
	return -1;
}

/*	terminal_read()
 *
 *	INPUT: fd - file descriptor
 *		   buf - buffer to put in the input ended with a newline
 * 		   nbytes - number of bytes to read (or up to new line)
 *	OUTPUT:	none
 *	RETURN: returns number of bytes read
 *	DESCRIPTION: This function waits for a newline to be pressed
 * 				 (unless one has already been pressed and not 
 *				 processed) and stores the information from save_buf
 *				 into the argument buf.
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes) {
	if (fd != 0) return -1;
	// flag to break out of while loop
	int flag = 0;
	while (1) {
		// switch statement based on active_terminal
		switch (return_active_terminal()) {
			case TERMINAL_1:
				// check if the running process corresponds to the active terminal
				if (check_read_process() == -1) break;
				if (save_buf_1[0] != NULL_CHAR) {
					buf = my_strcpy(buf, save_buf_1);
					save_buf_1[0] = NULL_CHAR;
					flag = 1;
				}
				break;
			case TERMINAL_2:
				// check if the running process corresponds to the active terminal
				if (check_read_process() == -1) break;
				if (save_buf_2[0] != NULL_CHAR) {
					buf = my_strcpy(buf, save_buf_2);
					save_buf_2[0] = NULL_CHAR;
					flag = 1;
				}
				break;
			case TERMINAL_3:
				// check if the running process corresponds to the active terminal
				if (check_read_process() == -1) break;
				if (save_buf_3[0] != NULL_CHAR) {
					buf = my_strcpy(buf, save_buf_3);
					save_buf_3[0] = NULL_CHAR;
					flag = 1;
				}
				break;
			default:
				break;
		}
		if (flag) break;
	}
	return my_strlen(buf);
}

/*	terminal_write()
 *
 *	INPUT: fd - file descriptor
 *		   buf - buffer to output to the terminal
 * 		   nbytes - number of bytes to write (or up to null char)
 *	OUTPUT:	writes to terminal screen
 *	RETURN: returns number of bytes written or -1 on error
 *	DESCRIPTION: This function outputs to the terminal screen a string
 * 				 represented by the argument buf.
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes) {
	if (fd != 1) return -1;
	cli();
	// Modified to print until nbytes have been reached (may need error check)
	if (my_puts((int8_t*)buf, nbytes) == nbytes) {
		sti();
		return nbytes;
	}
	sti();
	return -1;
}

/*  getScancode()
 *
 *  INPUT: none 
 *	OUTPUT: none
 *	RETURN: unsigned char representing the scancode from the keyboard
 *	DESCRIPTION: This function polls the keyboard port to 
 *				 receive a scancode representing a key.
 * 				 Called by getChar so that the returned
 * 				 scancode can be used to index the scancode
 *				 array.
 */
unsigned char getScancode() {
	// poll keyboard for scancode
	unsigned char c = inb(KEYBOARD_PORT);
	return c;
}

/*  getChar()
 *
 *  INPUT: none 
 *	OUTPUT: none
 *	RETURN: unsigned char representing the key pressed
 *	DESCRIPTION: This function calls getScancode to get
 *				 a scancode from the keyboard and indexes
 *				 into the scancode array and returns that
 *				 corresponding key. Also checks for special
 *				 keys such as shift or ctl.
 */
unsigned char getChar() {
	// get scancode from keyboard
	unsigned char c = getScancode();

	// if key was pressed
	if (c <= MAX_PRESSED_CODE) {
		// raise flag
		pressed_flags[c] = PRESSED;
		// check if CapsLock was pressed
		if (c == CAPSLOCK_KEY) {
			// turns into opposite state
			caps_lock_check *= -1;
			return DONT_PRINT;
		}
		// if ctl + L: clear screen and put cursor on top
		// this will happen no matter what other keys are pressed besides capslock
		if (pressed_flags[L_CTL_KEY] && c == L_KEY) {
			my_clear();
			// put current keyboard_buf
			terminal_write(1, terminals[active_terminal].save_buf, keyboard_buf_idx);
			return DONT_PRINT;
		} else if (pressed_flags[L_CTL_KEY] && c == C_KEY) {
			// Makeshift keyboard interrupt on ctrl + c
			// doesnt work
			printf("\nKeyboard interrupt!\n");
			terminals[active_terminal].save_buf[keyboard_buf_idx] = '\n';
			// We need to send a eoi before we halt or the bad things start to happen
			send_eoi(KEYBOARD_IRQ_NUM);
			system_halt(EXCP_RET);
			while(1){}
		} else if (pressed_flags[L_CTL_KEY] && c == T_KEY) {
			// Change the color of the text being printed
			//cycle_text_color();
			return DONT_PRINT;
		}
		// if alt + function key: change visual terminal to terminal f#
		if (pressed_flags[L_ALT_KEY] && (c >= FUNCTION_START) && (c < (FUNCTION_START + MAX_TERMINALS))) {
			// Change the terminal to # = (c - FUNCTION_START)
			send_eoi(KEYBOARD_IRQ_NUM);
			switch_terminal(c-FUNCTION_START);	
			return DONT_PRINT;		
		}
		// if horizontal arrow keys: update cursor 
		if (c == LEFT_ARROW) {
			return DONT_PRINT;
		} else if (c == RIGHT_ARROW) {
			return DONT_PRINT;
		}
		// if vertical arrow keys: update buffer
		if (c == UP_ARROW) {
			// copies the last buf into the current buf
			//keyboard_buf_idx = last_idx;
			//my_strcpy(keyboard_buf, last_buf);
			return DONT_PRINT;
		} else if (c == DOWN_ARROW) {
			// just resets the buffer completely
			return DONT_PRINT;
		}
		// if backspace: update terminal and keyboard_buf
		if (c == BACKSPACE_KEY && keyboard_buf_idx > 0) {
			_backspace();
			keyboard_buf_idx--;
			num_chars--;
			return DONT_PRINT;
		}
		// if a shift is held: update index into scancode array
		if (pressed_flags[L_SHIFT_KEY] || pressed_flags[R_SHIFT_KEY]) {
			if (caps_lock_check != 1) 
				c += SCANCODE_ARRAY_SIZE;
		} else if (caps_lock_check == 1) {
			c += SCANCODE_ARRAY_SIZE;
		}
		// autocomplete
		if (c == TAB_KEY) {
			keyboard_buf_idx = fs_search((uint8_t*) terminals[active_terminal].save_buf, keyboard_buf_idx);
		}
		// check if char is null
		unsigned char ret = scancode[c];
		if (ret == NULL_CHAR) return DONT_PRINT;
		return ret;
	}
	// if key was released
	else if (c <= MAX_RELEASED_CODE) {
		// lower flag
		pressed_flags[c & RELEASED_MASK] = NOT_PRESSED;
		return DONT_PRINT;
	}
	return DONT_PRINT;
}

/*  keyboard_handler()
 *
 *  INPUT: none 
 *	OUTPUT: prints a character to the screen
 *	RETURN: none
 *	DESCRIPTION: This function calls getChar to get the
 * 				 character pressed on the keyboard and
 *				 prints it to the screen. An end of interrupt
 *				 to the pic for they keyboard.
 */
void keyboard_handler(void) {
	// terminal on screen
	active_terminal = return_active_terminal();
	// buffer index for terminal
	keyboard_buf_idx = terminals[active_terminal].save_idx;
	// get char from keyboard
	unsigned char test = getChar();
	if (test != DONT_PRINT) {
		// if keyboard buffer filled up and enter not pressed: do not echo
		if (keyboard_buf_idx == BUFSIZE-1 && test != ENTER_KEY) {
			terminals[active_terminal].save_buf[keyboard_buf_idx] = '\n';
			send_eoi(KEYBOARD_IRQ_NUM);
			return;
		}
		// else update keyboard_buf
		else {
			terminals[active_terminal].save_buf[keyboard_buf_idx] = test;
			if (test != ENTER_KEY) {
				keyboard_buf_idx++;
				if(keyboard_buf_idx > num_chars) {
					num_chars = keyboard_buf_idx;
				}
			}
			// if enter pressed: save keyboard_buf into save_buf and reset keyboard_buf
			else {
				// switch statemetn to update correct terminal buffer
				switch (active_terminal) {
					case TERMINAL_1:
						my_strcpy((int8_t*)save_buf_1, (int8_t*)terminals[active_terminal].save_buf);
						break;
						
					case TERMINAL_2:
						my_strcpy((int8_t*)save_buf_2,(int8_t*)terminals[active_terminal].save_buf);
						break;
						
					case TERMINAL_3:
						my_strcpy((int8_t*)save_buf_3, (int8_t*)terminals[active_terminal].save_buf);
						break;
					
					default:
						break;
				}
				//my_strcpy(save_buf, terminals[active_terminal].save_buf);

				// save into last buf
				my_strcpy((int8_t*)last_buf, (int8_t*)terminals[active_terminal].save_buf); 
				last_idx = keyboard_buf_idx;

				// reset the keyboard buf
				keyboard_buf_idx = 0;
				num_chars = 0;
				terminals[active_terminal].save_buf[keyboard_buf_idx] = NULL_CHAR;
			}
		}
		key_putc(test);
	}
	// save idx
	terminals[active_terminal].save_idx = keyboard_buf_idx;
	send_eoi(KEYBOARD_IRQ_NUM);
}
