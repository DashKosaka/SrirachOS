#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "rtc.h"
#include "keyboard.h"
#include "filesys.h"
#define PASS 1
#define FAIL 0

#define irq_8 8
#define test_count 99
#define slow_count 10
/* format these macros as you see fit */
#define TEST_HEADER     \
    printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)   \
    printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
    /* Use exception #15 for assertions, otherwise
       reserved by Intel */
    asm volatile("int $15");
}


/*************** Checkpoint 1 tests ***************/

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
    TEST_HEADER;

    int i;
    int result = PASS;
    for (i = 0; i < 10; ++i){
        if ((idt[i].offset_15_00 == NULL) && 
            (idt[i].offset_31_16 == NULL)){
            assertion_failure();
            result = FAIL;
        }
    }

    return result;
}

// add more tests here

/*************** Checkpoint 2 tests ***************/

// void checkpoint_2_tests() {}

/* Filesystem oops copied the headers in the wrong place */
/* Tests that a dentry can be read by name */
// void fs_dentry_name() {}
/* Tests that a dentry can be read by index */
// void fs_dentry_index() {}

/* Tests that valid data can be read */
// void fs_read_data_valid() {}
/* Tests for error checking for read_data */
// void fs_read_data_invalid() {}

/* Tests that write functions do nothing */
// void fs_read_only() {}

/* Tests that we can open a valid file */
// void fs_open_valid() {}
/* Tests that error checking for open */
// void fs_open_invalid() {}

/* Tests that we can read a valid file */
// void fs_read_valid() {}
/* Tests for error checking */
// void fs_read_invalid() {}

/* Tests that we can close a valid file */
// void fs_close_valid() {}
/* Tests for error checking */
// void fs-close_invalid() {}



/*************** Checkpoint 3 tests ***************/
/*************** Checkpoint 4 tests ***************/
/*************** Checkpoint 5 tests ***************/


/* Test suite entry point */
void launch_tests(){
    //TEST_OUTPUT("idt_test", idt_test());
    // launch your tests here

    /* This test will result in a page-fault, comment out to test other functionality*/

    // fs_list_files();
    //fs_read_by_name();
    
    //fs_file_read_invalid_name();
    //fs_file_read_valid_name();
    // fs_file_read_data();
    // fs_dir_tests();
	// fs_file_tests();

    /*Test divide by zero exception*/
    //divide_exc();
    // test_rtc_open();
    // test_rtc();
    
    //penguin();
    //int j;
    
    //for(j=0;j<434364334;j++ ){

    //}
    
    //captain_sr();
    /* Paging tests, comment out individual tests, one test at a time*/
    
    //page_deref_no_fault(); // Run this tests first, it will dereference(at 7MB) a valid page - NO PAGE-FAULT
    //page_deref_fault();  // Run this test second, it will dereference(at 3MB) a non-valid page - PAGE FAULT
    //page_deref_null();     // Run this test third, it will dereference(NULL) a non-valid page - PAGE FAULT
    

}



/*  divide_exc()
 *  
 *  INPUT: none
 *  OUTPUT: none
 *  RETURN: none
 *  DESCRIPTION: Test divide by zero exception
 */
int divide_exc(){

    int a=7;    /*random number for test*/
    int b=0;    /*zero to test */
    return a/b;
}
/*  test_rtc_open()
 *  
 *  INPUT: none
 *  OUTPUT: write to vid memory
 *  RETURN: none
 *  DESCRIPTION: Test rtc 
 */
void test_rtc_open(){

    my_clear();
    rtc_open(NULL);
    int d=slow_count;

    while(d>0){
        rtc_read(NULL, (int*)0,0);      /* print char to screen */
        d--;
    }

}/* test_rtc()
 *  
 *  INPUT: none
 *  OUTPUT: write to vid memory
 *  RETURN: none
 *  DESCRIPTION: Test rtc 
 */
