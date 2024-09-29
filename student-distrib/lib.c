/* lib.c - Some basic library functions (printf, strlen, etc.)
 * vim:ts=4 noexpandtab */

#include "lib.h"
#include "syscall.h"

#define VIDEO       0xB8000
#define NUM_COLS    80
#define NUM_ROWS    25

#define BLACK       0x0
#define BLUE        0x1
#define GREEN       0x2
#define CYAN        0x3
#define RED         0x4
#define MAGENTA     0x5
#define BROWN       0x6
#define GRAY        0x7
#define DARK_GRAY   0x8
#define LIGHT_BLUE  0x9
#define LIGHT_GREEN 0xA
#define LIGHT_CYAN  0xB
#define LIGHT_RED   0xC
#define LIGHT_MAGENTA  0xD
#define YELLOW       0xE
#define WHITE       0xF


//set your favorite colors(choose from above)
//#define ATTRIB ((BACKGROUND_GOES_HERE)<<4) | (FOREGROUND_GOES_HERE)
#define ATTRIB ((BLACK)<<4) | (LIGHT_CYAN)

//#define ATTRIB 0x7 //uncomment here to return to default which is black and gray
static char text_color = ((BLACK)<<4) | (GRAY);

static int screen_x;
static int screen_y;
static char* video_mem = (char *)VIDEO;

/* void cycle_text_color
 * INPUTS: none
 * OUTPUTS: new text color
 * RETURNS: none
 * DESCRIPTION: change the text color to something (hopefully) more appealing
 */
void cycle_text_color(int terminal) {
  //  char next_color = ((text_color & 0x0F) + 1) % 0x10;
    //text_color &= 0xF0;
    
	//text_color |= next_color;
	
	
	text_color = ((BLACK)<<4) | (GRAY);
	
	// if(terminal == 1){
		
		// text_color = ((BLACK)<<4) | (LIGHT_CYAN);
		
	// }
	// else if(terminal ==2){
		
		// text_color = ((BLACK)<<4) | (LIGHT_RED);
	// }
	// else{
		// text_color = ((BLACK)<<4) | (LIGHT_GREEN);

	// }
	
}


/* void clear(void);
 * Inputs: void
 * Return Value: none
 * Function: Clears video memory */
void clear(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = text_color;
    }
}
/* void my_clear(void);
 * Inputs: void
 * Return Value: none
 * Function: Clears video memory */
void my_clear(void) {
    int32_t i;
    screen_x=0;
    screen_y=0;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = text_color;
    }
	update_cursor();
}

/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output. */
int32_t printf(int8_t *format, ...) {

    /* Pointer to the format string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            putc('%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    puts(conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    puts(&conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                int8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    itoa(-value, &conv_buf[1], 10);
                                } else {
                                    itoa(value, conv_buf, 10);
                                }
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            putc((uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            puts(*((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                putc(*buf);
                break;
        }
        buf++;
    }
    return (buf - format);
}


/* fs_printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output. */
int32_t fs_printf(int8_t *format, ...) {

    /* Pointer to the format string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            putc('%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    puts(conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    puts(&conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                int8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    itoa(-value, &conv_buf[1], 10);
                                } else {
                                    itoa(value, conv_buf, 10);
                                }
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            putc((uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            fs_puts(*((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                putc(*buf);
                break;
        }
        buf++;
    }
    return (buf - format);
}

/* int32_t puts(int8_t* s);
 *   Inputs: int_8* s = pointer to a string of characters
 *   Return Value: Number of bytes written
 *    Function: Output a string to the console */
int32_t puts(int8_t* s) {
    register int32_t index = 0;
    while (s[index] != '\0') {
        putc(s[index]);
        index++;
    }
    return index;
}

/* int32_t fs_puts(int8_t* s);
 *   Inputs: int_8* s = pointer to a string of characters
 *   Return Value: Number of bytes written
 *    Function: Output a string to the console */
int32_t fs_puts(int8_t* s) {
    register int32_t index = 0;
    register int32_t count = 0;
    while (s[index] != '\0' && count < 32) {
        putc(s[index]);
        index++;
        count++;
    }
    return index;
}

/* void putc(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 *  Function: Output a character to the console */
void putc(uint8_t c) {
	//char* video_addr = video_return();
	//int x = x_return();
	//int y = y_return();
    if(c == '\n' || c == '\r') {
        //screen_y++;
		//above line was replaced by below if else statement
		if (screen_y == NUM_ROWS-1) {
				_scroll_down();
		}
		else {
			screen_y++;
		}
        screen_x = 0;
    } else {
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1)) = c;
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1) + 1) = text_color;
        screen_x++;
        screen_x %= NUM_COLS;
        screen_y = (screen_y + (screen_x / NUM_COLS)) % NUM_ROWS;
		// if statement was added by matt
		if (screen_x == 0) {
			if (screen_y == NUM_ROWS-1) {
				_scroll_down();
			}
			else {
				screen_y++;
			}
		}
    }
	// update cursor position added by matt
	update_cursor();
}

