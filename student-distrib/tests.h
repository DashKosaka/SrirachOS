#ifndef TESTS_H
#define TESTS_H


/********* Checkpoint 2 *********/

// void checkpoint_2_tests();

/* Filesystem */
/* Tests that a dentry can be read by name */
// void fs_dentry_name();
/* Tests that a dentry can be read by index */
// void fs_dentry_index();

/* Tests that valid data can be read */
// void fs_read_data_valid();
/* Tests for error checking for read_data */
// void fs_read_data_invalid();

/* Tests that write functions do nothing */
// void fs_read_only();

/* Tests that we can open a valid file */
// void fs_open_valid();
/* Tests that error checking for open */
// void fs_open_invalid();

/* Tests that we can read a valid file */
// void fs_read_valid();
/* Tests for error checking */
// void fs_read_invalid();

/* Tests that we can close a valid file */
// void fs_close_valid();
/* Tests for error checking */
// void fs-close_invalid();


/********************************/

// test launcher
void launch_tests();

/*Test paging exception*/

/*Test divide by zero exception*/
int divide_exc();
/*Test rtc */
void test_rtc();
void test_rtc_open();

void test_terminal_read();

/*Test paging exception*/
void page_deref_null();
void page_deref_no_fault();
void page_deref_fault();

/*Captian SR print*/
void captain_sr();
void penguin();

/* File system tests */
void fs_list_files();
void fs_read_by_name();

void fs_file_read_invalid_name();
void fs_file_read_valid_name();
void fs_file_read_data();
void fs_file_read_close_read();

void fs_dir_tests();
void fs_file_tests();

#endif /* TESTS_H */