void test_rtc(){
    
    /*Disclaimer the varible d shows the amount of times the char prints to screen. It's arbirtarily set*/

    my_clear();
    int d=slow_count;
    rtc_write(0,(int*)2,4);     /*Change frequency to 2*/

    while(d>0){
        rtc_read(NULL, (int*)0,0);      /* print char to screen */
        d--;
    }


    my_clear();
    d=test_count;
    rtc_write(0,(int*)32,4);        /*Change frequency to 32*/

    while(d>0){
        rtc_read(NULL, (int*)0,0);      /* print char to screen */
        d--;
    }
    my_clear();

    d=999;
    rtc_write(0,(int*)128,4);       /*Change frequency to 128*/
    
    while(d>0){
    
        rtc_read(NULL, (int*)0,0);      /*print char to screen */
        d--;
    }
    my_clear();

    d=25;
    rtc_write(0,(int*)4,4);             /*Change frequency to 4*/
    
    while(d>0){
    
        rtc_read(NULL, (int*)0,0);      /* print char to screen */
        d--;
    }
    my_clear();

    d=99999;
    rtc_write(0,(int*)512,4);           /*Change frequency to 512*/
    
    while(d>0){
    
        rtc_read(NULL, (int*)0,0);      /* print char to screen  */
        d--;
    }

}
/* less_bytes() calls terminal write with less bytes
 * than the string
 */
void less_bytes() {
	//terminal_write(1, (int8_t*)"test\n", 10);
	terminal_write(1, (int8_t*)"test\n", 2);
}

/* test_hello() called from test_terminal_read() to read
 * input from terminal and echo to the screen
 */
void test_hello() {
    int32_t cnt;
    int8_t buf[128];

    terminal_write(1, (int8_t*)"Hi, what's your name? ", 22);
    if (-1 == (cnt = terminal_read(0, buf, 128-1))) {
        terminal_write(1, (int8_t*)"Can't read name from keyboard.\n", 30);
        return;
    }
    //buf[cnt] = '\n';
    //buf[cnt+1] = '\0';
    terminal_write(1, (int8_t*)"Hello, ", 7);
    terminal_write(1, buf, cnt);
}

/* test_terminal_read() tests the line buffered input and calls
 * other tests for the terminal
 */
void test_terminal_read() {
    int32_t cnt;
    int8_t buf[128];
    my_clear();
    terminal_write(1, (int8_t*)"Starting 391 Shell\n", 19);

    while (1) {
        terminal_write(1, (int8_t*)"391OS> ", 7);
        if (-1 == (cnt = terminal_read(0, buf, 128-1))) {
            terminal_write(1, (int8_t*)"read from keyboard failed\n", 26);
        }
    
        //if (cnt > 0 && '\n' == buf[cnt - 1])
        //    cnt--;
        //buf[cnt] = '\0';
        if (0 == strncmp(buf, (int8_t*)"exit\n", 5)) {
            printf("Exiting...\n");
            return;
        }
        else if (0 == strncmp(buf, (int8_t*)"hello\n", 6)) {
            test_hello();
        }
		else if (0 == strncmp(buf, (int8_t*)"bytes\n", 6)) {
			less_bytes();
			printf("\n");
		}
        else {
            printf("Invalid command: type 'hello' or 'bytes' or 'exit'\n");
        }
    }
    
}


/*  page_deref_no_fault
 *  
 *  INPUT: none
 *  OUTPUT: none
 *  RETURN: none
 *  DESCRIPTION: Assigns a pointer to a present page address and shall not result in page-fault upon dereference.
 */
// void page_deref_no_fault(){
    // int *ptr;
    // int j;

    // /* This pointer dereference shall not result in a page-fault */ 
    // ptr = (int*)0x700000; // This address is at 7mb which is a present page
    // j = *ptr;
// }

/*  paging_deref_fault
 *  
 *  INPUT: none
 *  OUTPUT: none
 *  RETURN: none
 *  DESCRIPTION: Assigns a pointer to a non-present page address and shall result in page-fault upon dereference
 */
// void page_deref_fault(){
    // my_clear(); //The printf function has a black background that why there is still
                // a black background on test. I know how to make my own printf that prints blue as background
                // but no worries for now.
    // int *ptr;
    // int j;

    // /* This pointer dereference shall result in page-fault */   
    // ptr = (int*)0x300000;  // This address is at 3mb which is a non-present page
    // j = *ptr;
// }