/* int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix);
 * Inputs: uint32_t value = number to convert
 *            int8_t* buf = allocated buffer to place string in
 *          int32_t radix = base system. hex, oct, dec, etc.
 * Return Value: number of bytes written
 * Function: Convert a number to its ASCII representation, with base "radix" */
int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix) {
    static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int8_t *newbuf = buf;
    int32_t i;
    uint32_t newval = value;

    /* Special case for zero */
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    /* Go through the number one place value at a time, and add the
     * correct digit to "newbuf".  We actually add characters to the
     * ASCII string from lowest place value to highest, which is the
     * opposite of how the number should be printed.  We'll reverse the
     * characters later. */
    while (newval > 0) {
        i = newval % radix;
        *newbuf = lookup[i];
        newbuf++;
        newval /= radix;
    }

    /* Add a terminating NULL */
    *newbuf = '\0';

    /* Reverse the string and return */
    return strrev(buf);
}

/* int8_t* strrev(int8_t* s);
 * Inputs: int8_t* s = string to reverse
 * Return Value: reversed string
 * Function: reverses a string s */
int8_t* strrev(int8_t* s) {
    register int8_t tmp;
    register int32_t beg = 0;
    register int32_t end = strlen(s) - 1;

    while (beg < end) {
        tmp = s[end];
        s[end] = s[beg];
        s[beg] = tmp;
        beg++;
        end--;
    }
    return s;
}

/* uint32_t strlen(const int8_t* s);
 * Inputs: const int8_t* s = string to take length of
 * Return Value: length of string s
 * Function: return length of string s */
uint32_t strlen(const int8_t* s) {
    register uint32_t len = 0;
    while (s[len] != '\0')
        len++;
    return len;
}