/*  page_deref_null
 *  
 *  INPUT: none
 *  OUTPUT: none
 *  RETURN: none
 *  DESCRIPTION: Assigns a pointer to NULL and shall result in page-fault upon dereference.
 */
// void page_deref_null(){
    // int *ptr = NULL; 
    // int j;

    // /* This pointer dereference shall result in a page-fault */ 
    // j = *ptr;
// }
/*  captain_sr
 *  
 *  INPUT: none
 *  OUTPUT: none
 *  RETURN: none
 *  DESCRIPTION: Prints this on the screen
 */

void captain_sr(){
clear();
printf("                  {}\n");
printf("  ,   A           {}\n");
printf(" / \\,| ,        .--.\n");
printf("|  =|= >        /.--.\\\n");
printf(" \\ /`| `        |====|\n");
printf("  `   |         |`::`|\n");
printf("      |     .-;`\\..../`;_.-^-._\n");
printf("     /\\\\/  /  |...::..|`   :   `|\n");
printf("     |:'\\ |   /'''::''|   .:.     |\n");
printf("      \\ /\\;-,/\\   ::  |SRIRACHOS|\n");
printf("      |\\ <` >  >._::_.| ':...:'   |\n");
printf("      | `""`_/   ^^/>/>  |   ':'   |\n");
printf("      |       |       \\    :    /\n");
printf("      |       |        \\   :   /\n");
printf("      |       |___/\\___|`-.:.-`\n");
printf("      |        \\_ || _/    `\n");
printf("      |        <_ >< _>\n");
printf("      |        |  ||  |\n");
printf("      |        |  ||  |\n");
printf("      |       _\\.:||:./_\n");
printf("      |      /____/\\____\\\n");
printf("\n");
printf(".:*~*:._.:*~*:._.:*~*:._.:*~*:._.:*~*:._.:*~*:._.:*~*:._.:*~*:._.:*~*:\n");




}
void penguin(){
clear();
printf("                 .88888888:. \n");
printf("                88888888.88888. \n");
printf("              .8888888888888888. \n");
printf("              888888888888888888 \n");
printf("              88' _`88'_  `88888 \n");
printf("              88 88 88 88  88888 \n");
printf("              88_88_::_88_:88888 \n");
printf("              88:::,::,:::::8888 \n");
printf("              88`:::::::::'`8888 \n");
printf("             .88  `::::'    8:88. \n");
printf("            8888            `8:888. \n");
printf("          .8888'             `888888. \n");
printf("         .8888:..  .::.  ...:'8888888:. \n");
printf("        .8888.'     :'     `'::`88:88888 \n");
printf("       .8888        '         `.888:8888. \n");
printf("      888:8         .           888:88888 \n");
printf("    .888:88        .:           888:88888: \n");
printf("    8888888.       ::           88:888888 \n");
printf("    `.::.888.      ::          .88888888 \n");
printf("   .::::::.888.    ::         :::`8888'.:. \n");
printf("  ::::::::::.888   '         .:::::::::::: \n");
printf("  ::::::::::::.8    '      .:8::::::::::::. \n");
printf(" .::::::::::::::.        .:888::::::::::::: \n");
printf(" :::::::::::::::88:.__..:88888:::::::::::' \n");
//printf("  `'.:::::::::::88888888888.88:::::::::' \n");
//printf("        `':::_:' -- '' -'-' `':_::::'` \n");









}


/*  fs_list_files
 *  
 *  INPUT: none
 *  OUTPUT: Files and their information within given directory
 *  RETURN: none
 *  DESCRIPTION: Will print all files in the directory '.' to screen.
 */
// void fs_list_files() {

    // int32_t valid = -1, valid_read = -1;


    // if( -1 == (valid = dir_open((uint8_t*)"."))){
        // printf("ERROR:  Directory opened failed\n");
    // }
    // if(-1 == (valid_read = dir_read(valid, 0, 0)))
        // valid = 0;
        // printf("ERROR:  Directory read failed\n");
    // if(-1 == (valid = dir_close((uint8_t*)".")))
        // valid = 0;
        // printf("ERROR:  Directory close failed\n");

    // printf(" Entered fs_list_files]\n");

// }

/*  fs_read_by_name
 *  
 *  INPUT: none
 *  OUTPUT: Contents of a file to screen
 *  RETURN: none
 *  DESCRIPTION: Will print, given file name, the contents of file to screen
 */
// void fs_read_by_name() {
// int32_t valid = -1, valid_read = -1;


    // if( -1 == (valid = file_open((uint8_t*)"frame0.txt"))){
        // printf("ERROR:  File opened failed\n");
    // }
    // if( -1 == (valid = file_open((uint8_t*)"verylargetextwithverylongname.tx"))){
        // printf("ERROR:  File opened failed\n");
    // }
    // if( -1 == (valid = file_open((uint8_t*)"frame1.txt"))){
    //     printf("ERROR:  File opened failed\n");
    // }
    
    // if( -1 == (valid = file_open((uint8_t*)"frame0.txt"))){
    //     printf("ERROR:  File already opened\n");
    // }

    // if(-1 == (valid_read = file_read(2, 0)))
    //     printf("ERROR:  File read failed\n");
    // if(-1 == (valid_read = file_read(1, 0, 5277)))
        // printf("ERROR:  File read failed\n");
    // if(-1 == (valid_read = file_read(0, 0, 198)))
        // printf("ERROR:  File read failed\n");

        //valid = 0;
    //if(-1 == (valid = dir_close((uint8_t*)".")))
        //valid = 0;
        //printf("ERROR:  Directory close failed\n");






    // printf("Entered fs_read_by_name\n");

// }


/* Test that the file functions work as intended and don't read invalid names */
// void fs_file_read_invalid_name() {
    // int32_t fd = -1;

    // /* Not a valid file name in current file system, will fail */
    // if( -1 == (fd = file_open((uint8_t*)"doesnotexist")))
        // printf("ERROR:  File opened failed\n");
    // else
        // printf("SUCCESS: File opened!\n");

    // /* Not a valid file name as it is too long for our file system to interpret */
    // if( -1 == (fd = file_open((uint8_t*)"verylargetextwithverylongname.txt")))
        // printf("ERROR:  File opened failed\n");
    // else
        // printf("SUCCESS: File opened!\n");
// }

/* Test that the file functions work as intended and can read valid names */
// void fs_file_read_valid_name() {
    // int32_t fd;
    
    // /* Valid file should open */
    // if( -1 == (fd = file_open((uint8_t*)"frame0.txt")))
        // printf("ERROR:  File opened failed\n");
    // else
        // printf("SUCCESS: File opened!\n");

    // /* Valid file, should open. */
    // if( -1 == (fd = file_open((uint8_t*)"pingpong")))
        // printf("ERROR:  File opened failed\n");
    // else 
        // printf("SUCCESS: Filed opened!\n");

    // /* Valid file, should open */
    // if( -1 == (fd = file_open((uint8_t*)"verylargetextwithverylongname.tx")))
        // printf("ERROR:  File opened failed\n");
    // else 
        // printf("SUCCESS: Filed opened!\n");
// }

/* Test that we can read data from files correctly */
// void fs_file_read_data() {
    // uint32_t fd;
    
    // /* Valid file should open, with file descriptor of index 0 */
    // if( -1 == (fd = file_open((uint8_t*)"frame0.txt")))
        // printf("ERROR:  File opened failed\n");
    // else
        // printf("SUCCESS: File opened!\n");

    // /* Valid read by file descriptor */
    // if( -1 == (file_read(fd, 0, -1)))
        // printf("ERROR:  File read failed\n");
    // else
        // printf("SUCCESS: File read!\n");

    // /* Read only part of the file */
    // if( -1 == (file_read(fd, 25, 137)))    	
        // printf("ERROR:  File read failed\n");
    // else
        // printf("SUCCESS: File read!\n");

    // /* Valid file, should open with file descriptor of index 1 */
    // if( -1 == (fd = file_open((uint8_t*)"pingpong")))
        // printf("ERROR:  File opened failed\n");
    // else 
        // printf("SUCCESS: Filed opened!\n");

    // /* read non text file shoule work */
    // if( -1 == (file_read(fd, 0, -1)))
        // printf("ERROR:  File read failed\n");
    // else
        // printf("SUCCESS: File read!\n");
    
    // /* Valid file, should open with file descriptor of index 2 */
    // if( -1 == (fd = file_open((uint8_t*)"verylargetextwithverylongname.tx")))
        // printf("ERROR:  File opened failed\n");
    // else 
        // printf("SUCCESS: Filed opened!\n");    

    // /* fd at this point is at index 2, file should read succesfully */
    // if( -1 == (file_read(fd, 0, -1)))
        // printf("ERROR:  File read failed\n");
    // else
        // printf("SUCCESS: File read!\n");

    // /* fd at this point is at index 2, file should read succesfully and only part of the file */
    // if( -1 == (file_read(fd, 5206, 62)))
        // printf("ERROR:  File read failed\n");
    // else
        // printf("SUCCESS: File read!\n");

    // /* Setting file descriptor to an unopened file/process, should fail read, as file is not opened */
    // fd = 6;
    // if( -1 == (file_read(fd, 0, -1)))
        // printf("ERROR:  File read failed\n");
    // else
        // printf("SUCCESS: File read!\n");