/* void* memset(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive bytes of pointer s to value c */
void* memset(void* s, int32_t c, uint32_t n) {
    c &= 0xFF;
    asm volatile ("                 \n\
            .memset_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memset_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memset_aligned \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memset_top     \n\
            .memset_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     stosl           \n\
            .memset_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memset_done    \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%edx       \n\
            jmp     .memset_bottom  \n\
            .memset_done:           \n\
            "
            :
            : "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_word(void* s, int32_t c, uint32_t n);
 * Description: Optimized memset_word
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set lower 16 bits of n consecutive memory locations of pointer s to value c */
void* memset_word(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosw           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_dword(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive memory locations of pointer s to value c */
void* memset_dword(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosl           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memcpy(void* dest, const void* src, uint32_t n);
 * Inputs:      void* dest = destination of copy
 *         const void* src = source of copy
 *              uint32_t n = number of byets to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of src to dest */
void* memcpy(void* dest, const void* src, uint32_t n) {
    asm volatile ("                 \n\
            .memcpy_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memcpy_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memcpy_aligned \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memcpy_top     \n\
            .memcpy_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     movsl           \n\
            .memcpy_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memcpy_done    \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%edx       \n\
            jmp     .memcpy_bottom  \n\
            .memcpy_done:           \n\
            "
            :
            : "S"(src), "D"(dest), "c"(n)
            : "eax", "edx", "memory", "cc"
    );
    return dest;
}

/* void* memmove(void* dest, const void* src, uint32_t n);
 * Description: Optimized memmove (used for overlapping memory areas)
 * Inputs:      void* dest = destination of move
 *         const void* src = source of move
 *              uint32_t n = number of byets to move
 * Return Value: pointer to dest
 * Function: move n bytes of src to dest */
void* memmove(void* dest, const void* src, uint32_t n) {
    asm volatile ("                             \n\
            movw    %%ds, %%dx                  \n\
            movw    %%dx, %%es                  \n\
            cld                                 \n\
            cmp     %%edi, %%esi                \n\
            jae     .memmove_go                 \n\
            leal    -1(%%esi, %%ecx), %%esi     \n\
            leal    -1(%%edi, %%ecx), %%edi     \n\
            std                                 \n\
            .memmove_go:                        \n\
            rep     movsb                       \n\
            "
            :
            : "D"(dest), "S"(src), "c"(n)
            : "edx", "memory", "cc"
    );
    return dest;
}

/* int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n)
 * Inputs: const int8_t* s1 = first string to compare
 *         const int8_t* s2 = second string to compare
 *               uint32_t n = number of bytes to compare
 * Return Value: A zero value indicates that the characters compared
 *               in both strings form the same string.
 *               A value greater than zero indicates that the first
 *               character that does not match has a greater value
 *               in str1 than in str2; And a value less than zero
 *               indicates the opposite.
 * Function: compares string 1 and string 2 for equality */
int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n) {
    int32_t i;
    for (i = 0; i < n; i++) {
        if ((s1[i] != s2[i]) || (s1[i] == '\0') /* || s2[i] == '\0' */) {

            /* The s2[i] == '\0' is unnecessary because of the short-circuit
             * semantics of 'if' expressions in C.  If the first expression
             * (s1[i] != s2[i]) evaluates to false, that is, if s1[i] ==
             * s2[i], then we only need to test either s1[i] or s2[i] for
             * '\0', since we know they are equal. */
            return s1[i] - s2[i];
        }
    }
    return 0;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 * Return Value: pointer to dest
 * Function: copy the source string into the destination string */
int8_t* strcpy(int8_t* dest, const int8_t* src) {
    int32_t i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src, uint32_t n)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 *                uint32_t n = number of bytes to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of the source string into the destination string */
int8_t* strncpy(int8_t* dest, const int8_t* src, uint32_t n) {
    int32_t i = 0;
    while (src[i] != '\0' && i < n) {
        dest[i] = src[i];
        i++;
    }
    while (i < n) {
        dest[i] = '\0';
        i++;
    }
    return dest;
}

/* void test_interrupts(void)
 * Inputs: void
 * Return Value: void
 * Function: increments video memory. To be used to test rtc */
void test_interrupts(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        video_mem[i << 1]++;
    }
}

/**********************************/
/* Functions below added by Matt. */
/**********************************/

/* int8_t* my_strcpy(int8_t* dest, const int8_t* src)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 * Return Value: pointer to dest
 * Function: copy the source string into the destination string
 * 			 -- depends on new line
 */
int8_t* my_strcpy(int8_t* dest, const int8_t* src) {
    int32_t i = 0;
    while (src[i] != '\n') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\n';
    return dest;
}

/* uint32_t my_strlen(const int8_t* s);
 * Inputs: const int8_t* s = string to take length of
 * Return Value: length of string s
 * Function: return length of string s -- depends on
 * 			 new line */
uint32_t my_strlen(const int8_t* s) {
    register uint32_t len = 0;
    while (s[len] != '\n')
        len++;
	len++;
    return len;
}

#define ASCII_SPACE	0x20
/* void _backspace();
 * Inputs: none
 * Return Value: none
 * Function: removes the previous char if possible
 * 			 and updates screen_x and screen_y
 */
// void _backspace() {
	// if (screen_x == 0 && screen_y == 0) {
		// update_cursor();
		// return;
	// }
	// if (screen_x == 0) {
		// screen_x = NUM_COLS-1;
		// screen_y--;
		// putc(ASCII_SPACE);
		// screen_x = NUM_COLS-1;
		// screen_y--;
	// }
	// else {
		// screen_x--;
		// putc(ASCII_SPACE);
		// screen_x--;
	// }
	// update_cursor();
// }
void _backspace() {
	screen_x = x_return(1);
	screen_y = y_return(1);
	if (screen_x == 0 && screen_y == 0) {
		update_cursor();
		return;
	}
	if (screen_x == 0) {
		screen_x = NUM_COLS-1;
		screen_y--;
		set_terminal_pos(screen_x, screen_y, 1);
		key_putc(ASCII_SPACE);
		screen_x = NUM_COLS-1;
		screen_y--;
		set_terminal_pos(screen_x, screen_y, 1);
	}
	else {
		screen_x--;
		set_terminal_pos(screen_x, screen_y, 1);
		key_putc(ASCII_SPACE);
		screen_x--;
		set_terminal_pos(screen_x, screen_y, 1);
	}
	set_terminal_pos(screen_x, screen_y, 1);
	update_cursor();
}
/* void _arrow_key()
 * Inputs: int dir
 * Return Value: none
 * Function: changes the position of the cursor 
 *           
 */
void _arrow_key(int dir) {
    if (dir == 1) {
        screen_x++;
    } else if (dir == -1) {
        screen_x--;
    }
    update_cursor();
}

/* void _scroll_down();
 * Inputs: none
 * Return Value: none
 * Function: copies each line on the screen to the 
 * 			 previous line and clears the bottom line
 * 			 to implement scrolling with no history
 */
void _scroll_down() {
	int i, j;
	for (i = 0; i < NUM_ROWS; i++) {
		for (j = 0; j < NUM_COLS; j++) {
			if (i == NUM_ROWS-1) {
				*(uint8_t*)(video_mem+((NUM_COLS*i + j)<<1)) = ' ';
			}
			else {
				*(uint8_t*)(video_mem+((NUM_COLS*i + j)<<1)) = *(uint8_t*)(video_mem + ((NUM_COLS*(i+1) + j)<<1));
			}
		}
	}
}

/* int32_t my_puts(int8_t* s);
 * Inputs: int_8* s = pointer to a string of characters
 * Return Value: Number of bytes written
 * Function: Output a string to the console --
 *			 depends on newline
 */
int32_t my_puts(int8_t* s, int32_t nbytes) {
    register int32_t index = 0;
    // while (s[index] != '\0' && index < nbytes) {
    while (1) {
        // if((s[index] == '\0' && index == nbytes) overkill
        if(index == nbytes) {break;}
        my_putc(s[index]);
        index++;
    }
    return index;
}

#define CURSOR_HIGH		0x0E
#define CURSOR_LOW		0x0F
#define CURSOR_MASK		0xFF
#define CURSOR_INDEX	0x3D4
/* void update_cursor();
 * Inputs: none
 * Return Value: none
 * Function: updates the cursor position to show at 
 * 			 the screen_x and screen_y location
 * Code referenced from https://wiki.osdev.org/Text_Mode_Cursor 
 */
void update_cursor() {
	uint16_t pos = screen_y * NUM_COLS + screen_x;
	outb(CURSOR_LOW, CURSOR_INDEX);
	outb((uint8_t) (pos & CURSOR_MASK), CURSOR_INDEX+1);
	outb(CURSOR_HIGH, CURSOR_INDEX);
	// shift by 8 because we split pos into two registers
	outb((uint8_t) ((pos >> 8) & CURSOR_MASK), CURSOR_INDEX+1);
}

/*  x_screen_return
 *  
 *  INPUT: none
 *  OUTPUT: screen_x
 *  RETURN: none
 *  DESCRIPTION: Will return current x coordinate of screen.
 *                  
 */
int x_screen_return() {
	return screen_x;
}

/*  y_screen_return
 *  
 *  INPUT: none
 *  OUTPUT: screen_y
 *  RETURN: none
 *  DESCRIPTION: Will return current y coordinate of screen.
 *                  
 */
int y_screen_return() {
	return screen_y;
}

/*  x_set_screen
 *  
 *  INPUT: Integer x
 *  OUTPUT: none
 *  RETURN: none
 *  DESCRIPTION: Sets current x screen coordinate.
 *                  
 */
void x_set_screen(int x) {
	screen_x = x;
}

/*  y_set_screen
 *  
 *  INPUT: Integer y
 *  OUTPUT: none
 *  RETURN: none
 *  DESCRIPTION: Sets current y  screen coordinate.
 *                  
 */
void y_set_screen(int y) {
	screen_y = y;
}

/*  my_putc
 *  
 *  INPUT: Character - c
 *  OUTPUT: Writes to screen
 *  RETURN: none
 *  DESCRIPTION: Takes character and writes to given coordinates on screen
 *                  
 */
void my_putc(uint8_t c) {
	char* video_addr = video_return();
	int x = x_return(0);
	int y = y_return(0);
    if(c == '\n' || c == '\r') {
        //screen_y++;
		//above line was replaced by below if else statement
		if (y == NUM_ROWS-1) {
				_scroll_down_(video_addr);
		}
		else {
			y++;
		}
        x = 0;
    } else {
        *(uint8_t *)(video_addr + ((NUM_COLS * y + x) << 1)) = c;
        *(uint8_t *)(video_addr + ((NUM_COLS * y + x) << 1) + 1) = text_color;
        x++;
        x %= NUM_COLS;
        y = (y + (x / NUM_COLS)) % NUM_ROWS;
		// if statement was added by matt
		if (x == 0) {
			if (y == NUM_ROWS-1) {
				_scroll_down_(video_addr);
			}
			else {
				y++;
			}
		}
    }
	set_terminal_pos(x, y, 0);
	// update cursor position added by matt
	update_cursor();
}

/*  _scroll_down_
 *  
 *  INPUT: video_addr - pointer to a video address
 *  OUTPUT: Updates screen position
 *  RETURN: none
 *  DESCRIPTION: Depending on bounds, will shift screen or add space to properly scroll.
 *                  
 */
void _scroll_down_(char* video_addr) {
	int i, j;
	for (i = 0; i < NUM_ROWS; i++) {
		for (j = 0; j < NUM_COLS; j++) {
			if (i == NUM_ROWS-1) {
				*(uint8_t*)(video_addr+((NUM_COLS*i + j)<<1)) = ' ';
			}
			else {
				*(uint8_t*)(video_addr+((NUM_COLS*i + j)<<1)) = *(uint8_t*)(video_addr + ((NUM_COLS*(i+1) + j)<<1));
			}
		}
	}
}


/*  key_putc
 *  
 *  INPUT: Character
 *  OUTPUT: Writes to screen.
 *  RETURN: none
 *  DESCRIPTION: Takes a character and outputs it to console while updating screen accordingly
 *                  
 */
void key_putc(uint8_t c) {
	//char* video_addr = video_return();
	screen_x = x_return(1);
	screen_y = y_return(1);
    if(c == '\n' || c == '\r') {
        //screen_y++;
		//above line was replaced by below if else statement
		if (screen_y == NUM_ROWS-1) {
				_scroll_down();
		}
		else {
			screen_y++;
		}
        screen_x = 0;
    } else {
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1)) = c;
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1) + 1) = text_color;
        screen_x++;
        screen_x %= NUM_COLS;
        screen_y = (screen_y + (screen_x / NUM_COLS)) % NUM_ROWS;
		// if statement was added by matt
		if (screen_x == 0) {
			if (screen_y == NUM_ROWS-1) {
				_scroll_down();
			}
			else {
				screen_y++;
			}
		}
    }
	set_terminal_pos(screen_x, screen_y, 1);
	// update cursor position added by matt
	update_cursor();
}

/**********************************/