// }

/* Tests that we can close */
// void fs_file_read_close_read(){
    // uint32_t fd;

    // /* No current live valid file descriptors, this attempt at closing a file should fail */
    // fd = 2;
    // if( -1 == (fd = file_close(fd)))
        // printf("ERROR:  File close failed\n");
    // else
        // printf("SUCCESS: File closed!\n");

    // /* Following block should succesfully open this file, make a read, and successfully close it */
    // if( -1 == (fd = file_open((uint8_t*)"frame0.txt")))
        // printf("ERROR:  File opened failed\n");
    // else
        // printf("SUCCESS: File opened!\n");

    // /* Valid file read */
    // if( -1 == (file_read(fd, 0, -1)))
        // printf("ERROR:  File read failed\n");
    // else
        // printf("SUCCESS: File read!\n");

    // /* Valid file close */
    // if( -1 == (fd = file_close(fd)))
        // printf("ERROR:  File close failed\n");
    // else
        // printf("SUCCESS: File closed!\n");

    // /* Attempt to read recently closed file, should fail */
    // if( -1 == (file_read(fd, 0, -1)))
        // printf("ERROR:  File read failed\n");
    // else
        // printf("SUCCESS: File read!\n");
// }

/* Test that the directory functions work as intended */
// void fs_dir_tests(){
    // uint32_t fd;

    // /* Attempt to open a file with dir_open(), will fail */
    // if( -1 == (fd = dir_open((uint8_t *)"frame1.txt")))
        // printf("ERROR:  Directory open failed\n");
    // else
        // printf("SUCCESS: Directory opened!\n");  

    // /* Attempt to read a file with dir_read(), will fail */
    // if( -1 == (fd = file_open((uint8_t*)"frame0.txt")))
        // printf("ERROR:  File opened failed\n");
    // else
        // printf("SUCCESS: File opened!\n");

    // if( -1 == (fd = dir_read(fd,0,5)))
        // printf("ERROR:  Directory read failed\n");
    // else
        // printf("SUCCESS: Directory read!\n");

    // /* Attempt to close a file with dir_close(), will fail */
    // if( -1 == (fd = file_open((uint8_t*)"frame1.txt")))
        // printf("ERROR:  File opened failed\n");
    // else
        // printf("SUCCESS: File opened!\n");

    // if( -1 == (fd = dir_close(fd)))
        // printf("ERROR:  Directory close failed\n");
    // else
        // printf("SUCCESS: Directory closed!\n");

    // /* Attempt to read an unopened directory, will fail */
    // if( -1 == (fd = dir_read(fd,0,5)))
        // printf("ERROR:  Directory read failed\n");
    // else
        // printf("SUCCESS: Directory read!\n");

    // /* Attempt to close a directory that's not opened, will fail */
    // if( -1 == (fd = dir_close(fd)))
        // printf("ERROR:  Directory close failed\n");
    // else
        // printf("SUCCESS: Directory closed!\n");

    // /* Attempt to open a valid directory and attempt to reopen, will fail at second attempt */
    // if( -1 == (fd = dir_open((uint8_t*)".")))
        // printf("ERROR:  Directory open failed\n");
    // else
        // printf("SUCCESS: Directory open!\n");
    
    // if( -1 == (dir_open((uint8_t*)".")))
        // printf("ERROR:  Directory open failed\n");
    // else
        // printf("SUCCESS: Directory open!\n");
    // /* Make a read and close directory, and attempt to close again, will fail on second close attempt */
    // if( -1 == (dir_read(fd,0,5)))
        // printf("ERROR:  Directory read failed\n");
    // else
        // printf("SUCCESS: Directory read!\n");

    // if( -1 == (dir_close(fd)))
        // printf("ERROR:  Directory close failed\n");
    // else
        // printf("SUCCESS: Directory closed!\n");

    // if( -1 == (dir_close(fd)))
        // printf("ERROR:  Directory close failed\n");
    // else
        // printf("SUCCESS: Directory closed!\n");
// }

/* Test that the file functions work as intended */
// void fs_file_tests(){
    // uint32_t fd;
    // /* Attempt to open a directory with file_open(), will fail */
    // if( -1 == (fd = file_open((uint8_t *)".")))
        // printf("ERROR:  file open failed\n");
    // else
        // printf("SUCCESS: file opened!\n");  

    // /* Attempt to read a directory with file_read(), will fail */
    // if( -1 == (fd = dir_open((uint8_t *)".")))
        // printf("ERROR:  Directory open failed\n");
    // else
        // printf("SUCCESS: Directory opened!\n");  
    // if( -1 == (file_read(fd, 0, 0)))
        // printf("ERROR:  File read failed\n");
    // else
        // printf("SUCCESS: File read!\n");  


    // /* Attempt to close a directory with file_close(), will fail */
    // if( -1 == (file_close(fd)))
        // printf("ERROR:  File close failed\n");
    // else
        // printf("SUCCESS: File closed!\n");  

    // /* Attempt to read an unopened file, will fail */
    // fd = 7;
    // if( -1 == (file_read(7, 0, -1)))
        // printf("ERROR:  File read failed\n");
    // else
        // printf("SUCCESS: File read!\n");  

    // /* Attempt to close a file that's not opened, will fail */
    // if( -1 == (fd = file_close(fd)))
        // printf("ERROR:  File close failed\n");
    // else
        // printf("SUCCESS: File opened!\n");  

    // /* Attempt to open a valid file and attempt to reopen, will fail at second attempt */
    // if( -1 == (fd = file_open((uint8_t*)"frame1.txt")))
        // printf("ERROR:  File open failed\n");
    // else
        // printf("SUCCESS: File opened!\n");

    // if( -1 == (fd = file_open((uint8_t*)"frame1.txt")))
        // printf("ERROR:  File open failed\n");
    // else
        // printf("SUCCESS: File opened!\n");

    // /* Make a read and close file, and attempt to close again, will fail on second close attempt */
    // if( -1 == (fd = file_open((uint8_t*)"frame0.txt")))
        // printf("ERROR:  File open failed\n");
    // else
        // printf("SUCCESS: File opened!\n");

    // if( -1 == (file_read(fd, 0, -1)))
        // printf("ERROR:  File read failed\n");
    // else
        // printf("SUCCESS: File read!\n");

    // if( -1 == (file_close(fd)))
        // printf("ERROR:  File close failed\n");
    // else
        // printf("SUCCESS: File closed!\n");
    
    // if( -1 == (file_close(fd)))
        // printf("ERROR:  File close failed\n");
    // else
        // printf("SUCCESS: File closed!\n");


    /* Attempt to read at an invalid offset, will fail */
    // if( -1 == (fd = file_open((uint8_t*)"frame0.txt")))
        // printf("ERROR:  File open failed\n");
    // else
        // printf("SUCCESS: File opened!\n");

    // if( -1 == (file_read(fd, 99999, -1)))
        // printf("ERROR:  File read failed\n");
    // else
        // printf("SUCCESS: File read!\n");  

    // /* Attempt to read at an invalid length, will fail */
    // if( -1 == (file_read(fd, 0, 4096)))
        // printf("ERROR:  File read failed\n");
    // else
        // printf("SUCCESS: File read!\n"); 
// }

